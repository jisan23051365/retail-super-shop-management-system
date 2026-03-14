#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* =========================================================
   ADVANCED RETAIL SUPER SHOP MANAGEMENT SYSTEM
   ========================================================= */

typedef struct {
    int id;
    char username[30];
    unsigned long password_hash;
} User;

typedef struct {
    int product_id;
    char name[50];
    int quantity;
    float price;
    int reorder_level;
} Product;

typedef struct {
    int sale_id;
    int product_id;
    int quantity;
    float total_amount;
    char timestamp[30];
} Sale;

/* ===================== UTILITIES ===================== */

unsigned long hashPassword(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

void logActivity(const char *msg) {
    FILE *fp = fopen("retail_logs.txt", "a");
    if (!fp) return;
    time_t now = time(NULL);
    fprintf(fp, "%s - %s\n", ctime(&now), msg);
    fprintf(fp, "--------------------------------\n");
    fclose(fp);
}

/* ===================== AUTH ===================== */

void initializeAdmin() {
    FILE *fp = fopen("retail_users.dat", "rb");
    if (fp) { fclose(fp); return; }

    fp = fopen("retail_users.dat", "wb");
    User admin = {1, "admin", hashPassword("admin123")};
    fwrite(&admin, sizeof(User), 1, fp);
    fclose(fp);
}

int login() {
    char username[30], password[30];
    unsigned long hash;
    User u;

    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);

    hash = hashPassword(password);

    FILE *fp = fopen("retail_users.dat", "rb");
    if (!fp) return 0;

    while (fread(&u, sizeof(User), 1, fp)) {
        if (strcmp(u.username, username) == 0 && u.password_hash == hash) {
            fclose(fp);
            logActivity("Admin login successful.");
            return 1;
        }
    }

    fclose(fp);
    printf("Invalid credentials.\n");
    return 0;
}

/* ===================== PRODUCT MANAGEMENT ===================== */

void addProduct() {
    FILE *fp = fopen("products.dat", "ab");
    if (!fp) return;

    Product p;
    printf("Product ID: "); scanf("%d", &p.product_id);
    printf("Product Name: "); scanf(" %[^\n]", p.name);
    printf("Quantity: "); scanf("%d", &p.quantity);
    printf("Price: "); scanf("%f", &p.price);
    printf("Reorder Level: "); scanf("%d", &p.reorder_level);

    fwrite(&p, sizeof(Product), 1, fp);
    fclose(fp);
    logActivity("Product added.");
}

void viewProducts() {
    FILE *fp = fopen("products.dat", "rb");
    if (!fp) return;

    Product p;
    printf("\n--- Product Inventory ---\n");
    while (fread(&p, sizeof(Product), 1, fp)) {
        printf("ID:%d | %s | Qty:%d | Price:%.2f",
               p.product_id, p.name, p.quantity, p.price);
        if (p.quantity <= p.reorder_level)
            printf("  **LOW STOCK**");
        printf("\n");
    }
    fclose(fp);
}

/* ===================== SALES PROCESSING ===================== */

void processSale() {
    FILE *pfp = fopen("products.dat", "rb+");
    FILE *sfp = fopen("sales.dat", "ab");
    if (!pfp || !sfp) return;

    int id, qty;
    Product p;

    printf("Product ID: "); scanf("%d", &id);
    printf("Quantity: "); scanf("%d", &qty);

    while (fread(&p, sizeof(Product), 1, pfp)) {
        if (p.product_id == id) {
            if (p.quantity >= qty) {
                p.quantity -= qty;
                fseek(pfp, -sizeof(Product), SEEK_CUR);
                fwrite(&p, sizeof(Product), 1, pfp);

                Sale s;
                s.sale_id = rand() % 100000;
                s.product_id = id;
                s.quantity = qty;
                s.total_amount = qty * p.price;
                strcpy(s.timestamp, ctime(&(time_t){time(NULL)}));

                fwrite(&s, sizeof(Sale), 1, sfp);
                printf("Sale completed. Total: %.2f\n", s.total_amount);
                logActivity("Sale processed.");
            } else {
                printf("Insufficient stock.\n");
            }
            break;
        }
    }

    fclose(pfp);
    fclose(sfp);
}

/* ===================== ANALYTICS ===================== */

void salesAnalytics() {
    FILE *fp = fopen("sales.dat", "rb");
    if (!fp) return;

    Sale s;
    float arr[500];
    int n = 0;

    while (fread(&s, sizeof(Sale), 1, fp) && n < 500) {
        arr[n++] = s.total_amount;
    }
    fclose(fp);

    if (n == 0) {
        printf("No sales data.\n");
        return;
    }

    float sum = 0;
    for (int i = 0; i < n; i++) sum += arr[i];
    float mean = sum / n;

    float variance = 0;
    for (int i = 0; i < n; i++)
        variance += pow(arr[i] - mean, 2);
    variance /= n;

    float std = sqrt(variance);

    printf("\n--- Sales Analytics ---\n");
    printf("Total Sales: %d\n", n);
    printf("Average Sale: %.2f\n", mean);
    printf("Variance: %.2f\n", variance);
    printf("Standard Deviation: %.2f\n", std);

    logActivity("Sales analytics generated.");
}

/* ===================== MAIN ===================== */

int main() {
    initializeAdmin();

    printf("=== RETAIL SUPER SHOP SYSTEM ===\n");

    if (!login()) return 0;

    int choice;

    while (1) {
        printf("\n1.Add Product\n");
        printf("2.View Products\n");
        printf("3.Process Sale\n");
        printf("4.Sales Analytics\n");
        printf("5.Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addProduct(); break;
            case 2: viewProducts(); break;
            case 3: processSale(); break;
            case 4: salesAnalytics(); break;
            case 5:
                logActivity("System exited.");
                exit(0);
            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;
}