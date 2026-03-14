#include "../include/analytics.h"
#include "../include/sales.h"
#include "../include/inventory.h"
#include "../include/utils.h"

/* ── Internal types ──────────────────────────────────────────────────────── */

typedef struct {
    int    product_id;
    char   name[MAX_NAME_LEN];
    char   category[MAX_CATEGORY_LEN];
    double cost_price;
    double sell_price;
    int    total_qty;
    double total_revenue;
    double total_cost;
} ProductStat;

/* ── Internal helpers ────────────────────────────────────────────────────── */

static void accumulate_stats(const char *date_from, const char *date_to,
                              double *total_revenue, int *total_transactions,
                              int *total_items,
                              ProductStat **p_stats, int *p_count, int *p_cap) {
    FILE *fp = fopen(SALES_FILE, "rb");
    if (!fp) return;

    Sale s;
    while (fread(&s, sizeof(Sale), 1, fp) == 1) {
        if (date_from && date_to &&
            !utils_date_in_range(s.date, date_from, date_to)) continue;

        *total_revenue      += s.total_amount;
        (*total_transactions)++;

        for (int i = 0; i < s.item_count; i++) {
            const SaleItem *it = &s.items[i];
            *total_items += it->quantity;

            /* Find or create entry in ProductStat array */
            int found = 0;
            for (int j = 0; j < *p_count; j++) {
                if ((*p_stats)[j].product_id == it->product_id) {
                    (*p_stats)[j].total_qty     += it->quantity;
                    (*p_stats)[j].total_revenue += it->subtotal;
                    (*p_stats)[j].total_cost    += (*p_stats)[j].cost_price * it->quantity;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                if (*p_count == *p_cap) {
                    *p_cap *= 2;
                    ProductStat *tmp = realloc(*p_stats,
                                               (size_t)(*p_cap) * sizeof(ProductStat));
                    if (!tmp) { fclose(fp); return; }
                    *p_stats = tmp;
                }
                ProductStat *ps = &(*p_stats)[(*p_count)++];
                memset(ps, 0, sizeof(ProductStat));
                ps->product_id     = it->product_id;
                ps->total_qty      = it->quantity;
                ps->total_revenue  = it->subtotal;
                strncpy(ps->name, it->product_name, MAX_NAME_LEN - 1);

                /* Look up cost & category from inventory */
                Product prod;
                if (inventory_find_by_id(it->product_id, &prod) == SUCCESS) {
                    ps->cost_price = prod.cost_price;
                    ps->sell_price = prod.sell_price;
                    strncpy(ps->category, prod.category, MAX_CATEGORY_LEN - 1);
                }
                ps->total_cost = ps->cost_price * it->quantity;
            }
        }
    }
    fclose(fp);
}

/* Simple insertion sort by total_qty descending */
static void sort_stats_by_qty(ProductStat *arr, int n) {
    for (int i = 1; i < n; i++) {
        ProductStat key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j].total_qty < key.total_qty) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

/* ── Public functions ────────────────────────────────────────────────────── */

void analytics_overall_summary(void) {
    utils_print_title("OVERALL SALES SUMMARY", 60);

    int    cap   = 64, count = 0;
    ProductStat *stats = malloc((size_t)cap * sizeof(ProductStat));
    if (!stats) return;

    double total_revenue = 0.0;
    int    total_txn = 0, total_items = 0;

    accumulate_stats(NULL, NULL,
                     &total_revenue, &total_txn, &total_items,
                     &stats, &count, &cap);

    printf("  Total transactions : %d\n",    total_txn);
    printf("  Total items sold   : %d\n",    total_items);
    printf("  Total revenue      : %.2f\n",  total_revenue);

    double total_cost = 0.0;
    for (int i = 0; i < count; i++) total_cost += stats[i].total_cost;
    printf("  Total cost         : %.2f\n",  total_cost);
    printf("  Gross profit       : %.2f\n",  total_revenue - total_cost);
    utils_print_separator('-', 60);
    free(stats);
}

void analytics_date_range_report(void) {
    char from[MAX_DATE_LEN], to[MAX_DATE_LEN];

    utils_print_title("DATE RANGE SALES REPORT", 60);
    utils_prompt_string("  From date (YYYY-MM-DD): ", from, sizeof(from));
    utils_prompt_string("  To   date (YYYY-MM-DD): ", to,   sizeof(to));

    int    cap   = 64, count = 0;
    ProductStat *stats = malloc((size_t)cap * sizeof(ProductStat));
    if (!stats) return;

    double total_revenue = 0.0;
    int    total_txn = 0, total_items = 0;

    accumulate_stats(from, to,
                     &total_revenue, &total_txn, &total_items,
                     &stats, &count, &cap);

    printf("  Period             : %s to %s\n", from, to);
    printf("  Total transactions : %d\n",    total_txn);
    printf("  Total items sold   : %d\n",    total_items);
    printf("  Total revenue      : %.2f\n",  total_revenue);

    double total_cost = 0.0;
    for (int i = 0; i < count; i++) total_cost += stats[i].total_cost;
    printf("  Gross profit       : %.2f\n",  total_revenue - total_cost);
    utils_print_separator('-', 60);
    free(stats);
}

void analytics_top_products(int top_n) {
    int    cap   = 64, count = 0;
    ProductStat *stats = malloc((size_t)cap * sizeof(ProductStat));
    if (!stats) return;

    double dummy_rev = 0.0;
    int    dummy_txn = 0, dummy_items = 0;
    accumulate_stats(NULL, NULL, &dummy_rev, &dummy_txn, &dummy_items,
                     &stats, &count, &cap);

    sort_stats_by_qty(stats, count);

    utils_print_title("TOP SELLING PRODUCTS", 70);
    printf("  %-4s %-22s %-12s %8s %10s\n",
           "Rank", "Product", "Category", "Qty Sold", "Revenue");
    utils_print_separator('-', 70);

    int limit = (count < top_n) ? count : top_n;
    for (int i = 0; i < limit; i++) {
        printf("  %-4d %-22s %-12s %8d %10.2f\n",
               i + 1,
               stats[i].name,
               stats[i].category,
               stats[i].total_qty,
               stats[i].total_revenue);
    }
    if (limit == 0) printf("  No sales data available.\n");
    utils_print_separator('-', 70);
    free(stats);
}

void analytics_revenue_by_category(void) {
    /* Accumulate all product stats first */
    int    cap   = 64, count = 0;
    ProductStat *stats = malloc((size_t)cap * sizeof(ProductStat));
    if (!stats) return;

    double dummy_rev = 0.0;
    int    dummy_txn = 0, dummy_items = 0;
    accumulate_stats(NULL, NULL, &dummy_rev, &dummy_txn, &dummy_items,
                     &stats, &count, &cap);

    /* Aggregate by category using a simple O(n^2) pass (small data sets) */
    typedef struct { char cat[MAX_CATEGORY_LEN]; double revenue; } CatStat;
    int ccat_cap = 32, ccat_count = 0;
    CatStat *cats = malloc((size_t)ccat_cap * sizeof(CatStat));
    if (!cats) { free(stats); return; }

    for (int i = 0; i < count; i++) {
        int found = 0;
        for (int j = 0; j < ccat_count; j++) {
            if (strcmp(cats[j].cat, stats[i].category) == 0) {
                cats[j].revenue += stats[i].total_revenue;
                found = 1;
                break;
            }
        }
        if (!found) {
            if (ccat_count == ccat_cap) {
                ccat_cap *= 2;
                CatStat *tmp = realloc(cats, (size_t)ccat_cap * sizeof(CatStat));
                if (!tmp) { free(cats); free(stats); return; }
                cats = tmp;
            }
            strncpy(cats[ccat_count].cat, stats[i].category, MAX_CATEGORY_LEN - 1);
            cats[ccat_count].revenue = stats[i].total_revenue;
            ccat_count++;
        }
    }

    utils_print_title("REVENUE BY CATEGORY", 50);
    printf("  %-20s %10s\n", "Category", "Revenue");
    utils_print_separator('-', 50);
    for (int j = 0; j < ccat_count; j++) {
        printf("  %-20s %10.2f\n", cats[j].cat, cats[j].revenue);
    }
    if (ccat_count == 0) printf("  No sales data available.\n");
    utils_print_separator('-', 50);

    free(cats);
    free(stats);
}

void analytics_profit_report(void) {
    int    cap   = 64, count = 0;
    ProductStat *stats = malloc((size_t)cap * sizeof(ProductStat));
    if (!stats) return;

    double dummy_rev = 0.0;
    int    dummy_txn = 0, dummy_items = 0;
    accumulate_stats(NULL, NULL, &dummy_rev, &dummy_txn, &dummy_items,
                     &stats, &count, &cap);

    /* Also pull current inventory for cost/price data */
    utils_print_title("PROFIT MARGIN REPORT", 80);
    printf("  %-4s %-22s %8s %8s %8s %8s %10s\n",
           "ID", "Product", "Cost", "Price", "Margin%", "QtySold", "Profit");
    utils_print_separator('-', 80);

    double grand_profit = 0.0;
    for (int i = 0; i < count; i++) {
        ProductStat *ps = &stats[i];
        double margin = (ps->sell_price > 0.0)
                        ? (ps->sell_price - ps->cost_price) / ps->sell_price * 100.0
                        : 0.0;
        double profit = ps->total_revenue - ps->total_cost;
        grand_profit += profit;
        printf("  %-4d %-22s %8.2f %8.2f %7.1f%% %8d %10.2f\n",
               ps->product_id, ps->name,
               ps->cost_price, ps->sell_price, margin,
               ps->total_qty, profit);
    }
    if (count == 0) printf("  No sales data available.\n");
    utils_print_separator('-', 80);
    printf("  Grand total profit: %.2f\n", grand_profit);
    utils_print_separator('=', 80);
    free(stats);
}
