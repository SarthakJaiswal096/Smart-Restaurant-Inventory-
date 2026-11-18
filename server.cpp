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

int today = 0;

int getToday()
{
    time_t now = time(0);
    tm *t = localtime(&now);
    return (t->tm_year + 1900) * 10000 + (t->tm_mon + 1) * 100 + t->tm_mday;
}

// Reads a whole number from the request. A missing key gives the default,
// a non-numeric value is reported to the browser as a clear error.
int getParam(const httplib::Request& req, string key, int def = 0)
{
    if (!req.has_param(key)) return def;
    try
    {
        size_t used = 0;
        int value = stoi(req.get_param_value(key), &used);
        if (used != req.get_param_value(key).size()) throw invalid_argument("x");
        return value;
    }
    catch (const exception&)
    {
        throw runtime_error("'" + key + "' must be a whole number");
    }
}

// Reads text from the request. The data files use '|' as separator and the JSON
// replies use quotes, so those characters (and control characters) are removed.
string getText(const httplib::Request& req, string key, string def = "")
{
    if (!req.has_param(key)) return def;
    string in = req.get_param_value(key), out;
    for (size_t i = 0; i < in.size(); i++)
    {
        unsigned char c = (unsigned char)in[i];
        if (c == '|' || c == '"' || c == '\\' || c == '<' || c == '>' || c < 32) continue;
        out += (char)c;
    }
    size_t a = out.find_first_not_of(' ');
    size_t b = out.find_last_not_of(' ');
    out = (a == string::npos) ? "" : out.substr(a, b - a + 1);
    for (size_t i = 0; i < out.size(); i++)
        if (out[i] == ' ') out[i] = '_'; // names are one word in the data files
    return out.empty() ? def : out;
}

// Makes any text safe to place inside a JSON string.
string jsonEsc(const string& in)
{
    string out;
    for (size_t i = 0; i < in.size(); i++)
    {
        unsigned char c = (unsigned char)in[i];
        if (c == '"' || c == '\\') { out += '\\'; out += (char)c; }
        else if (c < 32) out += ' ';
        else out += (char)c;
    }
    return out;
}

string errorJson(string message)
{
    return "{\"ok\":false,\"error\":\"" + message + "\"}";
}

int main()
{
    today = getToday();
    files.load(menu, orderManager, recipeBook, inventory);

    httplib::Server svr;
    ifstream checkRootWeb("./website/index.html");
    if (checkRootWeb.good()) {
        svr.set_mount_point("/", "./website");
    } else {
        svr.set_mount_point("/", ".");
    }

    svr.Post("/api/data/save", [&](const httplib::Request&, httplib::Response& res)
    {
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content("{\"ok\":true,\"message\":\"Data saved to data/*.txt files\"}", "application/json");
    });

    svr.Post("/api/login", [](const httplib::Request& req, httplib::Response& res)
    {
        string u = req.has_param("user") ? req.get_param_value("user") : "";
        string p = req.has_param("pass") ? req.get_param_value("pass") : "";
        string role = loginManager.login(u, p);
        if (role == "")
            res.set_content("{\"ok\":false,\"error\":\"Invalid username or password\"}", "application/json");
        else
            res.set_content("{\"ok\":true,\"user\":\"" + getText(req, "user") + "\",\"role\":\"" + role + "\"}", "application/json");
    });

    svr.Get("/api/state", [&](const httplib::Request&, httplib::Response& res)
    {
        today = getToday();
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
            j += "{\"id\":" + to_string(foods[i].getId()) + ",\"name\":\"" + jsonEsc(foods[i].getName())
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
                    summary += jsonEsc(f->getName()) + " x" + to_string(lines[k].quantity);
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
            j += "{\"id\":" + to_string(id) + ",\"name\":\"" + jsonEsc(ings[i].getName())
               + "\",\"unit\":\"" + jsonEsc(ings[i].getUnit()) + "\",\"usable\":" + to_string(usable)
               + ",\"min\":" + to_string(ings[i].getMinimumStock())
               + ",\"cost\":" + to_string(ings[i].getCostPerUnit()) + "}";
        }

        j += "],\"batches\":[";
        for (int i = 0; i < (int)batches.size(); i++)
        {
            if (i) j += ",";
            Ingredient *ing = inventory.findIngredient(batches[i].ingredientId);
            string iname = ing ? jsonEsc(ing->getName()) : ("Ing#" + to_string(batches[i].ingredientId));
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
               + ",\"note\":\"" + jsonEsc(txns[i].note) + "\"}";
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
            j += "{\"ingId\":" + to_string(id) + ",\"name\":\"" + jsonEsc(ings[i].getName())
               + "\",\"unit\":\"" + jsonEsc(ings[i].getUnit()) + "\",\"needed\":" + to_string(demand.get(id))
               + ",\"usable\":" + to_string(inventory.getUsableStock(id, today)) + "}";
        }
        j += "],\"sales\":[";
        vector<FoodSale> sales;
        for (int f = 0; f < (int)foods.size(); f++)
        {
            FoodSale row = {foods[f].getName(), 0, 0};
            for (int i = 0; i < (int)orders.size(); i++)
                if (orders[i].getStatus() == "CONFIRMED")
                {
                    vector<OrderLine>& lines = orders[i].getItems();
                    for (int k = 0; k < (int)lines.size(); k++)
                        if (lines[k].foodId == foods[f].getId())
                        {
                            row.plates += lines[k].quantity;
                            row.amount += lines[k].quantity * foods[f].getPrice();
                        }
                }
            sales.push_back(row);
        }
        if (!sales.empty()) mergeSortSales(sales, 0, (int)sales.size() - 1);
        for (int i = 0; i < (int)sales.size(); i++)
        {
            if (i) j += ",";
            j += "{\"name\":\"" + jsonEsc(sales[i].name) + "\",\"plates\":" + to_string(sales[i].plates)
               + ",\"amount\":" + to_string(sales[i].amount) + "}";
        }
        j += "]}";
        res.set_content(j, "application/json");
    });

    svr.Post("/api/food/add", [](const httplib::Request& req, httplib::Response& res)
    {
        today = getToday();
        int id = getParam(req, "id"), price = getParam(req, "price");
        string name = getText(req, "name");
        if (id <= 0 || name == "" || price < 0)
            res.set_content(errorJson("Enter a positive ID, a name and a price"), "application/json");
        else if (menu.getFood(id) != NULL)
            res.set_content(errorJson("Food ID already exists"), "application/json");
        else
        {
            menu.addFood(id, name, price);
            files.save(menu, orderManager, recipeBook, inventory);
            res.set_content("{\"ok\":true}", "application/json");
        }
    });

    svr.Post("/api/food/update", [](const httplib::Request& req, httplib::Response& res)
    {
        today = getToday();
        int id = getParam(req, "id"), price = getParam(req, "price");
        string name = getText(req, "name");
        if (id <= 0 || name == "" || price < 0)
            res.set_content(errorJson("Enter valid Food ID, name and price"), "application/json");
        else if (menu.getFood(id) == NULL)
            res.set_content(errorJson("Food ID not found"), "application/json");
        else
        {
            menu.updateFood(id, name, price);
            files.save(menu, orderManager, recipeBook, inventory);
            res.set_content("{\"ok\":true}", "application/json");
        }
    });

    svr.Post("/api/order/place", [&](const httplib::Request& req, httplib::Response& res)
    {
        today = getToday();
        vector<OrderLine> lines;

        if (req.has_param("items"))
        {
            string itemsParam = req.get_param_value("items");
            stringstream ss(itemsParam);
            string pairStr;
            while (getline(ss, pairStr, ','))
            {
                stringstream pss(pairStr);
                string fidStr, qtyStr;
                if (getline(pss, fidStr, ':') && getline(pss, qtyStr, ':'))
                {
                    try
                    {
                        int fid = stoi(fidStr), qty = stoi(qtyStr);
                        if (menu.getFood(fid) != NULL && qty > 0)
                            lines.push_back(OrderLine(fid, qty));
                    }
                    catch (...) {}
                }
            }
        }
        else
        {
            int fid = getParam(req, "foodId"), qty = getParam(req, "qty", 1);
            if (menu.getFood(fid) != NULL && qty > 0)
                lines.push_back(OrderLine(fid, qty));
        }

        if (lines.empty())
        {
            res.set_content(errorJson("Food not found or quantity is not positive"), "application/json");
            return;
        }

        int oid = orderManager.placeOrder(lines, today);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content("{\"ok\":true,\"id\":" + to_string(oid) + "}", "application/json");
    });

    svr.Post("/api/order/confirmNext", [&](const httplib::Request&, httplib::Response& res)
    {
        today = getToday();
        string errMsg;
        int id = orderManager.confirmNext(recipeBook, inventory, today, errMsg);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content(id == -1 ? errorJson(errMsg.empty() ? "No pending order or insufficient stock" : errMsg) : "{\"ok\":true,\"id\":" + to_string(id) + "}", "application/json");
    });

    svr.Post("/api/order/action", [&](const httplib::Request& req, httplib::Response& res)
    {
        today = getToday();
        int id = getParam(req, "id");
        string act = req.has_param("act") ? req.get_param_value("act") : "";
        if (act != "confirm" && act != "cancel")
        {
            res.set_content(errorJson("act must be confirm or cancel"), "application/json");
            return;
        }
        string errMsg;
        bool ok = false;
        if (act == "confirm") {
            ok = orderManager.confirmOrder(id, recipeBook, inventory, today, errMsg);
        } else {
            ok = orderManager.cancelOrder(id);
            if (!ok) errMsg = "Only pending orders can be cancelled";
        }
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content(ok ? "{\"ok\":true}" : errorJson(errMsg.empty() ? "Action failed or insufficient stock" : errMsg), "application/json");
    });
    
    svr.Post("/api/recipe/add", [](const httplib::Request& req, httplib::Response& res)
    {
        today = getToday();
        int fid = getParam(req, "foodId"), gid = getParam(req, "ingId"), qty = getParam(req, "qty");
        if (menu.getFood(fid) == NULL)
            res.set_content(errorJson("Food ID not found"), "application/json");
        else if (inventory.findIngredient(gid) == NULL)
            res.set_content(errorJson("Ingredient ID not found"), "application/json");
        else if (qty <= 0)
            res.set_content(errorJson("Quantity must be positive"), "application/json");
        else
        {
            recipeBook.addRecipe(fid, gid, qty);
            files.save(menu, orderManager, recipeBook, inventory);
            res.set_content("{\"ok\":true}", "application/json");
        }
    });

    svr.Post("/api/recipe/delete", [](const httplib::Request& req, httplib::Response& res)
    {
        today = getToday();
        int fid = getParam(req, "foodId");
        if (fid <= 0)
            res.set_content(errorJson("Invalid Food ID"), "application/json");
        else
        {
            recipeBook.deleteRecipe(fid);
            files.save(menu, orderManager, recipeBook, inventory);
            res.set_content("{\"ok\":true}", "application/json");
        }
    });

    svr.Post("/api/data/load", [&](const httplib::Request&, httplib::Response& res)
    {
        today = getToday();
        files.load(menu, orderManager, recipeBook, inventory);
        res.set_content("{\"ok\":true,\"message\":\"Data reloaded from text files\"}", "application/json");
    });

    svr.Post("/api/ingredient/add", [](const httplib::Request& req, httplib::Response& res)
    {
        today = getToday();
        string name = getText(req, "name");
        string unit = getText(req, "unit", "g");
        int id = getParam(req, "id"), minimum = getParam(req, "min"), cost = getParam(req, "cost");
        if (id <= 0 || name == "" || minimum < 0 || cost < 0)
            res.set_content(errorJson("Enter a positive ID, a name and non-negative numbers"), "application/json");
        else if (!inventory.addIngredient(id, name, unit, minimum, cost))
            res.set_content(errorJson("Ingredient ID already exists"), "application/json");
        else
        {
            files.save(menu, orderManager, recipeBook, inventory);
            res.set_content("{\"ok\":true}", "application/json");
        }
    });

    svr.Post("/api/stock/receive", [&](const httplib::Request& req, httplib::Response& res)
    {
        today = getToday();
        int expiry = getParam(req, "expiry");
        bool ok = (expiry >= 20000101 && expiry <= 29991231) &&
                  inventory.addStock(getParam(req, "batchId"), getParam(req, "ingId"),
                                     getParam(req, "qty"), expiry, today);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content(ok ? "{\"ok\":true}" : errorJson("Check ingredient ID, quantity, expiry (YYYYMMDD) and that the batch ID is new"), "application/json");
    });

    svr.Post("/api/stock/use", [&](const httplib::Request& req, httplib::Response& res)
    {
        today = getToday();
        bool ok = inventory.useStock(getParam(req, "ingId"), getParam(req, "qty"), today);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content(ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"Not enough usable stock\"}", "application/json");
    });

    svr.Post("/api/stock/waste", [&](const httplib::Request& req, httplib::Response& res)
    {
        today = getToday();
        string note = getText(req, "note", "Spoiled");
        bool ok = inventory.recordWastage(getParam(req, "ingId"), getParam(req, "qty"), today, note);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content(ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"Not enough stock\"}", "application/json");
    });

    svr.Post("/api/stock/expire", [&](const httplib::Request&, httplib::Response& res)
    {
        today = getToday();
        inventory.writeOffExpired(today);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content("{\"ok\":true}", "application/json");
    });

    svr.Post("/api/stock/undo", [&](const httplib::Request&, httplib::Response& res)
    {
        today = getToday();
        bool ok = inventory.undoLastStock(today);
        files.save(menu, orderManager, recipeBook, inventory);
        res.set_content(ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"Cannot undo\"}", "application/json");
    });

    svr.Post("/api/export/save", [](const httplib::Request& req, httplib::Response& res)
    {
        ofstream outFile("data/website_export.txt");
        if (!outFile.is_open())
            outFile.open("website_export.txt");
        if (outFile.is_open())
        {
            outFile << req.body;
            outFile.close();
            res.set_content("{\"ok\":true,\"file\":\"website_export.txt\"}", "application/json");
        }
        else
        {
            res.set_content("{\"ok\":false,\"error\":\"Could not write file on server\"}", "application/json");
        }
    });

    cout << "Server running at http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}
