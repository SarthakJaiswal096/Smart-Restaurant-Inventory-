#ifndef MODELS_H
#define MODELS_H

#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Item
{
protected:
    int id;
    string name;

public:
    Item()
    {
        id = 0;
        name = "";
    }
    Item(int i, string n)
    {
        id = i;
        name = n;
    }
    int getId() { 
        return id; 
    }
    string getName() {
         return name; 
        }
    void setName(string n) { 
        name = n;
     }
    virtual void show() { 
        cout << id << " - " << name;
    }
    virtual ~Item(){
    }
};

class Food : public Item
{
private:
    int price;

public:
    Food() : Item() {
         price = 0; 
        }
    Food(int i, string n, int p) : Item(i, n) { 
        price = p;
     }
    int getPrice() { 
        return price;
     }
    void setPrice(int p) {
         price = p;
         }
    void show() {
         cout << id << " - " << name << " (Rs. " << price << ")"; 
        }
};

class Ingredient : public Item
{
private:
    string unit;
    int minimumStock;
    int costPerUnit;

public:
    Ingredient() : Item(){
        unit = "g";
        minimumStock = 0;
        costPerUnit = 0;
    }
    Ingredient(int i, string n, string u, int m, int c = 0) : Item(i, n){
        unit = u;
        minimumStock = m;
        costPerUnit = c;
    }
    string getUnit() { 
        return unit;
     }
    int getMinimumStock() { 
        return minimumStock; 
    }
    int getCostPerUnit() { 
        return costPerUnit; 
    }
    void setMinimumStock(int m) {
         minimumStock = m;
         }
    void setCostPerUnit(int c) {
         costPerUnit = c; 
        }
};

struct Batch
{
    int id, ingredientId, quantity, expiry;
    Batch(int b = 0, int g = 0, int q = 0, int e = 0){
        id = b;
        ingredientId = g;
        quantity = q;
        expiry = e;
    }
};

struct OrderLine
{
    int foodId, quantity;
    OrderLine(int f = 0, int q = 0)
    {
        foodId = f;
        quantity = q;
    }
};

class Order
{
private:
    int id;
    string status;
    int orderDate;
    vector<OrderLine> items;

public:
    Order(int i = 0, int d = 20261114){
        id = i;
        status = "PENDING";
        orderDate = d;
    }
    int getId() { return id; }
    string getStatus() { return status; }
    int getDate() { return orderDate; }
    void setStatus(string s) { status = s; }
    void setDate(int d) { orderDate = d; }
    vector<OrderLine>& getItems() { return items; }
    void addFood(int foodId, int qty) { items.push_back(OrderLine(foodId, qty)); }
};

struct StockTransaction
{
    string type;
    int ingredientId, quantity, date;
    string note;
    StockTransaction(string t = "", int id = 0, int q = 0, int d = 20261114, string n = "")
    {
        type = t;
        ingredientId = id;
        quantity = q;
        date = d;
        note = n;
    }
};

#endif
