#ifndef RECIPE_H
#define RECIPE_H

#include "models.h"

struct RecipeItem
{
    int foodId, ingredientId, quantity;
    RecipeItem(int f = 0, int g = 0, int q = 0)
    {
        foodId = f;
        ingredientId = g;
        quantity = q;
    }
};

class DemandHashTable
{
private:
    int keys[101], values[101], used[101];

public:
    DemandHashTable()
    {
        for (int i = 0; i < 101; i++)
        {
            keys[i] = 0;
            values[i] = 0;
            used[i] = 0;
        }
    }

    void add(int key, int amount)
    {
        int pos = key % 101;
        while (used[pos] && keys[pos] != key)
            pos = (pos + 1) % 101;
        if (!used[pos])
        {
            keys[pos] = key;
            used[pos] = 1;
        }
        values[pos] += amount;
    }

    int get(int key)
    {
        int pos = key % 101, start = pos;
        while (used[pos])
        {
            if (keys[pos] == key) return values[pos];
            pos = (pos + 1) % 101;
            if (pos == start) break;
        }
        return 0;
    }

    void show()
    {
        cout << "\n----- INGREDIENT DEMAND -----\n";
        for (int i = 0; i < 101; i++)
            if (used[i]) cout << "Ingredient " << keys[i] << " : " << values[i] << "\n";
    }
};

class RecipeBook
{
private:
    vector<RecipeItem> recipes;

public:
    void addRecipe(int foodId, int ingredientId, int quantity)
    {
        recipes.push_back(RecipeItem(foodId, ingredientId, quantity));
    }

    void deleteRecipe(int foodId)
    {
        int i = 0;
        while (i < (int)recipes.size())
        {
            if (recipes[i].foodId == foodId) recipes.erase(recipes.begin() + i);
            else i++;
        }
        cout << "Recipe entries for food " << foodId << " deleted.\n";
    }

    vector<RecipeItem>& getRecipes() { return recipes; }

    void showRecipes()
    {
        cout << "\n----- RECIPES -----\n";
        for (int i = 0; i < (int)recipes.size(); i++)
            cout << "Food " << recipes[i].foodId << " needs " << recipes[i].quantity
                 << " of ingredient " << recipes[i].ingredientId << " per serving.\n";
    }
};

#endif
