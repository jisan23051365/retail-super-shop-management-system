#include "../include/sales.h"
#include "../include/inventory.h"
#include "../include/auth.h"
#include "../include/utils.h"

/* ── Internal helpers ────────────────────────────────────────────────────── */

static int next_sale_id(void) {
    FILE *fp = fopen(SALES_FILE, "rb");
    if (!fp) return 1;
    int max_id = 0;
    Sale s;
    while (fread(&s, sizeof(Sale), 1, fp) == 1) {
        if (s.id > max_id) max_id = s.id;
    }
    fclose(fp);
    return max_id + 1;
}

/* ── Public functions ────────────────────────────────────────────────────── */

void sales_print_receipt(const Sale *s) {
    utils_print_title("RECEIPT", 56);
    printf("  Sale ID   : %d\n", s->id);
    printf("  Cashier   : %s\n", s->cashier_name);
    printf("  Date/Time : %s\n", s->date);
    utils_print_separator('-', 56);
    printf("  %-4s %-18s %5s %8s %10s\n",
           "ID", "Product", "Qty", "Price", "Subtotal");
    utils_print_separator('-', 56);
    for (int i = 0; i < s->item_count; i++) {
        const SaleItem *it = &s->items[i];
        printf("  %-4d %-18s %5d %8.2f %10.2f\n",
               it->product_id, it->product_name,
               it->quantity, it->unit_price, it->subtotal);
    }
    utils_print_separator('-', 56);
    printf("  %-28s %10s %10.2f\n", "", "TOTAL:", s->total_amount);
    printf("  %-28s %10s %10.2f\n", "", "PAID:",  s->amount_paid);
    printf("  %-28s %10s %10.2f\n", "", "CHANGE:", s->change_due);
    utils_print_separator('=', 56);
}

int sales_new_sale(void) {
    Sale sale;
    memset(&sale, 0, sizeof(Sale));
    sale.id          = next_sale_id();
    sale.cashier_id  = current_user.id;
    strncpy(sale.cashier_name, current_user.username, MAX_USERNAME_LEN - 1);
    utils_current_datetime(sale.date, sizeof(sale.date));

    utils_print_title("NEW SALE", 56);
    printf("  Enter product IDs to add (0 to finish).\n\n");

    /* ── Shopping cart loop ──────────────────────────────────────────────── */
    while (sale.item_count < MAX_SALE_ITEMS) {
        inventory_list_products();

        int pid = utils_prompt_int("  Product ID (0 = done): ");
        if (pid == 0) break;

        Product p;
        if (inventory_find_by_id(pid, &p) != SUCCESS) {
            printf("  Product ID %d not found or inactive.\n", pid);
            continue;
        }
        if (p.quantity == 0) {
            printf("  '%s' is out of stock.\n", p.name);
            continue;
        }

        printf("  Available: %d  |  Price: %.2f\n", p.quantity, p.sell_price);
        int qty = utils_prompt_positive_int("  Quantity: ");
        if (qty > p.quantity) {
            printf("  Not enough stock (available: %d).\n", p.quantity);
            continue;
        }

        /* Check if this product already in cart, if so add qty */
        int found_in_cart = 0;
        for (int i = 0; i < sale.item_count; i++) {
            if (sale.items[i].product_id == pid) {
                int new_qty = sale.items[i].quantity + qty;
                if (new_qty > p.quantity) {
                    printf("  Cannot add %d more (total would exceed stock of %d).\n",
                           qty, p.quantity);
                    found_in_cart = 1;
                    break;
                }
                sale.items[i].quantity  = new_qty;
                sale.items[i].subtotal  = new_qty * sale.items[i].unit_price;
                sale.total_amount      += qty * sale.items[i].unit_price;
                found_in_cart = 1;
                printf("  Updated cart: %s x%d\n", p.name, new_qty);
                break;
            }
        }
        if (!found_in_cart) {
            SaleItem *it = &sale.items[sale.item_count++];
            it->product_id = pid;
            strncpy(it->product_name, p.name, MAX_NAME_LEN - 1);
            it->quantity   = qty;
            it->unit_price = p.sell_price;
            it->subtotal   = qty * p.sell_price;
            sale.total_amount += it->subtotal;
            printf("  Added: %s x%d @ %.2f = %.2f\n",
                   p.name, qty, p.sell_price, it->subtotal);
        }
    }

    if (sale.item_count == 0) {
        printf("  No items in cart. Sale cancelled.\n");
        return FAILURE;
    }

    /* Print cart summary */
    utils_print_separator('-', 56);
    printf("  Cart total: %.2f\n", sale.total_amount);

    /* Payment */
    double paid;
    while (1) {
        paid = utils_prompt_double("  Amount paid: ");
        if (paid >= sale.total_amount) break;
        printf("  Insufficient payment. Total is %.2f.\n", sale.total_amount);
    }
    sale.amount_paid = paid;
    sale.change_due  = paid - sale.total_amount;

    /* Deduct stock */
    for (int i = 0; i < sale.item_count; i++) {
        if (inventory_decrease_stock(sale.items[i].product_id,
                                     sale.items[i].quantity) != SUCCESS) {
            printf("  Warning: could not update stock for product %d.\n",
                   sale.items[i].product_id);
        }
    }

    /* Persist sale */
    utils_ensure_data_dir();
    FILE *fp = fopen(SALES_FILE, "ab");
    if (!fp) { perror("Cannot write sales file"); return FAILURE; }
    fwrite(&sale, sizeof(Sale), 1, fp);
    fclose(fp);

    sales_print_receipt(&sale);
    return SUCCESS;
}

void sales_view_sale(void) {
    int id = utils_prompt_int("  Enter Sale ID: ");
    FILE *fp = fopen(SALES_FILE, "rb");
    if (!fp) { printf("  No sales records found.\n"); return; }

    Sale s;
    int found = 0;
    while (fread(&s, sizeof(Sale), 1, fp) == 1) {
        if (s.id == id) { found = 1; break; }
    }
    fclose(fp);

    if (!found) printf("  Sale ID %d not found.\n", id);
    else        sales_print_receipt(&s);
}

void sales_list_recent(int n) {
    FILE *fp = fopen(SALES_FILE, "rb");
    if (!fp) { printf("  No sales records found.\n"); return; }

    /* Load all */
    Sale *arr = NULL;
    int count = 0, cap = 64;
    arr = malloc((size_t)cap * sizeof(Sale));
    if (!arr) { fclose(fp); return; }

    Sale s;
    while (fread(&s, sizeof(Sale), 1, fp) == 1) {
        if (count == cap) {
            cap *= 2;
            Sale *tmp = realloc(arr, (size_t)cap * sizeof(Sale));
            if (!tmp) { free(arr); fclose(fp); return; }
            arr = tmp;
        }
        arr[count++] = s;
    }
    fclose(fp);

    utils_print_title("RECENT SALES", 80);
    printf("  %-6s %-20s %-20s %10s\n", "ID", "Cashier", "Date/Time", "Total");
    utils_print_separator('-', 80);

    int start = (count > n) ? count - n : 0;
    for (int i = start; i < count; i++) {
        printf("  %-6d %-20s %-20s %10.2f\n",
               arr[i].id, arr[i].cashier_name,
               arr[i].date, arr[i].total_amount);
    }
    utils_print_separator('-', 80);
    free(arr);
}
