# retail-super-shop-management-system

A C-based retail management system that supports product inventory management, sales processing, authentication, and sales analytics using file-based storage.

---

## Features

| Module | Capabilities |
|---|---|
| **Authentication** | Login / logout, role-based access (Admin / Cashier), add users, toggle user status, change passwords |
| **Inventory** | Add / edit / deactivate products, list all products, keyword search, low-stock report |
| **Sales** | Interactive shopping cart, stock validation, change calculation, receipt printing, view / list past sales |
| **Analytics** | Overall summary, date-range report, top-N selling products, revenue by category, profit-margin report |

All data is persisted in binary flat files under the `data/` directory (created automatically on first run).

---

## Requirements

* GCC (or any C11-compatible compiler)
* POSIX-compatible OS (Linux / macOS)
* `make`

---

## Build

```bash
make
```

This produces the `retail_shop` executable.

To remove build artefacts:

```bash
make clean
```

---

## Run

```bash
./retail_shop
```

On the **first run** the system creates a default administrator account:

| Field | Value |
|---|---|
| Username | `admin` |
| Password | `admin123` |

> **Important:** change the default password immediately after first login via *User Management → Change User Password*.

---

## Menus

### Admin Menu

```
1. Inventory Management
2. Sales
3. Analytics & Reports
4. User Management
5. Logout
```

### Cashier Menu

```
1. New Sale
2. View Sale by ID
3. Recent Sales
4. Product List
5. Search Products
6. Logout
```

---

## Project Structure

```
.
├── Makefile
├── main.c                  # Entry point and menu dispatcher
├── include/
│   ├── common.h            # Shared constants and includes
│   ├── auth.h              # Authentication API
│   ├── inventory.h         # Inventory API
│   ├── sales.h             # Sales API
│   ├── analytics.h         # Analytics API
│   └── utils.h             # Utility helpers
└── src/
    ├── auth.c
    ├── inventory.c
    ├── sales.c
    ├── analytics.c
    └── utils.c
```

Runtime data files (auto-created in `data/`):

| File | Contents |
|---|---|
| `data/users.dat` | User accounts (binary) |
| `data/products.dat` | Product records (binary) |
| `data/sales.dat` | Sales transactions (binary) |

---

## License

MIT
