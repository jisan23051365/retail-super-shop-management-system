#include "../include/auth.h"
#include "../include/utils.h"
#include <strings.h>

/* ── Global session state ────────────────────────────────────────────────── */
User current_user;

/* ── Internal helpers ────────────────────────────────────────────────────── */

static int next_user_id(void) {
    FILE *fp = fopen(USERS_FILE, "rb");
    if (!fp) return 1;
    int max_id = 0;
    User u;
    while (fread(&u, sizeof(User), 1, fp) == 1) {
        if (u.id > max_id) max_id = u.id;
    }
    fclose(fp);
    return max_id + 1;
}

/* Load all users into a heap-allocated array; caller must free.
   Returns the count of users read, or 0 on failure. */
static int load_users(User **out) {
    FILE *fp = fopen(USERS_FILE, "rb");
    if (!fp) { *out = NULL; return 0; }

    fseek(fp, 0, SEEK_END);
    long bytes = ftell(fp);
    rewind(fp);
    int count = (int)(bytes / (long)sizeof(User));
    if (count == 0) { fclose(fp); *out = NULL; return 0; }

    *out = malloc((size_t)count * sizeof(User));
    if (!*out) { fclose(fp); return 0; }
    count = (int)fread(*out, sizeof(User), count, fp);
    fclose(fp);
    return count;
}

static int save_users(const User *arr, int count) {
    FILE *fp = fopen(USERS_FILE, "wb");
    if (!fp) { perror("Cannot rewrite users file"); return FAILURE; }
    fwrite(arr, sizeof(User), count, fp);
    fclose(fp);
    return SUCCESS;
}

static void seed_default_admin(void) {
    /* Create a default admin account if the users file is empty/missing */
    FILE *fp = fopen(USERS_FILE, "rb");
    if (fp) {
        User u;
        if (fread(&u, sizeof(User), 1, fp) == 1) {
            fclose(fp);
            return;  /* at least one user exists */
        }
        fclose(fp);
    }
    /* Write the default admin */
    fp = fopen(USERS_FILE, "wb");
    if (!fp) {
        perror("Cannot create users file");
        return;
    }
    User admin;
    memset(&admin, 0, sizeof(User));
    admin.id     = 1;
    admin.role   = ROLE_ADMIN;
    admin.active = 1;
    strncpy(admin.username, "admin",    MAX_USERNAME_LEN - 1);
    strncpy(admin.password, "admin123", MAX_PASSWORD_LEN - 1);
    fwrite(&admin, sizeof(User), 1, fp);
    fclose(fp);
    printf("  [INFO] Default admin account created (user: admin / pass: admin123).\n");
    printf("  [INFO] Please change the password after first login.\n\n");
}

/* ── Public functions ────────────────────────────────────────────────────── */

int auth_login(void) {
    utils_ensure_data_dir();
    seed_default_admin();

    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];

    utils_print_title("LOGIN", 50);
    utils_prompt_string("  Username: ", username, sizeof(username));
    utils_read_password("  Password: ", password, sizeof(password));

    FILE *fp = fopen(USERS_FILE, "rb");
    if (!fp) {
        printf("  Error: cannot open user database.\n");
        return FAILURE;
    }

    User u;
    int found = 0;
    while (fread(&u, sizeof(User), 1, fp) == 1) {
        if (strcmp(u.username, username) == 0 &&
            strcmp(u.password, password) == 0 &&
            u.active == 1) {
            found = 1;
            break;
        }
    }
    fclose(fp);

    if (!found) {
        printf("  Invalid username or password, or account is disabled.\n\n");
        return FAILURE;
    }

    current_user = u;
    printf("\n  Welcome, %s! (Role: %s)\n\n",
           current_user.username,
           current_user.role == ROLE_ADMIN ? "Admin" : "Cashier");
    return SUCCESS;
}

void auth_logout(void) {
    printf("\n  Logged out. Goodbye, %s!\n\n", current_user.username);
    memset(&current_user, 0, sizeof(User));
}

int auth_add_user(void) {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char role_str[8];
    int  role;

    utils_print_title("ADD USER", 50);
    utils_prompt_string("  Username   : ", username, sizeof(username));

    /* Check if username already exists */
    FILE *fp = fopen(USERS_FILE, "rb");
    if (fp) {
        User u;
        while (fread(&u, sizeof(User), 1, fp) == 1) {
            if (strcmp(u.username, username) == 0) {
                fclose(fp);
                printf("  Error: username '%s' already exists.\n", username);
                return FAILURE;
            }
        }
        fclose(fp);
    }

    utils_read_password("  Password   : ", password, sizeof(password));
    if (strlen(password) < 4) {
        printf("  Error: password must be at least 4 characters.\n");
        return FAILURE;
    }

    while (1) {
        utils_prompt_string("  Role (admin/cashier): ", role_str, sizeof(role_str));
        if (strcasecmp(role_str, "admin") == 0) {
            role = ROLE_ADMIN;
            break;
        } else if (strcasecmp(role_str, "cashier") == 0) {
            role = ROLE_CASHIER;
            break;
        }
        printf("  Please enter 'admin' or 'cashier'.\n");
    }

    User new_user;
    memset(&new_user, 0, sizeof(User));
    new_user.id     = next_user_id();
    new_user.role   = role;
    new_user.active = 1;
    strncpy(new_user.username, username, MAX_USERNAME_LEN - 1);
    strncpy(new_user.password, password, MAX_PASSWORD_LEN - 1);

    fp = fopen(USERS_FILE, "ab");
    if (!fp) {
        perror("Cannot open users file for writing");
        return FAILURE;
    }
    fwrite(&new_user, sizeof(User), 1, fp);
    fclose(fp);

    printf("  User '%s' created with ID %d.\n", new_user.username, new_user.id);
    return SUCCESS;
}

void auth_list_users(void) {
    utils_print_title("USER LIST", 60);
    FILE *fp = fopen(USERS_FILE, "rb");
    if (!fp) {
        printf("  No users found.\n");
        return;
    }
    printf("  %-5s %-20s %-10s %-8s\n", "ID", "Username", "Role", "Status");
    utils_print_separator('-', 60);
    User u;
    while (fread(&u, sizeof(User), 1, fp) == 1) {
        printf("  %-5d %-20s %-10s %-8s\n",
               u.id,
               u.username,
               u.role == ROLE_ADMIN ? "Admin" : "Cashier",
               u.active ? "Active" : "Disabled");
    }
    fclose(fp);
    utils_print_separator('-', 60);
}

int auth_toggle_user(void) {
    auth_list_users();
    int id = utils_prompt_int("  Enter User ID to toggle: ");

    User *users;
    int count = load_users(&users);
    if (!count) { printf("  No user database found.\n"); return FAILURE; }

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (users[i].id == id) {
            if (users[i].id == current_user.id) {
                printf("  Cannot disable your own account.\n");
                free(users);
                return FAILURE;
            }
            users[i].active = !users[i].active;
            printf("  User '%s' is now %s.\n",
                   users[i].username,
                   users[i].active ? "Active" : "Disabled");
            found = 1;
            break;
        }
    }
    if (!found) { printf("  User ID %d not found.\n", id); free(users); return FAILURE; }

    int rc = save_users(users, count);
    free(users);
    return rc;
}

int auth_change_password(void) {
    auth_list_users();
    int id = utils_prompt_int("  Enter User ID to change password: ");

    User *users;
    int count = load_users(&users);
    if (!count) { printf("  No user database found.\n"); return FAILURE; }

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (users[i].id == id) {
            char newpw[MAX_PASSWORD_LEN];
            utils_read_password("  New password: ", newpw, sizeof(newpw));
            if (strlen(newpw) < 4) {
                printf("  Error: password must be at least 4 characters.\n");
                free(users);
                return FAILURE;
            }
            strncpy(users[i].password, newpw, MAX_PASSWORD_LEN - 1);
            printf("  Password updated for user '%s'.\n", users[i].username);
            found = 1;
            break;
        }
    }
    if (!found) { printf("  User ID %d not found.\n", id); free(users); return FAILURE; }

    int rc = save_users(users, count);
    free(users);
    return rc;
}

int auth_is_admin(void) {
    return current_user.role == ROLE_ADMIN;
}
