#ifndef COMMON_H
#define COMMON_H

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* ── Data-file paths ─────────────────────────────────────────────────────── */
#define USERS_FILE      "data/users.dat"
#define PRODUCTS_FILE   "data/products.dat"
#define SALES_FILE      "data/sales.dat"

/* ── Field-length limits ─────────────────────────────────────────────────── */
#define MAX_NAME_LEN      64
#define MAX_USERNAME_LEN  32
#define MAX_PASSWORD_LEN  64
#define MAX_CATEGORY_LEN  32
#define MAX_CART_ITEMS    50
#define MAX_SALE_ITEMS    50
#define MAX_DATE_LEN      20

/* ── User roles ──────────────────────────────────────────────────────────── */
#define ROLE_ADMIN    1
#define ROLE_CASHIER  2

/* ── Return codes ────────────────────────────────────────────────────────── */
#define SUCCESS  0
#define FAILURE -1

#endif /* COMMON_H */
