#ifndef MENU_H
#define MENU_H

#include "models.h"

class FoodMenu
{
private:
    vector<Food> foods;

public:
    void addFood(int id, string name, int price)
    {
        if (searchFood(id) != -1)
        {
            cout << "Food ID already exists.\n";
            return;
        }
        foods.push_back(Food(id, name, price));
        for (int i = (int)foods.size() - 1; i > 0; i--)
        {
            if (foods[i].getId() < foods[i - 1].getId())
            {
                Food temp = foods[i];
                foods[i] = foods[i - 1];
                foods[i - 1] = temp;
            }
            else break;
        }
    }

    int searchFood(int id)
    {
        int low = 0, high = (int)foods.size() - 1;
        while (low <= high)
        {
            int mid = (low + high) / 2;
            if (foods[mid].getId() == id) return mid;
            if (foods[mid].getId() < id) low = mid + 1;
            else high = mid - 1;
        }
        return -1;
    }

    Food* getFood(int id)
    {
        int pos = searchFood(id);
        return (pos == -1) ? NULL : &foods[pos];
    }

    void updateFood(int id, string name, int price)
    {
        Food *food = getFood(id);
        if (food == NULL)
        {
            cout << "Food not found.\n";
            return;
        }
        food->setName(name);
        food->setPrice(price);
        cout << "Food updated.\n";
    }

    void showMenu()
    {
        cout << "\n----- FOOD MENU -----\n";
        for (int i = 0; i < (int)foods.size(); i++)
        {
            foods[i].show();
            cout << "\n";
        }
    }

    vector<Food>& getFoods() { return foods; }
};

#endif
