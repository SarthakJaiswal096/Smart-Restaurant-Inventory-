SMART RESTAURANT FOOD INVENTORY SYSTEM  -  SIMPLE VERSION 2 (all features, own functions)
=========================================================================================

BUILD AND RUN
  g++ -std=c++11 -o restaurant main.cpp        (or: make)
  ./restaurant        <- run inside this folder so it finds the .txt data files
  Windows: restaurant.exe

LOGIN (demo accounts)
  admin / admin123          ADMIN      everything
  order / order123          ORDER      food, orders, recipes, workflow, reports
  inventory / inv123        INVENTORY  recipes, inventory, workflow, reports
  (Passwords are plain text in users_data.txt. That is NOT secure, it is only a demo.)

THE IDEA IN ONE LINE
  Customer order -> recipe -> ingredient demand -> check stock -> use oldest stock first -> bill

TRY THIS (the sample data is made for it)
  Start the program, type the date 20261105, then login as admin.
   1. Recipes and demand (3) -> 4 : demand of confirmed order 1005
        rice 4000 g, chicken 5000 g, flour 2000 g, onion 1000 g, butter 200 g
   2. Orders (2) -> 7 -> 1005 : stock is used (oldest expiry first), order is COMPLETED,
        bill = 5800 + 5% GST = 6090.00
   3. Orders (2) -> 5 : takes order 1006 from the front of the queue and confirms it
   4. Workflow (5) -> 1 : butter is SHORT (needs 300 g, only 100 g left)
   5. Inventory (4) -> 6 : receive 1000 g of butter (ingredient 4, a later expiry date)
   6. Orders (2) -> 7 -> 1006 : now it works.
   7. Inventory (4) -> 10 : undo, 4 -> 7 : use stock manually, 4 -> 8 : wastage
   8. 0 = save and exit. Run again: all data is still there.

NO LIBRARY FUNCTIONS  (only cin, cout and file streams)
  The program includes only <iostream> and <fstream>. Everything else is written in common.h:
    text      text_length, copy_text, append_text, same_text, compare_text, contains_text,
              make_upper (text = char array ending with '\0', so names are ONE word: Veg_Pulao)
    numbers   number_to_text, text_to_number, money_to_text
              (money is kept in hundredths, so GST is exact: no decimal numbers needed)
    printing  print_left, print_right, print_left_number, print_right_number,
              print_right_money, print_line, print_date
    dates     is_leap_year, days_in_month, valid_date, day_number, days_between, add_days
              (that is how "expires in 3 days" works without a date library)
    input     read_word (reads letter by letter), read_number, read_name, read_date,
              ask_yes_no, and file_word / file_number for the data files
  Because input is read as words and checked by our own functions, typing letters instead of
  numbers never breaks the program.
  The program asks for today's date at the start (no clock function is used).
  Backup copies the files letter by letter (BackupManager::copy_file).

RULES (agreed by the team)
  * IDs are whole numbers. Dates are YYYYMMDD (example 20261105).
  * Quantities are whole numbers in g, ml or piece. Recipe quantities are PER ONE SERVING.
  * Order status: PENDING -> CONFIRMED -> COMPLETED   (or CANCELLED).
  * Only CONFIRMED orders count as active demand. Cancelled orders never count.
  * Calculating demand never changes the stock. Stock changes only by: receive, use,
    wastage, undo, or PREPARING an order.
  * Expired batches are never used. Stock never goes below zero.
    If there is not enough stock, NOTHING is changed.

FILES
  common.h              own helper functions, constants, data file names, abstract class Report
  menu.h                Item, Food, FoodMenu (sorted by ID, binary search)
  order.h               OrderLine, Order, CircularQueue, OrderBook, bill
  recipe.h              Recipe, RecipeBook
  ingredient.h          Ingredient, Batch
  transaction.h         Transaction (history of every stock change)
  stock_structures.h    MinHeap (FEFO), IdTable (hash table), UndoStack
  inventory_manager.h   Inventory (receive, use, wastage, undo, alerts)
  demand.h              HashTable, merge sort, DemandCalculator
  user.h                User, LoginManager, roles
  audit_log.h           AuditLog (recent activity stack + audit_log.txt)
  file_manager.h        FileManager (load / save / validate)
  backup_manager.h      BackupManager (backup and restore)
  report.h              Sales, Stock, Expiry, LowStock, Wastage, Demand, PopularFoods reports
  main.cpp              menus + connects all the modules
  *_data.txt            saved data. First number in a file = how many records follow.
                        Example food_data.txt:   6   then   1 Chicken_Biryani 250 1
  audit_log.txt         created by the program        backup_*.txt  created by "Create backup"

ALL FEATURES (nothing was dropped)
  Food menu      show sorted (ID/name/price), add, update, remove, binary search, name search
  Orders         place, add food, change quantity, cancel, FIFO confirm, confirm chosen order,
                 prepare (uses stock), queue view, history by status, bill with GST
  Recipes        add/edit/remove ingredients, delete recipe, demand, Top-K popular foods,
                 compare two date ranges, average daily demand (forecast)
  Inventory      ingredient add/update/delete, batches, receive, FEFO use, wastage, write off all
                 expired stock, undo (stack), low-stock and expiry alerts, transaction history
  Workflow       shortage check, what to buy, daily check
  Reports        sales (per day + per food, saved to sales_report.txt), stock value, expiry,
                 low stock, wastage, demand, popular foods
  System         save, recent activity, full audit log, backup, restore, consistency check,
                 add user, change date, login with roles

OOP CONCEPTS - WHERE
  Inheritance     Item -> Food (menu.h), Item -> Ingredient (ingredient.h)
  Polymorphism    Report (common.h) -> SalesReport, StockReport, ExpiryReport, LowStockReport,
                  WastageReport, DemandReport, PopularFoodsReport (all in report.h).
                  report_screen() keeps  Report* list[7]  and calls list[i]->show().
  Abstraction     Report has pure virtual functions (title, generate)
  Encapsulation   private / protected data with get_ and set_ functions in every class
  Composition     Order HAS OrderLines | OrderBook HAS CircularQueue | Inventory HAS Ingredients,
                  Batches, Transactions, IdTable and UndoStack | FileManager HAS the modules
  Constructors    default + with values in every class

DSA CONCEPTS - WHERE
  Circular queue  CircularQueue (order.h)        pending orders, first come first served
  Binary search   FoodMenu::find_by_id (menu.h) the menu array is always sorted by ID
  Linear search   find_by_name, RecipeBook::find, OrderBook::find_by_id, LoginManager::find_user
  Insertion sort  FoodMenu::show_menu / add_food, Inventory::sort_batches, SalesReport (by date)
  Merge sort      merge_sort (demand.h)            ranks demand, popular foods, sales
  Top-K           PopularFoodsReport             merge sort, then show the first K
  Hash table      HashTable (demand.h) key % 211  adds up demand, counts popularity, revenue
                  IdTable (stock_structures.h) key % 127  ingredient ID -> position
                  both solve collisions with linear probing
  Min-heap        MinHeap (stock_structures.h)           the batch that expires first is on top (FEFO)
  Stack           UndoStack (stock_structures.h)          undo last stock operation
                  AuditLog (audit_log.h)             recent activity, newest on top
  File handling   FileManager (file_manager.h)  load / save / validate, BackupManager

VIVA - ONE SENTENCE PER PART
  Orders   : "An order starts PENDING and waits in a circular queue. It is confirmed first come,
            first served. Foods are found with binary search because the menu is sorted by ID."
  Demand   : "Demand = quantity per serving x servings ordered. I add each ingredient up in a hash
            table and sort the result with merge sort."
  Inventory: "To use 6 kg of rice I put the non-expired batches in a min-heap and take from the top
            until 6 kg is reached. If there is not enough stock, nothing changes."
  System   : "Login with roles, the audit log, saving and loading the files, backup and
            restore, and the reports. Main.cpp connects all the modules."

LIMITS
  Max: 50 foods, 200 orders, 10 foods per order, 50 ingredients, 200 batches, 10 ingredients per
  recipe, 1000 transactions, 10 users. Names are one word (max 23 letters).
  Do not edit the data files while the program is running.
