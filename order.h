#ifndef ORDER_H
#define ORDER_H

#include "models.h"
#include "recipe.h"
#include "inventory.h"

class OrderCircularQueue
{
private:
    int data[200];
    int front, rear, count;

public:
    OrderCircularQueue() { 
        front = rear = count = 0; 
    }
    bool empty() { 
        return count == 0; 
    }
    int size() { 
        return count;
    }
    void clear() { 
        front = rear = count = 0; 
    }

    void push(int orderId)
    {
        if (count == 200) return;
        data[rear] = orderId;
        rear = (rear + 1) % 200;
        count++;
    }

    int pop()
    {
        if (empty()) return -1;
        int val = data[front];
        front = (front + 1) % 200;
        count--;
        return val;
    }
};

class OrderManager
{
private:
    vector<Order> orders;
    OrderCircularQueue queue;
    int nextId;

    // Calculates total ingredients for the order and consumes them in FIFO order
    bool consumeOrderFIFO(Order& o, RecipeBook& recipes, Inventory& inv, int today)
    {
        DemandHashTable need;
        vector<OrderLine>& lines = o.getItems();
        vector<RecipeItem>& recs = recipes.getRecipes();

        for (int i = 0; i < (int)lines.size(); i++)
            for (int k = 0; k < (int)recs.size(); k++)
                if (recs[k].foodId == lines[i].foodId)
                    need.add(recs[k].ingredientId, recs[k].quantity * lines[i].quantity);

        vector<Ingredient>& ings = inv.getIngredients();
        // Step 1: Verify ALL ingredients have enough non-expired stock before deducting anything
        for (int i = 0; i < (int)ings.size(); i++)
        {
            int id = ings[i].getId();
            int req = need.get(id);
            if (req > 0 && inv.getUsableStock(id, today) < req)
            {
                cout << "Cannot confirm: not enough usable " << ings[i].getName() << ".\n";
                return false;
            }
        }

        // Step 2: Deduct each ingredient in FIFO order
        for (int i = 0; i < (int)ings.size(); i++)
        {
            int id = ings[i].getId();
            int req = need.get(id);
            if (req > 0) inv.useStockFIFO(id, req, today);
        }
        return true;
    }

public:
    OrderManager() { 
        nextId = 1001;
    }

    int placeOrder(vector<OrderLine>& items, int today)
    {
        Order o(nextId, today);
        for (int i = 0; i < (int)items.size(); i++)
            o.addFood(items[i].foodId, items[i].quantity);
        orders.push_back(o);
        queue.push(nextId);
        return nextId++;
    }

    int confirmNext(RecipeBook& recipes, Inventory& inv, int today)
    {
        while (!queue.empty())
        {
            int id = queue.pop();
            for (int i = 0; i < (int)orders.size(); i++)
            {
                if (orders[i].getId() == id && orders[i].getStatus() == "PENDING")
                {
                    if (!consumeOrderFIFO(orders[i], recipes, inv, today))
                    {
                        queue.push(id); // Keep in queue if stock is short
                        return -1;
                    }
                    orders[i].setStatus("CONFIRMED");
                    cout << "Order " << id << " confirmed and stock deducted (FIFO).\n";
                    return id;
                }
            }
        }
        cout << "No pending order.\n";
        return -1;
    }

    bool confirmOrder(int id, RecipeBook& recipes, Inventory& inv, int today)
    {
        for (int i = 0; i < (int)orders.size(); i++)
        {
            if (orders[i].getId() == id && orders[i].getStatus() == "PENDING")
            {
                if (!consumeOrderFIFO(orders[i], recipes, inv, today)) return false;
                orders[i].setStatus("CONFIRMED");
                cout << "Order confirmed and stock deducted (FIFO).\n";
                return true;
            }
        }
        cout << "Pending order not found.\n";
        return false;
    }

    bool cancelOrder(int id)
    {
        for (int i = 0; i < (int)orders.size(); i++)
        {
            if (orders[i].getId() == id && orders[i].getStatus() == "PENDING")
            {
                orders[i].setStatus("CANCELLED");
                cout << "Order cancelled.\n";
                return true;
            }
        }
        cout << "Only pending orders can be cancelled.\n";
        return false;
    }

    void showOrders()
    {
        cout << "\n----- ORDERS -----\n";
        for (int i = 0; i < (int)orders.size(); i++)
        {
            cout << "Order " << orders[i].getId() << " | " << orders[i].getStatus()<< " | " << orders[i].getDate() << "\n";
            vector<OrderLine>& lines = orders[i].getItems();
            for (int j = 0; j < (int)lines.size(); j++)
                cout << "    Food " << lines[j].foodId << " x " << lines[j].quantity << "\n";
        }
    }

    vector<Order>& getOrders() { return orders; }

    void setOrders(vector<Order>& list)
    {
        orders = list;
        queue.clear();
        nextId = 1001;
        for (int i = 0; i < (int)orders.size(); i++)
        {
            if (orders[i].getId() >= nextId) nextId = orders[i].getId() + 1;
            if (orders[i].getStatus() == "PENDING") queue.push(orders[i].getId());
        }
    }
};

#endif
