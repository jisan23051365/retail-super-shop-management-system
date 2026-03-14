#ifndef INVENTORY_H
#define INVENTORY_H

#include "common.h"

/* ── Data structures ─────────────────────────────────────────────────────── */

typedef struct {
    int    id;
    char   name[MAX_NAME_LEN];
    char   category[MAX_CATEGORY_LEN];
    double cost_price;
    double sell_price;
    int    quantity;
    int    low_stock_threshold;
    int    active;   /* 1 = available, 0 = removed */
} Product;

/* ── Function prototypes ─────────────────────────────────────────────────── */

/* Admin: add a new product */
int  inventory_add_product(void);

/* Admin: edit an existing product */
int  inventory_edit_product(void);

/* Admin: deactivate (soft-delete) a product */
int  inventory_delete_product(void);

/* List all active products */
void inventory_list_products(void);

/* Search products by name or category */
void inventory_search_products(void);

/* Show low-stock products */
void inventory_low_stock_report(void);

/* Find a product by ID; returns SUCCESS and fills *p, or FAILURE */
int  inventory_find_by_id(int id, Product *p);

/* Decrease stock after a sale; returns SUCCESS or FAILURE */
int  inventory_decrease_stock(int product_id, int qty);

/* Increase stock (e.g. returns or restocking) */
int  inventory_increase_stock(int product_id, int qty);

/* Print a single product row */
void inventory_print_product(const Product *p);

#endif /* INVENTORY_H */
