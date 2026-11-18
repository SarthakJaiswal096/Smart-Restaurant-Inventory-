#ifndef MENU_H
#define MENU_H

#include "models.h"
//rem
class FoodMenu
{
private:
    vector<Food> foods;

public:
    

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
