#ifndef REPORT_H
#define REPORT_H

#include "models.h"
#include "menu.h"
#include "order.h"
#include "inventory.h"

class Report
{
public:
    virtual void show() = 0;
    virtual ~Report() {}
};

struct FoodSale
{
    string name;
    int plates;
    int amount;
};

void mergeSales(vector<FoodSale>& sales, int left, int mid, int right)
{
    vector<FoodSale> temp;
    int i = left, j = mid + 1;
    while (i <= mid && j <= right)
    {
        if (sales[i].amount >= sales[j].amount) temp.push_back(sales[i++]);
        else temp.push_back(sales[j++]);
    }
    while (i <= mid) temp.push_back(sales[i++]);
    while (j <= right) temp.push_back(sales[j++]);
    for (int k = 0; k < (int)temp.size(); k++) sales[left + k] = temp[k];
}

void mergeSortSales(vector<FoodSale>& sales, int left, int right)
{
    if (left >= right) return;
    int mid = (left + right) / 2;
    mergeSortSales(sales, left, mid);
    mergeSortSales(sales, mid + 1, right);
    mergeSales(sales, left, mid, right);
}

class SalesReport : public Report
{
private:
    FoodMenu *menu;
    OrderManager *orders;

public:
    SalesReport(FoodMenu *m, OrderManager *o)
    {
        menu = m;
        orders = o;
    }

    void perDay(int date)
    {
        int total = 0;
        vector<Order>& all = orders->getOrders();
        for (int i = 0; i < (int)all.size(); i++)
            if (all[i].getDate() == date && all[i].getStatus() == "CONFIRMED")
            {
                vector<OrderLine>& lines = all[i].getItems();
                for (int j = 0; j < (int)lines.size(); j++)
                {
                    Food *food = menu->getFood(lines[j].foodId);
                    if (food != NULL) total += food->getPrice() * lines[j].quantity;
                }
            }
        cout << "Sales on " << date << " = Rs. " << total << "\n";
    }

    void perFood()
    {
        cout << "\n----- SALES PER FOOD -----\n";
        vector<Food>& foods = menu->getFoods();
        vector<Order>& all = orders->getOrders();
        vector<FoodSale> sales;
        for (int f = 0; f < (int)foods.size(); f++)
        {
            FoodSale row = {foods[f].getName(), 0, 0};
            for (int i = 0; i < (int)all.size(); i++)
                if (all[i].getStatus() == "CONFIRMED")
                {
                    vector<OrderLine>& lines = all[i].getItems();
                    for (int j = 0; j < (int)lines.size(); j++)
                        if (lines[j].foodId == foods[f].getId())
                        {
                            row.plates += lines[j].quantity;
                            row.amount += lines[j].quantity * foods[f].getPrice();
                        }
                }
            sales.push_back(row);
        }
        if (!sales.empty()) mergeSortSales(sales, 0, (int)sales.size() - 1);
        for (int i = 0; i < (int)sales.size(); i++)
            cout << sales[i].name << " | Plates: " << sales[i].plates << " | Sales: Rs. " << sales[i].amount << "\n";
    }

    void show() { perFood(); }
};

class StockReport : public Report
{
private:
    Inventory *inventory;
    int today;

public:
    StockReport(Inventory *i, int d)
    {
        inventory = i;
        today = d;
    }

    void show()
    {
        cout << "\n----- STOCK VALUE REPORT -----\n";
        vector<Ingredient>& items = inventory->getIngredients();
        int totalValue = 0;
        for (int i = 0; i < (int)items.size(); i++)
        {
            int usable = inventory->getUsableStock(items[i].getId(), today);
            int value = usable * items[i].getCostPerUnit();
            totalValue += value;
            cout << items[i].getName() << " | Usable Qty: " << usable << " | Value: Rs. " << value << "\n";
        }
        cout << "Total stock value: Rs. " << totalValue << "\n";
    }
};

class ExpiryReport : public Report
{
private:
    Inventory *inventory;
    int today;

public:
    ExpiryReport(Inventory *i, int d)
    {
        inventory = i;
        today = d;
    }

    void show()
    {
        cout << "\n----- EXPIRY REPORT -----\n";
        vector<Batch>& batches = inventory->getBatches();
        for (int i = 0; i < (int)batches.size(); i++)
        {
            if (batches[i].quantity <= 0) continue;
            bool expired = batches[i].expiry < today;
            bool soon = !expired && inventoryDateNumber(batches[i].expiry) - inventoryDateNumber(today) <= 7;
            if (expired || soon)
            {
                cout << "Batch " << batches[i].id << " | Ingredient " << batches[i].ingredientId<< " | Qty " << batches[i].quantity << " | Expiry " << batches[i].expiry;
                cout << (expired ? " | EXPIRED" : " | EXPIRING SOON") << "\n";
            }
        }
    }
};

class LowStockReport : public Report
{
private:
    Inventory *inventory;
    int today;

public:
    LowStockReport(Inventory *i, int d)
    {
        inventory = i;
        today = d;
    }

    void show()
    {
        cout << "\n----- LOW STOCK REPORT -----\n";
        vector<Ingredient>& items = inventory->getIngredients();
        for (int i = 0; i < (int)items.size(); i++)
        {
            int usable = inventory->getUsableStock(items[i].getId(), today);
            if (usable < items[i].getMinimumStock())
                cout << items[i].getName() << " | Current usable stock: " << usable<< " | Minimum: " << items[i].getMinimumStock() << "\n";
        }
    }
};

class WastageReport : public Report
{
private:
    Inventory *inventory;

public:
    WastageReport(Inventory *i) { inventory = i; }

    void show()
    {
        cout << "\n----- WASTAGE REPORT -----\n";
        vector<StockTransaction>& log = inventory->getTransactions();
        int total = 0;
        for (int i = 0; i < (int)log.size(); i++)
            if (log[i].type == "WASTAGE" || log[i].type == "EXPIRED")
            {
                cout << log[i].type << " | Ingredient " << log[i].ingredientId<< " | Qty " << log[i].quantity << " | " << log[i].date << "\n";
                total += log[i].quantity;
            }
        cout << "Total wasted quantity: " << total << "\n";
    }
};

#endif
