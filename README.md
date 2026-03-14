# Advanced Retail Super Shop Management System

## Overview
The Advanced Retail Super Shop Management System is a console-based application written in C that simulates the operations of a retail store or supermarket. The system allows administrators to manage products, process sales transactions, track inventory levels, and analyze sales performance.

It also includes authentication, activity logging, and file-based data storage to maintain persistent retail records.

## Features

### Authentication System
- Admin login with username and password
- Password hashing for security
- Automatic admin account initialization

### Product Management
- Add new products to inventory
- View product inventory
- Monitor product quantity
- Automatic low-stock alert when inventory falls below reorder level

### Sales Processing
- Process product sales
- Automatic inventory update after sales
- Record each sale with timestamp and total amount

### Sales Analytics
The system analyzes sales data and provides statistical insights including:
- Total sales count
- Average sale amount
- Variance
- Standard deviation

### Activity Logging
All important operations are logged including:
- Admin login
- Product addition
- Sales processing
- Analytics generation
- System exit

## Technologies Used
- C Programming Language
- Standard C Libraries
- File-based persistent storage

## System Data Files

| File | Purpose |
|-----|-----|
| products.dat | Product inventory records |
| sales.dat | Sales transaction records |
| retail_users.dat | User authentication data |
| retail_logs.txt | System activity logs |

## Default Admin Login

Username: admin  
Password: admin123

## Program Menu

1. Add Product  
2. View Products  
3. Process Sale  
4. Sales Analytics  
5. Exit  

## Learning Objectives

This project demonstrates:

- File handling in C
- Inventory management systems
- Sales processing systems
- Authentication using hashing
- Logging systems
- Statistical analysis
- Console-based retail management applications

## Author
Jisan
