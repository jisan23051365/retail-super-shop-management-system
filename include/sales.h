#ifndef SALES_H
#define SALES_H

#include "common.h"

/* ── Data structures ─────────────────────────────────────────────────────── */

typedef struct {
    int    product_id;
    char   product_name[MAX_NAME_LEN];
    int    quantity;
    double unit_price;
    double subtotal;
} SaleItem;

typedef struct {
    int      id;
    int      cashier_id;
    char     cashier_name[MAX_USERNAME_LEN];
    char     date[MAX_DATE_LEN];          /* "YYYY-MM-DD HH:MM:SS" */
    SaleItem items[MAX_SALE_ITEMS];
    int      item_count;
    double   total_amount;
    double   amount_paid;
    double   change_due;
} Sale;

/* ── Function prototypes ─────────────────────────────────────────────────── */

/* Interactive: create a new sale (cashier/admin) */
int  sales_new_sale(void);

/* View a past sale by ID */
void sales_view_sale(void);

/* List recent sales (last N records) */
void sales_list_recent(int n);

/* Print a formatted receipt for a sale */
void sales_print_receipt(const Sale *s);

#endif /* SALES_H */
