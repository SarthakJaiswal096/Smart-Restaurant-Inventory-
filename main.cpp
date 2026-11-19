#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include "models.h"
#include "menu.h"
#include "order.h"
#include "recipe.h"
#include "inventory.h"
#include "report.h"
#include "file_manager.h"
#include "login.h"

using namespace std;

FoodMenu menu;
OrderManager orderManager;
RecipeBook recipeBook;
Inventory inventory;
FileManager files;

bool inputBad()
{
    if (cin.fail())
    {
        if (cin.eof()) return true;
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid input. Please enter the values asked for.\n";
        return true;
    }
    return false;
}

void placeOrder(int today)
{
    vector<OrderLine> items;
    int foodId = 0, quantity = 0;

    menu.showMenu();
    cout << "Enter food ID (0 to finish): ";
    cin >> foodId;

    while (!inputBad() && foodId != 0)
    {
        if (menu.getFood(foodId) == NULL)
            cout << "Food not found.\n";
        else
        {
            cout << "Quantity: ";
            cin >> quantity;
            if (inputBad()) break;
            if (quantity > 0)
                items.push_back(OrderLine(foodId, quantity));
        }
        cout << "Next food ID (0 to finish): ";
        cin >> foodId;
    }

    if (!items.empty())
        cout << "Order created. ID: " << orderManager.placeOrder(items, today) << "\n";
    else
        cout << "No food added.\n";
}

void calculateDemand()
{
    DemandHashTable demand;
    vector<Order>& orders = orderManager.getOrders();
    vector<RecipeItem>& recipes = recipeBook.getRecipes();

    for (int i = 0; i < (int)orders.size(); i++)
        if (orders[i].getStatus() == "CONFIRMED")
        {
            vector<OrderLine>& lines = orders[i].getItems();
            for (int j = 0; j < (int)lines.size(); j++)
                for (int k = 0; k < (int)recipes.size(); k++)
                    if (recipes[k].foodId == lines[j].foodId)
                        demand.add(recipes[k].ingredientId, recipes[k].quantity * lines[j].quantity);
        }

    demand.show();
    cout << "Demand is only calculated. Stock is not reduced here.\n";
}

void averageDailyDemand()
{
    vector<Order>& orders = orderManager.getOrders();
    vector<RecipeItem>& recipes = recipeBook.getRecipes();
    int firstDate = 99999999, lastDate = 0;

    for (int i = 0; i < (int)orders.size(); i++)
        if (orders[i].getStatus() == "CONFIRMED")
        {
            if (orders[i].getDate() < firstDate) firstDate = orders[i].getDate();
            if (orders[i].getDate() > lastDate) lastDate = orders[i].getDate();
        }

    if (lastDate == 0)
    {
        cout << "No confirmed orders for forecast.\n";
        return;
    }

    int days = inventoryDateNumber(lastDate) - inventoryDateNumber(firstDate) + 1;
    if (days < 1) days = 1;

    DemandHashTable total;
    for (int i = 0; i < (int)orders.size(); i++)
        if (orders[i].getStatus() == "CONFIRMED")
        {
            vector<OrderLine>& lines = orders[i].getItems();
            for (int j = 0; j < (int)lines.size(); j++)
                for (int k = 0; k < (int)recipes.size(); k++)
                    if (recipes[k].foodId == lines[j].foodId)
                        total.add(recipes[k].ingredientId, recipes[k].quantity * lines[j].quantity);
        }

    cout << "Average daily demand over " << days << " day(s):\n";
    vector<Ingredient>& ingredients = inventory.getIngredients();
    for (int i = 0; i < (int)ingredients.size(); i++)
        cout << ingredients[i].getName() << " : " << total.get(ingredients[i].getId()) / days
             << " " << ingredients[i].getUnit() << " per day\n";
}

void showReports(int today)
{
    int choice = -1;
    do
    {
        cout << "\n----- REPORTS -----\n";
        cout << "1. Sales per day\n2. Sales per food\n3. Stock value\n";
        cout << "4. Expiry report\n5. Low stock report\n6. Wastage report\n0. Back\nChoice: ";
        cin >> choice;
        if (cin.eof()) return;
        if (inputBad()) { choice = -1; continue; }

        if (choice == 1)
        {
            int date = 0;
            cout << "Enter date YYYYMMDD: ";
            cin >> date;
            if (inputBad()) continue;
            SalesReport report(&menu, &orderManager);
            report.perDay(date);
        }
        else if (choice == 2)
        {
            SalesReport report(&menu, &orderManager);
            report.perFood();
        }
        else if (choice == 3) { StockReport r(&inventory, today); Report *p = &r; p->show(); }
        else if (choice == 4) { ExpiryReport r(&inventory, today); Report *p = &r; p->show(); }
        else if (choice == 5) { LowStockReport r(&inventory, today); Report *p = &r; p->show(); }
        else if (choice == 6) { WastageReport r(&inventory); Report *p = &r; p->show(); }
    } while (choice != 0);
}

int getTodayDate()
{
    time_t now = time(0);
    tm *local = localtime(&now);
    return (local->tm_year + 1900) * 10000 + (local->tm_mon + 1) * 100 + local->tm_mday;
}

int main()
{
    LoginManager loginManager;
    string username, password, role;

    cout << "====================================\n";
    cout << " SMART RESTAURANT INVENTORY SYSTEM\n";
    cout << "====================================\n";
    cout << "Username: "; cin >> username;
    cout << "Password: "; cin >> password;

    role = loginManager.login(username, password);
    if (role == "")
    {
        cout << "Wrong username or password.\n";
        return 0;
    }

    cout << "Welcome. Login role: " << role << "\n";
    files.load(menu, orderManager, recipeBook, inventory);
    int today = getTodayDate();
    int choice = -1;

    do
    {
        cout << "\n========== MAIN MENU ==========\n";
        cout << "1. Show food menu          13. Average daily demand\n";
        cout << "2. Add food                14. Show stock\n";
        cout << "3. Update food             15. Add ingredient\n";
        cout << "4. Place order             16. Receive stock batch\n";
        cout << "5. Show orders             17. Show batches\n";
        cout << "6. Confirm next order      18. Use stock (FEFO)\n";
        cout << "7. Confirm chosen order    19. Record wastage\n";
        cout << "8. Cancel order            20. Write off expired stock\n";
        cout << "9. Show recipes            21. Undo last stock receipt\n";
        cout << "10. Add recipe ingredient  22. Reports\n";
        cout << "11. Delete complete recipe 23. Save all data\n";
        cout << "12. Calculate demand       24. Load all data\n";
        cout << "0. Save and exit\nChoice: ";
        cin >> choice;
        if (cin.eof()) break;                       // input ended: save and exit
        if (inputBad()) { 
            choice = -1; continue; 
        }  // letters instead of a number

        if (choice == 1) menu.showMenu();
        else if (choice == 2 && role != "STOCK_FILLER")
        {
            int id = 0, p = 0; string name;
            cout << "Food ID, name, price: "; 
            cin >> id >> name >> p;
            if (inputBad()) continue;
            menu.addFood(id, name, p);
        }
        else if (choice == 3 && role == "ADMIN")
        {
            int id = 0, p = 0; string name;
            cout << "Food ID, new name, new price: "; 
            cin >> id >> name >> p;
            if (inputBad()) 
            continue;
            menu.updateFood(id, name, p);
        }
        else if (choice == 4 && role != "STOCK_FILLER") 
            placeOrder(today);
        else if (choice == 5 && role != "STOCK_FILLER") 
            orderManager.showOrders();
        else if (choice == 6 && role != "STOCK_FILLER") 
            orderManager.confirmNext(recipeBook, inventory, today);
        else if (choice == 7 && role != "STOCK_FILLER")
        {
            int id = 0; cout << "Order ID: "; cin >> id;
            if (inputBad()) continue;
            orderManager.confirmOrder(id, recipeBook, inventory, today);
        }
        else if (choice == 8 && role != "STOCK_FILLER")
        {
            int id = 0;
            cout << "Order ID: "; 
            cin >> id;
            if (inputBad()) continue;
            orderManager.cancelOrder(id);
        }
        else if (choice == 9 && role != "STOCK_FILLER") recipeBook.showRecipes();
        else if (choice == 10 && role != "STOCK_FILLER")
        {
            int f = 0, g = 0, q = 0; cout << "Food ID, ingredient ID, qty per serving: "; cin >> f >> g >> q;
            if (inputBad()) continue;
            if (menu.getFood(f) == NULL) 
                cout << "Food not found.\n";
            else if (inventory.findIngredient(g) == NULL) 
                cout << "Ingredient not found.\n";
            else if (q <= 0) cout << "Quantity must be positive.\n";
            else { 
                recipeBook.addRecipe(f, g, q); 
                cout << "Recipe ingredient added.\n"; 
            }
        }
        else if (choice == 11 && role == "ADMIN")
        {
            int f = 0; cout << "Food ID: "; 
            cin >> f;
            if (inputBad()) continue;
            recipeBook.deleteRecipe(f);
        }
        else if (choice == 12 && role != "STOCK_FILLER") calculateDemand();
        else if (choice == 13 && role != "STOCK_FILLER") averageDailyDemand();
        else if (choice == 14) inventory.showStock(today);
        else if (choice == 15 && role != "STAFF")
        {
            int id = 0, m = 0, c = 0; string n, u;
            cout << "ID name unit min-stock cost-per-unit: "; cin >> id >> n >> u >> m >> c;
            if (inputBad()) continue;
            inventory.addIngredient(id, n, u, m, c);
        }
        else if (choice == 16 && role != "STAFF")
        {
            int b = 0, g = 0, q = 0, e = 0;
            cout << "Batch ID, ingredient ID, qty, expiry(YYYYMMDD): "; cin >> b >> g >> q >> e;
            if (inputBad()) continue;
            inventory.addStock(b, g, q, e, today);
        }
        else if (choice == 17) inventory.showBatches(today);
        else if (choice == 18 && role != "STAFF")
        {
            int id = 0, q = 0; cout << "Ingredient ID and quantity: "; cin >> id >> q;
            if (inputBad()) continue;
            inventory.useStock(id, q, today);
        }
        else if (choice == 19 && role != "STAFF")
        {
            int id = 0, q = 0; string note;
            cout << "Ingredient ID, qty, reason(one_word): "; cin >> id >> q >> note;
            if (inputBad()) continue;
            inventory.recordWastage(id, q, today, note);
        }
        else if (choice == 20 && role != "STAFF") inventory.writeOffExpired(today);
        else if (choice == 21 && role == "ADMIN") inventory.undoLastStock(today);
        else if (choice == 22) showReports(today);
        else if (choice == 23 && role == "ADMIN") files.save(menu, orderManager, recipeBook, inventory);
        else if (choice == 24 && role == "ADMIN") files.load(menu, orderManager, recipeBook, inventory);
        else if (choice != 0) cout << "Invalid choice or permission denied.\n";
    } while (choice != 0);

    files.save(menu, orderManager, recipeBook, inventory);
    cout << "Data saved. Goodbye.\n";
    return 0;
}

