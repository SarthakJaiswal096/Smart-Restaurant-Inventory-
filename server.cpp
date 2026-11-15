#include <iostream>
#include <ctime>
#include "httplib.h"
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
LoginManager loginManager;

int getToday()
{
    time_t now = time(0);
    tm *t = localtime(&now);
    return (t->tm_year + 1900) * 10000 + (t->tm_mon + 1) * 100 + t->tm_mday;
}

int getParam(const httplib::Request& req, string key, int def = 0)
{
    return req.has_param(key) ? stoi(req.get_param_value(key)) : def;
}

int main()
{
    int today = getToday();
    files.load(menu, orderManager, recipeBook, inventory);

    httplib::Server svr;
    svr.set_mount_point("/", "./website");

    svr.Post("/api/login", [](const httplib::Request& req, httplib::Response& res)
    {
        string u = req.has_param("user") ? req.get_param_value("user") : "";
        string p = req.has_param("pass") ? req.get_param_value("pass") : "";
        string role = loginManager.login(u, p);
        if (role == "")
            res.set_content("{\"ok\":false,\"error\":\"Invalid username or password\"}", "application/json");
        else
            res.set_content("{\"ok\":true,\"user\":\"" + u + "\",\"role\":\"" + role + "\"}", "application/json");
    });

    svr.Get("/api/state", [&](const httplib::Request&, httplib::Response& res)
    {
        vector<Food>& foods = menu.getFoods();
        vector<Order>& orders = orderManager.getOrders();
        vector<RecipeItem>& recs = recipeBook.getRecipes();
        vector<Ingredient>& ings = inventory.getIngredients();
        vector<Batch>& batches = inventory.getBatches();
        vector<StockTransaction>& txns = inventory.getTransactions();

        string j = "{\"ok\":true,\"today\":" + to_string(today) + ",\"foods\":[";
        for (int i = 0; i < (int)foods.size(); i++)
        {
            if (i) j += ",";
            j += "{\"id\":" + to_string(foods[i].getId()) + ",\"name\":\"" + foods[i].getName()
               + "\",\"price\":" + to_string(foods[i].getPrice()) + "}";
        }

        j += "],\"orders\":[";
        for (int i = 0; i < (int)orders.size(); i++)
        {
            if (i) j += ",";
            vector<OrderLine>& lines = orders[i].getItems();
            int total = 0;
            string summary = "";
            for (int k = 0; k < (int)lines.size(); k++)
            {
                Food *f = menu.getFood(lines[k].foodId);
                if (f != NULL)
                {
                    total += f->getPrice() * lines[k].quantity;
                    if (k) summary += ", ";
                    summary += f->getName() + " x" + to_string(lines[k].quantity);
                }
            }
            j += "{\"id\":" + to_string(orders[i].getId()) + ",\"status\":\"" + orders[i].getStatus()
               + "\",\"date\":" + to_string(orders[i].getDate()) + ",\"items\":\"" + summary
               + "\",\"total\":" + to_string(total) + "}";
        }

        j += "],\"recipes\":[";
        for (int i = 0; i < (int)recs.size(); i++)
        {
            if (i) j += ",";
            j += "{\"foodId\":" + to_string(recs[i].foodId) + ",\"ingId\":" + to_string(recs[i].ingredientId)
               + ",\"qty\":" + to_string(recs[i].quantity) + "}";
        }

        j += "],\"ingredients\":[";
        for (int i = 0; i < (int)ings.size(); i++)
        {
            if (i) j += ",";
            int id = ings[i].getId();
            int usable = inventory.getUsableStock(id, today);
            j += "{\"id\":" + to_string(id) + ",\"name\":\"" + ings[i].getName()
               + "\",\"unit\":\"" + ings[i].getUnit() + "\",\"usable\":" + to_string(usable)
               + ",\"min\":" + to_string(ings[i].getMinimumStock())
               + ",\"cost\":" + to_string(ings[i].getCostPerUnit()) + "}";
        }

        j += "],\"batches\":[";
        for (int i = 0; i < (int)batches.size(); i++)
        {
            if (i) j += ",";
            Ingredient *ing = inventory.findIngredient(batches[i].ingredientId);
            string iname = ing ? ing->getName() : ("Ing#" + to_string(batches[i].ingredientId));
            j += "{\"id\":" + to_string(batches[i].id) + ",\"ingId\":" + to_string(batches[i].ingredientId)
               + ",\"ingName\":\"" + iname + "\",\"qty\":" + to_string(batches[i].quantity)
               + ",\"expiry\":" + to_string(batches[i].expiry) + "}";
        }

        j += "],\"transactions\":[";
        for (int i = 0; i < (int)txns.size(); i++)
        {
            if (i) j += ",";
            j += "{\"type\":\"" + txns[i].type + "\",\"ingId\":" + to_string(txns[i].ingredientId)
               + ",\"qty\":" + to_string(txns[i].quantity) + ",\"date\":" + to_string(txns[i].date)
               + ",\"note\":\"" + txns[i].note + "\"}";
        }

        DemandHashTable demand;
        for (int i = 0; i < (int)orders.size(); i++)
            if (orders[i].getStatus() == "CONFIRMED")
            {
                vector<OrderLine>& lines = orders[i].getItems();
                for (int k = 0; k < (int)lines.size(); k++)
                    for (int r = 0; r < (int)recs.size(); r++)
                        if (recs[r].foodId == lines[k].foodId)
                            demand.add(recs[r].ingredientId, recs[r].quantity * lines[k].quantity);
            }

        j += "],\"demand\":[";
        for (int i = 0; i < (int)ings.size(); i++)
        {
            if (i) j += ",";
            int id = ings[i].getId();
            j += "{\"ingId\":" + to_string(id) + ",\"name\":\"" + ings[i].getName()
               + "\",\"unit\":\"" + ings[i].getUnit() + "\",\"needed\":" + to_string(demand.get(id))
               + ",\"usable\":" + to_string(inventory.getUsableStock(id, today)) + "}";
        }
        j += "]}";
        res.set_content(j, "application/json");
    });

    svr.Post("/api/food/add", [](const httplib::Request& req, httplib::Response& res)
    {
        int id = getParam(req, "id"), price = getParam(req, "price");
        string name = req.has_param("name") ? req.get_param_value("name") : "";
        menu.addFood(id, name, price);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content("{\"ok\":true}", "application/json");
    });

    svr.Post("/api/order/place", [&](const httplib::Request& req, httplib::Response& res)
    {
        int fid = getParam(req, "foodId"), qty = getParam(req, "qty", 1);
        vector<OrderLine> lines = { OrderLine(fid, qty) };
        int oid = orderManager.placeOrder(lines, today);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content("{\"ok\":true,\"id\":" + to_string(oid) + "}", "application/json");
    });

    svr.Post("/api/order/confirmNext", [&](const httplib::Request&, httplib::Response& res)
    {
        int id = orderManager.confirmNext(recipeBook, inventory, today);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content(id == -1 ? "{\"ok\":false,\"error\":\"No pending order or insufficient stock\"}" : "{\"ok\":true}", "application/json");
    });

    svr.Post("/api/order/action", [&](const httplib::Request& req, httplib::Response& res)
    {
        int id = getParam(req, "id");
        string act = req.has_param("act") ? req.get_param_value("act") : "";
        bool ok = (act == "confirm") ? orderManager.confirmOrder(id, recipeBook, inventory, today) : orderManager.cancelOrder(id);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content(ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"Action failed or insufficient stock\"}", "application/json");
    });
    
    svr.Post("/api/recipe/add", [](const httplib::Request& req, httplib::Response& res)
    {
        recipeBook.addRecipe(getParam(req, "foodId"), getParam(req, "ingId"), getParam(req, "qty"));
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content("{\"ok\":true}", "application/json");
    });

    svr.Post("/api/ingredient/add", [](const httplib::Request& req, httplib::Response& res)
    {
        string name = req.has_param("name") ? req.get_param_value("name") : "";
        string unit = req.has_param("unit") ? req.get_param_value("unit") : "g";
        inventory.addIngredient(getParam(req, "id"), name, unit, getParam(req, "min"), getParam(req, "cost"));
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content("{\"ok\":true}", "application/json");
    });

    svr.Post("/api/stock/receive", [&](const httplib::Request& req, httplib::Response& res)
    {
        bool ok = inventory.addStock(getParam(req, "batchId"), getParam(req, "ingId"),
                                     getParam(req, "qty"), getParam(req, "expiry"), today);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content(ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"Check inputs\"}", "application/json");
    });

    svr.Post("/api/stock/use", [&](const httplib::Request& req, httplib::Response& res)
    {
        bool ok = inventory.useStock(getParam(req, "ingId"), getParam(req, "qty"), today);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content(ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"Not enough usable stock\"}", "application/json");
    });

    svr.Post("/api/stock/waste", [&](const httplib::Request& req, httplib::Response& res)
    {
        string note = req.has_param("note") ? req.get_param_value("note") : "Spoiled";
        bool ok = inventory.recordWastage(getParam(req, "ingId"), getParam(req, "qty"), today, note);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content(ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"Not enough stock\"}", "application/json");
    });

    svr.Post("/api/stock/expire", [&](const httplib::Request&, httplib::Response& res)
    {
        inventory.writeOffExpired(today);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content("{\"ok\":true}", "application/json");
    });

    svr.Post("/api/stock/undo", [&](const httplib::Request&, httplib::Response& res)
    {
        bool ok = inventory.undoLastStock(today);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content(ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"Cannot undo\"}", "application/json");
    });

    cout << "Server running at http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}
