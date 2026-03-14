#ifndef ANALYTICS_H
#define ANALYTICS_H

#include "common.h"

/* ── Function prototypes ─────────────────────────────────────────────────── */

/* Summary of all-time totals */
void analytics_overall_summary(void);

/* Sales report for a given date range (YYYY-MM-DD) */
void analytics_date_range_report(void);

/* Top N best-selling products by quantity */
void analytics_top_products(int top_n);

/* Revenue by category */
void analytics_revenue_by_category(void);

/* Profit margin report (sell_price vs cost_price) */
void analytics_profit_report(void);

#endif /* ANALYTICS_H */
