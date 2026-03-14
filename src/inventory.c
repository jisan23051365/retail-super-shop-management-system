#include "../include/inventory.h"
#include "../include/utils.h"

/* ── Internal helpers ────────────────────────────────────────────────────── */

static int next_product_id(void) {
    FILE *fp = fopen(PRODUCTS_FILE, "rb");
    if (!fp) return 1;
    int max_id = 0;
    Product p;
    while (fread(&p, sizeof(Product), 1, fp) == 1) {
        if (p.id > max_id) max_id = p.id;
    }
    fclose(fp);
    return max_id + 1;
}

/* Read all products into a heap-allocated array; caller must free */
static int load_products(Product **out) {
    FILE *fp = fopen(PRODUCTS_FILE, "rb");
    if (!fp) { *out = NULL; return 0; }

    /* Count entries */
    fseek(fp, 0, SEEK_END);
    long bytes = ftell(fp);
    rewind(fp);
    int count = (int)(bytes / (long)sizeof(Product));
    if (count == 0) { fclose(fp); *out = NULL; return 0; }

    *out = malloc((size_t)count * sizeof(Product));
    if (!*out) { fclose(fp); return 0; }
    count = (int)fread(*out, sizeof(Product), count, fp);
    fclose(fp);
    return count;
}

static int save_products(const Product *arr, int count) {
    FILE *fp = fopen(PRODUCTS_FILE, "wb");
    if (!fp) { perror("Cannot write products file"); return FAILURE; }
    fwrite(arr, sizeof(Product), count, fp);
    fclose(fp);
    return SUCCESS;
}

/* ── Public functions ────────────────────────────────────────────────────── */

void inventory_print_product(const Product *p) {
    printf("  %-4d %-20s %-12s %8.2f %8.2f %6d %6d %s\n",
           p->id, p->name, p->category,
           p->cost_price, p->sell_price,
           p->quantity, p->low_stock_threshold,
           p->active ? "Active" : "Inactive");
}

static void print_product_header(void) {
    printf("  %-4s %-20s %-12s %8s %8s %6s %6s %s\n",
           "ID", "Name", "Category",
           "Cost", "Price", "Qty", "MinQty", "Status");
    utils_print_separator('-', 80);
}

int inventory_add_product(void) {
    Product p;
    memset(&p, 0, sizeof(Product));

    utils_print_title("ADD PRODUCT", 60);
    utils_prompt_string("  Name            : ", p.name,     sizeof(p.name));
    utils_prompt_string("  Category        : ", p.category, sizeof(p.category));
    p.cost_price           = utils_prompt_double("  Cost price      : ");
    p.sell_price           = utils_prompt_double("  Selling price   : ");
    p.quantity             = utils_prompt_int("  Initial quantity: ");
    p.low_stock_threshold  = utils_prompt_int("  Low-stock level : ");
    p.id     = next_product_id();
    p.active = 1;

    FILE *fp = fopen(PRODUCTS_FILE, "ab");
    if (!fp) { perror("Cannot open products file"); return FAILURE; }
    fwrite(&p, sizeof(Product), 1, fp);
    fclose(fp);

    printf("  Product '%s' added with ID %d.\n", p.name, p.id);
    return SUCCESS;
}

int inventory_edit_product(void) {
    inventory_list_products();
    int id = utils_prompt_int("  Enter Product ID to edit: ");

    Product *arr;
    int count = load_products(&arr);
    if (!count) { printf("  No products found.\n"); return FAILURE; }

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (arr[i].id == id && arr[i].active) {
            Product *p = &arr[i];
            utils_print_title("EDIT PRODUCT", 60);
            printf("  Leave blank to keep current value.\n\n");

            char buf[MAX_NAME_LEN];
            printf("  Name [%s]: ", p->name); fflush(stdout);
            if (utils_read_line(buf, sizeof(buf)) && buf[0]) strncpy(p->name, buf, MAX_NAME_LEN - 1);

            printf("  Category [%s]: ", p->category); fflush(stdout);
            if (utils_read_line(buf, sizeof(buf)) && buf[0]) strncpy(p->category, buf, MAX_CATEGORY_LEN - 1);

            char dbuf[32];
            printf("  Cost price [%.2f]: ", p->cost_price); fflush(stdout);
            if (utils_read_line(dbuf, sizeof(dbuf)) && dbuf[0]) {
                char *end; double v = strtod(dbuf, &end);
                if (end != dbuf && *end == '\0' && v >= 0) p->cost_price = v;
            }

            printf("  Selling price [%.2f]: ", p->sell_price); fflush(stdout);
            if (utils_read_line(dbuf, sizeof(dbuf)) && dbuf[0]) {
                char *end; double v = strtod(dbuf, &end);
                if (end != dbuf && *end == '\0' && v >= 0) p->sell_price = v;
            }

            printf("  Quantity [%d]: ", p->quantity); fflush(stdout);
            if (utils_read_line(dbuf, sizeof(dbuf)) && dbuf[0]) {
                char *end; long v = strtol(dbuf, &end, 10);
                if (end != dbuf && *end == '\0' && v >= 0) p->quantity = (int)v;
            }

            printf("  Low-stock level [%d]: ", p->low_stock_threshold); fflush(stdout);
            if (utils_read_line(dbuf, sizeof(dbuf)) && dbuf[0]) {
                char *end; long v = strtol(dbuf, &end, 10);
                if (end != dbuf && *end == '\0' && v >= 0) p->low_stock_threshold = (int)v;
            }

            found = 1;
            printf("  Product updated.\n");
            break;
        }
    }
    if (!found) { printf("  Product ID %d not found.\n", id); free(arr); return FAILURE; }

    int rc = save_products(arr, count);
    free(arr);
    return rc;
}

int inventory_delete_product(void) {
    inventory_list_products();
    int id = utils_prompt_int("  Enter Product ID to deactivate: ");

    Product *arr;
    int count = load_products(&arr);
    if (!count) { printf("  No products found.\n"); return FAILURE; }

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (arr[i].id == id && arr[i].active) {
            arr[i].active = 0;
            printf("  Product '%s' deactivated.\n", arr[i].name);
            found = 1;
            break;
        }
    }
    if (!found) { printf("  Active product ID %d not found.\n", id); free(arr); return FAILURE; }

    int rc = save_products(arr, count);
    free(arr);
    return rc;
}

void inventory_list_products(void) {
    utils_print_title("PRODUCT LIST", 80);
    FILE *fp = fopen(PRODUCTS_FILE, "rb");
    if (!fp) { printf("  No products on file.\n"); return; }

    print_product_header();
    Product p;
    int any = 0;
    while (fread(&p, sizeof(Product), 1, fp) == 1) {
        if (p.active) { inventory_print_product(&p); any = 1; }
    }
    fclose(fp);
    if (!any) printf("  No active products.\n");
    utils_print_separator('-', 80);
}

void inventory_search_products(void) {
    char keyword[MAX_NAME_LEN];
    utils_prompt_string("  Search (name or category): ", keyword, sizeof(keyword));

    /* convert keyword to lower-case for case-insensitive search */
    char kw[MAX_NAME_LEN];
    strncpy(kw, keyword, sizeof(kw) - 1);
    kw[sizeof(kw)-1] = '\0';
    for (int i = 0; kw[i]; i++) kw[i] = (char)tolower((unsigned char)kw[i]);

    FILE *fp = fopen(PRODUCTS_FILE, "rb");
    if (!fp) { printf("  No products on file.\n"); return; }

    utils_print_title("SEARCH RESULTS", 80);
    print_product_header();
    Product p;
    int any = 0;
    while (fread(&p, sizeof(Product), 1, fp) == 1) {
        if (!p.active) continue;
        char name[MAX_NAME_LEN], cat[MAX_CATEGORY_LEN];
        strncpy(name, p.name,     sizeof(name) - 1); name[sizeof(name)-1] = '\0';
        strncpy(cat,  p.category, sizeof(cat)  - 1); cat[sizeof(cat)-1]   = '\0';
        for (int i = 0; name[i]; i++) name[i] = (char)tolower((unsigned char)name[i]);
        for (int i = 0; cat[i];  i++) cat[i]  = (char)tolower((unsigned char)cat[i]);
        if (strstr(name, kw) || strstr(cat, kw)) {
            inventory_print_product(&p);
            any = 1;
        }
    }
    fclose(fp);
    if (!any) printf("  No products match '%s'.\n", keyword);
    utils_print_separator('-', 80);
}

void inventory_low_stock_report(void) {
    utils_print_title("LOW STOCK REPORT", 80);
    FILE *fp = fopen(PRODUCTS_FILE, "rb");
    if (!fp) { printf("  No products on file.\n"); return; }

    print_product_header();
    Product p;
    int any = 0;
    while (fread(&p, sizeof(Product), 1, fp) == 1) {
        if (p.active && p.quantity <= p.low_stock_threshold) {
            inventory_print_product(&p);
            any = 1;
        }
    }
    fclose(fp);
    if (!any) printf("  All products are sufficiently stocked.\n");
    utils_print_separator('-', 80);
}

int inventory_find_by_id(int id, Product *out) {
    FILE *fp = fopen(PRODUCTS_FILE, "rb");
    if (!fp) return FAILURE;
    Product p;
    while (fread(&p, sizeof(Product), 1, fp) == 1) {
        if (p.id == id && p.active) {
            *out = p;
            fclose(fp);
            return SUCCESS;
        }
    }
    fclose(fp);
    return FAILURE;
}

int inventory_decrease_stock(int product_id, int qty) {
    Product *arr;
    int count = load_products(&arr);
    if (!count) return FAILURE;

    int rc = FAILURE;
    for (int i = 0; i < count; i++) {
        if (arr[i].id == product_id && arr[i].active) {
            if (arr[i].quantity < qty) { free(arr); return FAILURE; }
            arr[i].quantity -= qty;
            rc = SUCCESS;
            break;
        }
    }
    if (rc == SUCCESS) save_products(arr, count);
    free(arr);
    return rc;
}

int inventory_increase_stock(int product_id, int qty) {
    Product *arr;
    int count = load_products(&arr);
    if (!count) return FAILURE;

    int rc = FAILURE;
    for (int i = 0; i < count; i++) {
        if (arr[i].id == product_id && arr[i].active) {
            arr[i].quantity += qty;
            rc = SUCCESS;
            break;
        }
    }
    if (rc == SUCCESS) save_products(arr, count);
    free(arr);
    return rc;
}
