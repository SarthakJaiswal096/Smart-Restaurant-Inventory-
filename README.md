# Smart Restaurant Food Inventory System

A C++ program that manages a restaurant's menu, recipes, customer orders and ingredient stock. It runs as a console app (`main.cpp`) and as a web dashboard (`server.cpp` + `website/`). Both use the same logic and the same data files.

## Project files

| File | What it contains |
|---|---|
| `models.h` | Data classes: food, ingredient, batch, order, stock log entry |
| `menu.h` | `FoodMenu`: the list of dishes |
| `recipe.h` | `RecipeBook` (dish to ingredient amounts) and `DemandHashTable` |
| `inventory.h` | `Inventory`: ingredients, stock batches, wastage, undo |
| `order.h` | `OrderManager`: place, confirm and cancel orders |
| `report.h` | Report classes: sales, stock value, expiry, low stock, wastage |
| `file_manager.h` | `FileManager`: save and load everything |
| `login.h` | `LoginManager`: the three user accounts |
| `main.cpp` | Console program |
| `server.cpp`, `httplib.h` | Web server and its small HTTP library |
| `website/` | `index.html`, `styles.css`, `app.js` (the dashboard) |
| `data/` | Text files holding the saved data |
| `run_console.bat`, `run_web.bat` | Compile and start each version |
| `modules/` | One small test program per module (menu and recipes, orders, inventory, reports) |
| `run_modules.bat` | Compile and start the module test programs |
| `tests/run_tests.sh` | Automatic build and smoke test (Linux, macOS or Git Bash) |

## How to run

- Console: double-click `run_console.bat`.
- Web dashboard: double-click `run_web.bat`. It compiles `server.cpp` (which uses `httplib.h` from the same folder) and opens `http://localhost:8080`.
- Module test programs: double-click `run_modules.bat` and pick a module.
- `website/index.html` also connects to the running server when it is opened directly from disk; with no server running it falls back to a browser-only preview.
- Always start the program from the project folder, because the `data/` path is relative.

Demo accounts:

| Role | Username | Password |
|---|---|---|
| ADMIN | admin | admin123 |
| STAFF | staff | staff123 |
| STOCK_FILLER | stock | stock123 |

---

## Functions by file

### models.h

| Class | Function | What it does |
|---|---|---|
| `Item` | `Item(id, name)` | Creates an item with an id and a name |
| | `getId()`, `getName()` | Return the id or the name |
| | `setName(name)` | Changes the name |
| | `show()` | Prints `id - name` |
| `Food` | `Food(id, name, price)` | Creates a dish |
| | `getPrice()`, `setPrice(price)` | Read or change the price |
| | `show()` | Prints the dish with its price |
| `Ingredient` | `Ingredient(id, name, unit, minimumStock, cost)` | Creates an ingredient |
| | `getUnit()`, `getMinimumStock()`, `getCostPerUnit()` | Read the fields |
| | `setMinimumStock(m)`, `setCostPerUnit(c)` | Change the fields |
| `Batch` | `Batch(id, ingredientId, quantity, expiry)` | One delivery of one ingredient |
| `OrderLine` | `OrderLine(foodId, quantity)` | One dish and how many plates |
| `Order` | `Order(id, date)` | Creates an order, status starts as `PENDING` |
| | `getId()`, `getStatus()`, `getDate()` | Read the fields |
| | `setStatus(s)`, `setDate(d)` | Change the fields |
| | `getItems()` | Returns the order lines |
| `StockTransaction` | `StockTransaction(type, ingredientId, quantity, date, note)` | One line of the stock log |

### menu.h (`FoodMenu`)

| Function | What it does |
|---|---|
| `addFood(id, name, price)` | Adds a dish and keeps the list sorted by id. Ignores a duplicate id |
| `searchFood(id)` | Returns the position of the dish, or -1 if not found |
| `getFood(id)` | Returns a pointer to the dish, or `NULL` |
| `updateFood(id, name, price)` | Changes the name and price of an existing dish |
| `showMenu()` | Prints all dishes |
| `getFoods()` | Returns the whole list |

### recipe.h

`DemandHashTable`

| Function | What it does |
|---|---|
| `add(key, amount)` | Adds an amount to the running total for an ingredient id (linear probing; safe for negative keys and a full table) |
| `get(key)` | Returns the total for an ingredient id |
| `show()` | Prints all totals |

`RecipeBook`

| Function | What it does |
|---|---|
| `addRecipe(foodId, ingredientId, quantity)` | Adds one ingredient requirement to a dish |
| `deleteRecipe(foodId)` | Removes all ingredient rows of a dish |
| `showRecipes()` | Prints all recipes |
| `getRecipes()` | Returns all recipe rows |

### inventory.h

Helper function and small classes

| Function | What it does |
|---|---|
| `inventoryDateNumber(date)` | Converts a `YYYYMMDD` date to a day number, so two dates can be subtracted |
| `BatchMinHeap::insert(batch)` | Adds a batch to the heap |
| `BatchMinHeap::removeTop()` | Removes and returns the batch with the earliest expiry |
| `BatchMinHeap::empty()` | True when the heap has no batches |
| `StockUndoStack::push(batch)` | Remembers a received batch |
| `StockUndoStack::pop()` | Returns the most recently received batch |
| `StockUndoStack::empty()` | True when there is nothing to undo |

`Inventory`

| Function | What it does |
|---|---|
| `addIngredient(id, name, unit, minimum, cost)` | Adds a new ingredient. Returns false (and prints "Ingredient already exists.") for a duplicate id |
| `findIngredient(id)` | Returns a pointer to the ingredient, or `NULL` |
| `addStock(batchId, ingredientId, quantity, expiry, date)` | Receives a new batch, logs `RECEIVED`, remembers it for undo. Rejects an unknown ingredient, a quantity of 0 or less and a duplicate batch id |
| `getStock(ingredientId)` | Total quantity of an ingredient, including expired batches |
| `getUsableStock(ingredientId, today)` | Total quantity that has not expired |
| `useStock(ingredientId, quantity, today)` | Uses stock from the earliest-expiring batch first, logs `USED` |
| `useStockFIFO(ingredientId, quantity, today)` | Uses stock in the order batches were received (used when confirming orders) |
| `recordWastage(ingredientId, quantity, today, note)` | Removes spoiled or lost stock, logs `WASTAGE` |
| `writeOffExpired(today)` | Sets expired batches to zero, logs `EXPIRED`, returns how many were cleared |
| `undoLastStock(today)` | Reverses the last received batch, logs `UNDO` |
| `showStock(today)` | Prints usable stock for every ingredient |
| `showBatches(today)` | Prints every batch with its expiry |
| `getIngredients()`, `getBatches()`, `getTransactions()` | Return the lists |
| `setTransactions(list)` | Replaces the stock log (used when loading) |
| `rebuildUndo()` | Resets the undo stack after loading |

### order.h

`OrderCircularQueue`

| Function | What it does |
|---|---|
| `push(orderId)` | Adds an order id at the back |
| `pop()` | Removes and returns the id at the front |
| `empty()` | True when no orders are waiting |

`OrderManager`

| Function | What it does |
|---|---|
| `placeOrder(items, today)` | Creates a `PENDING` order and returns its id (starts at 1001) |
| `confirmNext(recipes, inventory, today)` | Confirms the oldest waiting order. Returns its id, or -1 |
| `confirmOrder(id, recipes, inventory, today)` | Confirms the chosen `PENDING` order. Returns true or false |
| `cancelOrder(id)` | Cancels a `PENDING` order |
| `consumeOrderFIFO(order, recipes, inventory, today)` | Private. Totals the ingredients the order needs, checks all of them, and only then deducts the stock |
| `showOrders()` | Prints id, status and date of every order, with its food lines below |
| `getOrders()` | Returns the order list |
| `setOrders(list)` | Replaces the orders (used when loading) and rebuilds the waiting queue |

### report.h

| Function | What it does |
|---|---|
| `Report::show()` | Every report class has its own `show()` |
| `mergeSortSales(sales, left, right)` | Sorts the sales rows from highest amount to lowest |
| `mergeSales(sales, left, mid, right)` | Merges two sorted halves (used by `mergeSortSales`) |
| `SalesReport(menu, orders)` | Creates the sales report |
| `SalesReport::perDay(date)` | Total sales of confirmed orders on one date |
| `SalesReport::perFood()` | Plates sold and money earned for each dish, best seller first |
| `StockReport(inventory, today)::show()` | Value of usable stock per ingredient and the total |
| `ExpiryReport(inventory, today)::show()` | Batches that are expired or expire within 7 days |
| `LowStockReport(inventory, today)::show()` | Ingredients below their minimum stock |
| `WastageReport(inventory)::show()` | All wastage and expiry log lines with the total lost |

### file_manager.h (`FileManager`)

| Function | What it does |
|---|---|
| `save(menu, orders, recipes, inventory)` | Writes all data into the files in `data/` |
| `load(menu, orders, recipes, inventory)` | Clears memory and reads all files back in |

Files in `data/`: `foods.txt`, `ingredients.txt`, `batches.txt`, `recipes.txt`, `orders.txt`, `transactions.txt`. Each line is one record with fields separated by `|`.

### login.h

| Function | What it does |
|---|---|
| `User(username, password, role)` | Creates a user |
| `User::check(username, password)` | True if both match |
| `User::getRole()` | Returns the role |
| `LoginManager()` | Creates the three demo users |
| `LoginManager::login(username, password)` | Returns the role, or an empty string if login fails |

### main.cpp

| Function | What it does |
|---|---|
| `main()` | Login, load data, show the menu until 0, then save and exit |
| `placeOrder(today)` | Asks for dishes and quantities, then places one order |
| `calculateDemand()` | Totals the ingredient need of all confirmed orders |
| `averageDailyDemand()` | Total ingredient need divided by the number of days covered |
| `showReports(today)` | Reports sub-menu |
| `getTodayDate()` | Reads the system clock as a `YYYYMMDD` number |

### server.cpp and httplib.h

`server.cpp`

| Function | What it does |
|---|---|
| `main()` | Loads the data, registers all routes, starts the server on port 8080 |
| `getToday()` | Today's date as a `YYYYMMDD` number (read again on every request) |
| `getParam(req, key, default)` | Reads a whole number from the request, or returns the default. A non-number gives a clear error |
| `getText(req, key, default)` | Reads text and removes characters that would break the data files or the JSON (`\|`, quotes, `<`, `>`) |
| `jsonEsc(text)` | Makes any text safe inside a JSON string |
| `errorJson(message)` | Builds `{"ok":false,"error":"..."}` |

`httplib.h` (the link between the C++ code and the `website/` folder)

| Function | What it does |
|---|---|
| `Request::has_param(key)`, `get_param_value(key)` | Check for and read a request parameter. Values come from the URL query and from a form body |
| `Request::has_header(key)` | Check for a request header (names are lower case) |
| `Response::set_content(body, type)`, `set_header(key, value)` | Set what is sent back |
| `Server::set_mount_point(url, folder)` | Serves static files, here the `website/` folder. `/` opens `index.html`; `..` in a path is refused |
| `Server::Get(path, handler)`, `Post(path, handler)` | Register a route |
| `Server::listen(host, port)` | Accepts one request at a time. Sends the right status line, `Content-Length` and CORS header. An error inside a handler becomes a JSON error reply, not a crash. Idle browser connections time out after 1 second |

### website/app.js

| Function | What it does |
|---|---|
| `sync()` | Fetches `/api/state` and refreshes the page |
| `render()` | Draws the tables, numbers and cards |
| `postApi(url)` | Sends a POST request, then refreshes |
| `showAlert(message, ok)` | Shows a success or error message |
| `esc(text)` | Escapes text before it is put into the page |
| `dayNumber(date)` | Converts `YYYYMMDD` to a day number (same idea as `inventoryDateNumber`) |
| `quickOrder(foodId)` | Places an order for one plate of a dish |
| `orderAct(id, act)` | Confirms or cancels an order |

Dashboard tabs: Overview, Food Menu, Orders, Recipes, Inventory, Reports.

---

## Console menu

| No. | Option | Who can use it |
|---|---|---|
| 1 | Show food menu | All |
| 2 | Add food | ADMIN, STAFF |
| 3 | Update food | ADMIN |
| 4 | Place order | ADMIN, STAFF |
| 5 | Show orders | ADMIN, STAFF |
| 6 | Confirm next order | ADMIN, STAFF |
| 7 | Confirm chosen order | ADMIN, STAFF |
| 8 | Cancel order | ADMIN, STAFF |
| 9 | Show recipes | ADMIN, STAFF |
| 10 | Add recipe ingredient | ADMIN, STAFF |
| 11 | Delete complete recipe | ADMIN |
| 12 | Calculate demand | ADMIN, STAFF |
| 13 | Average daily demand | ADMIN, STAFF |
| 14 | Show stock | All |
| 15 | Add ingredient | ADMIN, STOCK_FILLER |
| 16 | Receive stock batch | ADMIN, STOCK_FILLER |
| 17 | Show batches | All |
| 18 | Use stock (FEFO) | ADMIN, STOCK_FILLER |
| 19 | Record wastage | ADMIN, STOCK_FILLER |
| 20 | Write off expired stock | ADMIN, STOCK_FILLER |
| 21 | Undo last stock receipt | ADMIN |
| 22 | Reports | All |
| 23 | Save all data | ADMIN |
| 24 | Load all data | ADMIN |
| 0 | Save and exit | All |

Reports sub-menu: 1 Sales per day, 2 Sales per food, 3 Stock value, 4 Expiry report, 5 Low stock report, 6 Wastage report.
