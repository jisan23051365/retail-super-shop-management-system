#ifndef AUTH_H
#define AUTH_H

#include "common.h"

/* ── Data structures ─────────────────────────────────────────────────────── */

typedef struct {
    int  id;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];   /* stored as plain text (demo system) */
    int  role;                         /* ROLE_ADMIN or ROLE_CASHIER          */
    int  active;                       /* 1 = active, 0 = disabled            */
} User;

/* ── Current session ─────────────────────────────────────────────────────── */
extern User current_user;

/* ── Function prototypes ─────────────────────────────────────────────────── */

/* Authenticate a user; returns SUCCESS or FAILURE */
int  auth_login(void);

/* Log out the current user */
void auth_logout(void);

/* Admin: add a new user account */
int  auth_add_user(void);

/* Admin: list all user accounts */
void auth_list_users(void);

/* Admin: toggle a user's active status */
int  auth_toggle_user(void);

/* Admin: change another user's password */
int  auth_change_password(void);

/* Return 1 if the current user is an admin, 0 otherwise */
int  auth_is_admin(void);

#endif /* AUTH_H */
