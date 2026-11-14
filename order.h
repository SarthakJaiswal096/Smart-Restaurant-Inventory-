#ifndef ORDER_H
#define ORDER_H

#include "models.h"

class OrderCircularQueue
{
private:
    int data[200];
    int front, rear, count;

public:
    OrderCircularQueue()
    {
        front = 0;
        rear = 0;
        count = 0;
    }
    bool empty() { return count == 0; }
    int size() { return count; }
    void clear()
    {
        front = 0;
        rear = 0;
        count = 0;
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

public:
    OrderManager() { nextId = 1001; }

    int placeOrder(vector<OrderLine>& items, int today)
    {
        Order o(nextId, today);
        for (int i = 0; i < (int)items.size(); i++)
            o.addFood(items[i].foodId, items[i].quantity);
        orders.push_back(o);
        queue.push(nextId);
        return nextId++;
    }

    int confirmNext()
    {
        while (!queue.empty())
        {
            int id = queue.pop();
            for (int i = 0; i < (int)orders.size(); i++)
            {
                if (orders[i].getId() == id && orders[i].getStatus() == "PENDING")
                {
                    orders[i].setStatus("CONFIRMED");
                    cout << "Order " << id << " confirmed.\n";
                    return id;
                }
            }
        }
        cout << "No pending order.\n";
        return -1;
    }

    bool confirmOrder(int id)
    {
        for (int i = 0; i < (int)orders.size(); i++)
        {
            if (orders[i].getId() == id && orders[i].getStatus() == "PENDING")
            {
                orders[i].setStatus("CONFIRMED");
                cout << "Order confirmed.\n";
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
            cout << "Order " << orders[i].getId() << " | " << orders[i].getStatus()
                 << " | " << orders[i].getDate() << "\n";
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
