SMART RESTAURANT FOOD INVENTORY SYSTEM (BEGINNER PBL VERSION)

See README.md for the full description, function tables and web API.

HOW TO RUN ON WINDOWS (g++ / MinGW needed):
1. run_console.bat   - C++ terminal app (main.cpp)
2. run_web.bat       - Web dashboard (server.cpp + httplib.h + website/), opens http://localhost:8080
3. run_modules.bat   - Small test program for each module (modules/ folder)
Always start from this folder, because the data/ path is relative.

HOW TO RUN ON LINUX / MAC:
   g++ -std=c++11 main.cpp -o restaurant && ./restaurant
   g++ -std=c++11 server.cpp -o restaurant_web && ./restaurant_web     (then open http://localhost:8080)
   bash tests/run_tests.sh                                               (build everything and run the checks)

DEMO LOGIN ACCOUNTS:
- Admin:        admin / admin123
- Staff:        staff / staff123
- Stock Filler: stock / stock123

OOP & DSA CONCEPTS INCLUDED:
- Inheritance & Polymorphism: Item -> Food/Ingredient; Report -> SalesReport/StockReport/ExpiryReport/LowStockReport/WastageReport
- Binary Search & Insertion Sort: menu.h
- Circular Queue (FIFO): order.h
- Hash Table (Linear Probing): recipe.h
- Min-Heap (FEFO) & Undo Stack (LIFO): inventory.h
- Manual Merge Sort: report.h
