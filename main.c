#include "include/auth.h"
#include "include/inventory.h"
#include "include/sales.h"
#include "include/analytics.h"
#include "include/utils.h"

/* ── Forward declarations ────────────────────────────────────────────────── */
static void menu_admin(void);
static void menu_cashier(void);
static void menu_inventory(void);
static void menu_users(void);
static void menu_analytics(void);
static void menu_sales(void);

/* ── Menu: top-level dispatcher ──────────────────────────────────────────── */

int main(void) {
    utils_ensure_data_dir();

    while (1) {
        if (auth_login() == SUCCESS) break;
        printf("  Try again? (y/n): ");
        char c[4];
        if (!utils_read_line(c, sizeof(c)) || tolower((unsigned char)c[0]) != 'y') {
            printf("  Exiting.\n");
            return 0;
        }
    }

    if (auth_is_admin()) menu_admin();
    else                 menu_cashier();

    return 0;
}

/* ── Admin main menu ─────────────────────────────────────────────────────── */

static void menu_admin(void) {
    while (1) {
        utils_print_title("ADMIN MENU", 50);
        printf("  1. Inventory Management\n");
        printf("  2. Sales\n");
        printf("  3. Analytics & Reports\n");
        printf("  4. User Management\n");
        printf("  5. Logout\n");
        utils_print_separator('-', 50);

        int choice = utils_prompt_int("  Choice: ");
        switch (choice) {
            case 1: menu_inventory(); break;
            case 2: menu_sales();     break;
            case 3: menu_analytics(); break;
            case 4: menu_users();     break;
            case 5:
                auth_logout();
                /* Allow re-login */
                while (1) {
                    if (auth_login() == SUCCESS) break;
                    printf("  Try again? (y/n): ");
                    char c[4];
                    if (!utils_read_line(c, sizeof(c)) ||
                        tolower((unsigned char)c[0]) != 'y') return;
                }
                if (auth_is_admin()) continue;
                else { menu_cashier(); return; }
            default:
                printf("  Invalid choice.\n");
        }
    }
}

/* ── Cashier main menu ───────────────────────────────────────────────────── */

static void menu_cashier(void) {
    while (1) {
        utils_print_title("CASHIER MENU", 50);
        printf("  1. New Sale\n");
        printf("  2. View Sale by ID\n");
        printf("  3. Recent Sales\n");
        printf("  4. Product List\n");
        printf("  5. Search Products\n");
        printf("  6. Logout\n");
        utils_print_separator('-', 50);

        int choice = utils_prompt_int("  Choice: ");
        switch (choice) {
            case 1: sales_new_sale();          break;
            case 2: sales_view_sale();         break;
            case 3: sales_list_recent(20);     break;
            case 4: inventory_list_products(); break;
            case 5: inventory_search_products(); break;
            case 6:
                auth_logout();
                return;
            default:
                printf("  Invalid choice.\n");
        }
    }
}

/* ── Inventory sub-menu ──────────────────────────────────────────────────── */

static void menu_inventory(void) {
    while (1) {
        utils_print_title("INVENTORY MANAGEMENT", 50);
        printf("  1. List Products\n");
        printf("  2. Add Product\n");
        printf("  3. Edit Product\n");
        printf("  4. Deactivate Product\n");
        printf("  5. Search Products\n");
        printf("  6. Low Stock Report\n");
        printf("  7. Back\n");
        utils_print_separator('-', 50);

        int choice = utils_prompt_int("  Choice: ");
        switch (choice) {
            case 1: inventory_list_products();    break;
            case 2: inventory_add_product();      break;
            case 3: inventory_edit_product();     break;
            case 4: inventory_delete_product();   break;
            case 5: inventory_search_products();  break;
            case 6: inventory_low_stock_report(); break;
            case 7: return;
            default: printf("  Invalid choice.\n");
        }
    }
}

/* ── User management sub-menu ────────────────────────────────────────────── */

static void menu_users(void) {
    while (1) {
        utils_print_title("USER MANAGEMENT", 50);
        printf("  1. List Users\n");
        printf("  2. Add User\n");
        printf("  3. Toggle User Status (enable/disable)\n");
        printf("  4. Change User Password\n");
        printf("  5. Back\n");
        utils_print_separator('-', 50);

        int choice = utils_prompt_int("  Choice: ");
        switch (choice) {
            case 1: auth_list_users();       break;
            case 2: auth_add_user();         break;
            case 3: auth_toggle_user();      break;
            case 4: auth_change_password();  break;
            case 5: return;
            default: printf("  Invalid choice.\n");
        }
    }
}

/* ── Analytics sub-menu ──────────────────────────────────────────────────── */

static void menu_analytics(void) {
    while (1) {
        utils_print_title("ANALYTICS & REPORTS", 50);
        printf("  1. Overall Sales Summary\n");
        printf("  2. Date Range Report\n");
        printf("  3. Top 10 Selling Products\n");
        printf("  4. Revenue by Category\n");
        printf("  5. Profit Margin Report\n");
        printf("  6. Low Stock Report\n");
        printf("  7. Back\n");
        utils_print_separator('-', 50);

        int choice = utils_prompt_int("  Choice: ");
        switch (choice) {
            case 1: analytics_overall_summary();     break;
            case 2: analytics_date_range_report();   break;
            case 3: analytics_top_products(10);      break;
            case 4: analytics_revenue_by_category(); break;
            case 5: analytics_profit_report();       break;
            case 6: inventory_low_stock_report();    break;
            case 7: return;
            default: printf("  Invalid choice.\n");
        }
    }
}

/* ── Sales sub-menu (admin) ──────────────────────────────────────────────── */

static void menu_sales(void) {
    while (1) {
        utils_print_title("SALES MENU", 50);
        printf("  1. New Sale\n");
        printf("  2. View Sale by ID\n");
        printf("  3. Recent Sales (last 20)\n");
        printf("  4. Back\n");
        utils_print_separator('-', 50);

        int choice = utils_prompt_int("  Choice: ");
        switch (choice) {
            case 1: sales_new_sale();      break;
            case 2: sales_view_sale();     break;
            case 3: sales_list_recent(20); break;
            case 4: return;
            default: printf("  Invalid choice.\n");
        }
    }
}
