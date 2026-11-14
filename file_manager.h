#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <fstream>
#include <sstream>
#include "menu.h"
#include "order.h"
#include "recipe.h"
#include "inventory.h"

class FileManager
{
public:
    void save(FoodMenu& menu, OrderManager& orders, RecipeBook& recipes, Inventory& inventory)
    {
        ofstream foodFile("data/foods.txt");
        vector<Food>& foods = menu.getFoods();
        for (int i = 0; i < (int)foods.size(); i++)
            foodFile << foods[i].getId() << "|" << foods[i].getName() << "|" << foods[i].getPrice() << "\n";
        foodFile.close();

        ofstream recipeFile("data/recipes.txt");
        vector<RecipeItem>& recipeList = recipes.getRecipes();
        for (int i = 0; i < (int)recipeList.size(); i++)
            recipeFile << recipeList[i].foodId << "|" << recipeList[i].ingredientId << "|" << recipeList[i].quantity << "\n";
        recipeFile.close();

        ofstream ingredientFile("data/ingredients.txt");
        vector<Ingredient>& ingredients = inventory.getIngredients();
        for (int i = 0; i < (int)ingredients.size(); i++)
            ingredientFile << ingredients[i].getId() << "|" << ingredients[i].getName() << "|"
                           << ingredients[i].getUnit() << "|" << ingredients[i].getMinimumStock() << "|"
                           << ingredients[i].getCostPerUnit() << "\n";
        ingredientFile.close();

        ofstream batchFile("data/batches.txt");
        vector<Batch>& batches = inventory.getBatches();
        for (int i = 0; i < (int)batches.size(); i++)
            batchFile << batches[i].id << "|" << batches[i].ingredientId << "|"
                      << batches[i].quantity << "|" << batches[i].expiry << "\n";
        batchFile.close();

        ofstream orderFile("data/orders.txt");
        vector<Order>& allOrders = orders.getOrders();
        for (int i = 0; i < (int)allOrders.size(); i++)
        {
            orderFile << allOrders[i].getId() << "|" << allOrders[i].getStatus() << "|" << allOrders[i].getDate() << "|";
            vector<OrderLine>& lines = allOrders[i].getItems();
            orderFile << lines.size();
            for (int j = 0; j < (int)lines.size(); j++)
                orderFile << "|" << lines[j].foodId << "|" << lines[j].quantity;
            orderFile << "\n";
        }
        orderFile.close();

        ofstream transactionFile("data/transactions.txt");
        vector<StockTransaction>& log = inventory.getTransactions();
        for (int i = 0; i < (int)log.size(); i++)
            transactionFile << log[i].type << "|" << log[i].ingredientId << "|" << log[i].quantity << "|"
                            << log[i].date << "|" << log[i].note << "\n";
        transactionFile.close();
        cout << "All data saved in the data folder.\n";
    }

    void load(FoodMenu& menu, OrderManager& orders, RecipeBook& recipes, Inventory& inventory)
    {
        menu.getFoods().clear();
        recipes.getRecipes().clear();
        inventory.getIngredients().clear();
        inventory.getBatches().clear();
        inventory.getTransactions().clear();
        vector<Order> noOrders;
        orders.setOrders(noOrders);
        inventory.rebuildUndo();

        string line;
        ifstream foodFile("data/foods.txt");
        while (getline(foodFile, line))
        {
            stringstream s(line);
            string id, name, price;
            getline(s, id, '|'); getline(s, name, '|'); getline(s, price, '|');
            if (id != "") menu.addFood(stoi(id), name, stoi(price));
        }
        foodFile.close();

        ifstream ingredientFile("data/ingredients.txt");
        while (getline(ingredientFile, line))
        {
            stringstream s(line);
            string id, name, unit, minimum, cost;
            getline(s, id, '|'); getline(s, name, '|'); getline(s, unit, '|');
            getline(s, minimum, '|'); getline(s, cost, '|');
            if (id != "") inventory.addIngredient(stoi(id), name, unit, stoi(minimum), stoi(cost));
        }
        ingredientFile.close();

        ifstream batchFile("data/batches.txt");
        while (getline(batchFile, line))
        {
            stringstream s(line);
            string id, g, q, e;
            getline(s, id, '|'); getline(s, g, '|'); getline(s, q, '|'); getline(s, e, '|');
            if (id != "") inventory.getBatches().push_back(Batch(stoi(id), stoi(g), stoi(q), stoi(e)));
        }
        batchFile.close();

        ifstream recipeFile("data/recipes.txt");
        while (getline(recipeFile, line))
        {
            stringstream s(line);
            string f, g, q;
            getline(s, f, '|'); getline(s, g, '|'); getline(s, q, '|');
            if (f != "") recipes.addRecipe(stoi(f), stoi(g), stoi(q));
        }
        recipeFile.close();

        vector<Order> loadedOrders;
        ifstream orderFile("data/orders.txt");
        while (getline(orderFile, line))
        {
            stringstream s(line);
            string id, status, date, count;
            getline(s, id, '|'); getline(s, status, '|'); getline(s, date, '|'); getline(s, count, '|');
            if (id == "") continue;
            Order order(stoi(id), stoi(date));
            order.setStatus(status);
            int number = stoi(count);
            for (int i = 0; i < number; i++)
            {
                string food, qty;
                getline(s, food, '|'); getline(s, qty, '|');
                order.addFood(stoi(food), stoi(qty));
            }
            loadedOrders.push_back(order);
        }
        orderFile.close();
        orders.setOrders(loadedOrders);

        vector<StockTransaction> loadedLog;
        ifstream transactionFile("data/transactions.txt");
        while (getline(transactionFile, line))
        {
            stringstream s(line);
            string type, id, qty, date, note;
            getline(s, type, '|'); getline(s, id, '|'); getline(s, qty, '|');
            getline(s, date, '|'); getline(s, note, '|');
            if (type != "") loadedLog.push_back(StockTransaction(type, stoi(id), stoi(qty), stoi(date), note));
        }
        transactionFile.close();
        inventory.setTransactions(loadedLog);
        cout << "Saved data loaded.\n";
    }
};

#endif
