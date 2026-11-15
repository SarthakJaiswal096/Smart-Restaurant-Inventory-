#ifndef INVENTORY_H
#define INVENTORY_H

#include "models.h"

int inventoryDateNumber(int date)
{
    int year = date / 10000, month = (date / 100) % 100, day = date % 100;
    int total = year * 365 + year / 4 - year / 100 + year / 400;
    int monthDays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) 
        monthDays[1] = 29;
    for (int i = 0; i < month - 1; i++) {
        total += monthDays[i];
    }
    return total + day;
}

class BatchMinHeap
{
private:
    Batch data[500];
    int size;

public:
    BatchMinHeap() { 
        size = 0; 
    }
    bool empty() { 
        return size == 0; 
    }

    void insert(Batch b)
    {
        if (size == 500) return;
        int i = size++;
        data[i] = b;
        while (i > 0){
            int parent = (i - 1) / 2;
            if (data[parent].expiry <= data[i].expiry) break;
            Batch temp = data[parent];
            data[parent] = data[i];
            data[i] = temp;
            i = parent;
        }
    }

    Batch removeTop()
    {
        Batch result = data[0];
        data[0] = data[size - 1];
        size--;
        int i = 0;
        while (i < size)
        {
            int left = 2 * i + 1, right = 2 * i + 2, small = i;
            if (left < size && data[left].expiry < data[small].expiry) small = left;
            if (right < size && data[right].expiry < data[small].expiry) small = right;
            if (small == i) break;
            Batch temp = data[i];
            data[i] = data[small];
            data[small] = temp;
            i = small;
        }
        return result;
    }
};

class StockUndoStack
{
private:
    Batch data[200];
    int top;

public:
    StockUndoStack() { 
        top = -1; 
    }
    bool empty() { 
        return top == -1; 
    }
    void push(Batch b) { 
        if (top < 199){ 
            data[++top] = b; 
    }
    Batch pop() { 
        return empty() ? Batch() : data[top--]; 
    }
};

class Inventory
{
private:
    vector<Ingredient> ingredients;
    vector<Batch> batches;
    vector<StockTransaction> transactions;
    StockUndoStack undo;

public:
    void addIngredient(int id, string name, string unit, int minimum, int cost = 0)
    {
        if (findIngredient(id) == NULL)
            ingredients.push_back(Ingredient(id, name, unit, minimum, cost));
    }

    bool addStock(int batchId, int ingredientId, int quantity, int expiry, int date)
    {
        if (findIngredient(ingredientId) == NULL || quantity <= 0)
        {
            cout << "Check ingredient and quantity.\n";
            return false;
        }
        Batch b(batchId, ingredientId, quantity, expiry);
        batches.push_back(b);
        undo.push(b);
        transactions.push_back(StockTransaction("RECEIVED", ingredientId, quantity, date, "New batch"));
        cout << "Batch added.\n";
        return true;
    }

    Ingredient* findIngredient(int id)
    {
        for (int i = 0; i < (int)ingredients.size(); i++)
            if (ingredients[i].getId() == id) return &ingredients[i];
        return NULL;
    }

    int getStock(int ingredientId)
    {
        int total = 0;
        for (int i = 0; i < (int)batches.size(); i++)
            if (batches[i].ingredientId == ingredientId) total += batches[i].quantity;
        return total;
    }

    int getUsableStock(int ingredientId, int today)
    {
        int total = 0;
        for (int i = 0; i < (int)batches.size(); i++)
            if (batches[i].ingredientId == ingredientId && batches[i].expiry >= today)
                total += batches[i].quantity;
        return total;
    }

    void showStock(int today)
    {
        cout << "\n----- INVENTORY -----\n";
        for (int i = 0; i < (int)ingredients.size(); i++)
        {
            int id = ingredients[i].getId();
            int usable = getUsableStock(id, today);
            cout << ingredients[i].getName() << " : " << usable << " usable " << ingredients[i].getUnit();
            if (usable < ingredients[i].getMinimumStock()) cout << "  (LOW STOCK)";
            cout << "\n";
        }
    }

    void showBatches(int today)
    {
        cout << "\n----- STOCK BATCHES -----\n";
        for (int i = 0; i < (int)batches.size(); i++)
        {
            cout << "Batch " << batches[i].id << " | Ingredient " << batches[i].ingredientId
                 << " | Qty " << batches[i].quantity << " | Expiry " << batches[i].expiry;
            if (batches[i].expiry < today) cout << " | EXPIRED - DO NOT USE";
            else if (inventoryDateNumber(batches[i].expiry) - inventoryDateNumber(today) <= 7) cout << " | EXPIRING SOON";
            cout << "\n";
        }
    }

    bool useStock(int ingredientId, int quantity, int today)
    {
        if (quantity <= 0 || getUsableStock(ingredientId, today) < quantity)
        {
            cout << "Not enough usable stock. Expired batches are not counted.\n";
            return false;
        }
        BatchMinHeap heap;
        for (int i = 0; i < (int)batches.size(); i++)
            if (batches[i].ingredientId == ingredientId && batches[i].quantity > 0 && batches[i].expiry >= today)
                heap.insert(batches[i]);

        int original = quantity;
        while (quantity > 0 && !heap.empty())
        {
            Batch b = heap.removeTop();
            int take = (b.quantity > quantity) ? quantity : b.quantity;
            for (int i = 0; i < (int)batches.size(); i++)
            {
                if (batches[i].id == b.id)
                {
                    batches[i].quantity -= take;
                    break;
                }
            }
            quantity -= take;
        }
        transactions.push_back(StockTransaction("USED", ingredientId, original, today, "FEFO stock use"));
        cout << "Stock used with FEFO.\n";
        return true;
    }

    bool recordWastage(int ingredientId, int quantity, int today, string note)
    {
        if (quantity <= 0 || getStock(ingredientId) < quantity)
        {
            cout << "Not enough stock for wastage entry.\n";
            return false;
        }
        int left = quantity;
        for (int i = 0; i < (int)batches.size() && left > 0; i++)
        {
            if (batches[i].ingredientId == ingredientId)
            {
                int take = (batches[i].quantity > left) ? left : batches[i].quantity;
                batches[i].quantity -= take;
                left -= take;
            }
        }
        transactions.push_back(StockTransaction("WASTAGE", ingredientId, quantity, today, note));
        cout << "Wastage recorded.\n";
        return true;
    }

    int writeOffExpired(int today)
    {
        int count = 0;
        for (int i = 0; i < (int)batches.size(); i++)
        {
            if (batches[i].quantity > 0 && batches[i].expiry < today)
            {
                transactions.push_back(StockTransaction("EXPIRED", batches[i].ingredientId, batches[i].quantity, today, "Expired batch"));
                batches[i].quantity = 0;
                count++;
            }
        }
        cout << "Expired stock checked and written off.\n";
        return count;
    }

    bool undoLastStock(int today)
    {
        if (undo.empty())
        {
            cout << "Nothing to undo.\n";
            return false;
        }
        Batch last = undo.pop();
        for (int i = 0; i < (int)batches.size(); i++)
        {
            if (batches[i].id == last.id)
            {
                if (batches[i].quantity == last.quantity)
                {
                    batches[i].quantity = 0;
                    transactions.push_back(StockTransaction("UNDO", last.ingredientId, last.quantity, today, "Stock receipt undone"));
                    cout << "Last stock addition undone.\n";
                    return true;
                }
                cout << "This batch has already been changed, so it cannot be fully undone.\n";
                return false;
            }
        }
        return false;
    }

    vector<Ingredient>& getIngredients() { 
        return ingredients; 
    }
    vector<Batch>& getBatches(){ 
        return batches; 
    }
    vector<StockTransaction>& getTransactions() { 
        return transactions; 
    }
    void setTransactions(vector<StockTransaction>& v) { 
        transactions = v; 
    }
    void rebuildUndo() { 
        undo = StockUndoStack(); 
    }
};

#endif
