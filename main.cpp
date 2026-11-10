// main.cpp - SMART RESTAURANT FOOD INVENTORY SYSTEM
//
// This file only shows the menus and connects all the modules:
//     menu + orders -> recipes + demand -> inventory (stock)
//     and login, files, backup, reports
//
// Build :  g++ -std=c++11 -o restaurant main.cpp        (or: make)
// Run   :  ./restaurant      (run it in the folder that has the data .txt files)

#include "common.h"
#include "menu.h"
#include "order.h"
#include "recipe.h"
#include "ingredient.h"
#include "transaction.h"
#include "stock_structures.h"
#include "inventory_manager.h"
#include "demand.h"
#include "user.h"
#include "audit_log.h"
#include "file_manager.h"
#include "backup_manager.h"
#include "report.h"

// ---------- one global object for each module ----------
FoodMenu menu;
OrderBook orders;
RecipeBook recipes;
Inventory inventory;
LoginManager logins;
AuditLog audit;
FileManager files(menu, orders, recipes, inventory, logins);
BackupManager backup;
DemandCalculator calculator;

// ---------- small helpers ----------
bool allowed(int section)
{
	if (logins.check_permission(section)) return true;
	cout << "\nAccess denied: role " << logins.current_role() << " cannot use this menu.\n";
	return false;
}

// writes "base_123" to the audit log
void log_id(const char base[], long id)
{
	char action[WORD_LEN], number[24];
	copy_text(action, base, WORD_LEN);
	append_text(action, "_", WORD_LEN);
	number_to_text(id, number);
	append_text(action, number, WORD_LEN);
	audit.write(logins.current_name(), action);
}

void log_text(const char action[])
{
	audit.write(logins.current_name(), action);
}

// ============================================================
//  1. FOOD MENU
// ============================================================
bool food_is_in_orders(int food_id)
{
	for (int i = 0; i < orders.size(); i++)
		for (int j = 0; j < orders.at(i).get_line_count(); j++)
			if (orders.at(i).get_line(j).get_food_id() == food_id) return true;
	return false;
}

void print_food_line(int i)
{
	cout << "  #" << menu.at(i).get_id() << " " << menu.at(i).get_name() << " - Rs " << menu.at(i).get_price()
	     << (menu.at(i).is_available() ? "" : " (unavailable)") << "\n";
}

void food_screen()
{
	if (!allowed(SEC_FOOD)) return;
	while (true)
	{
		cout << "\n===== FOOD MENU =====\n";
		cout << "1. Show menu (sorted)\n";
		cout << "2. Add food\n";
		cout << "3. Update food\n";
		cout << "4. Remove food\n";
		cout << "5. Search by ID   (binary search)\n";
		cout << "6. Search by name (linear search)\n";
		cout << "0. Back\n";
		int choice = read_number("Choice: ", 0, 6);

		if (choice == 0) return;

		if (choice == 1)
		{
			int how = read_number("Sort by 1 = ID, 2 = name, 3 = price: ", 1, 3);
			menu.show_menu(how);
		}
		else if (choice == 2)
		{
			char name[NAME_LEN];
			int id = menu.next_food_id();
			cout << "New food gets ID " << id << ".\n";
			read_name("Name (one word, use _ for spaces): ", name, NAME_LEN);
			int price = read_number("Price (Rs): ", 0, 100000);
			if (menu.add_food(id, name, price, true))
			{
				cout << "Food added.\n";
				log_id("added_food", id);
			}
			else cout << "Could not add the food (menu full).\n";
		}
		else if (choice == 3)
		{
			int id = read_number("Food ID to update: ", 1, 100000);
			int i = menu.find_by_id(id);
			if (i == -1)
			{
				cout << "No such food.\n";
				continue;
			}
			char name[NAME_LEN];
			read_name("New name: ", name, NAME_LEN);
			int price = read_number("New price (Rs): ", 0, 100000);
			bool available = ask_yes_no("Is it available");
			menu.update_food(id, name, price, available);
			cout << "Food updated. (Old orders keep their old price.)\n";
			log_id("updated_food", id);
		}
		else if (choice == 4)
		{
			int id = read_number("Food ID to remove: ", 1, 100000);
			if (menu.find_by_id(id) == -1)
			{
				cout << "No such food.\n";
				continue;
			}
			if (food_is_in_orders(id))
			{
				cout << "This food is used in orders. Use 'Update food' and mark it unavailable instead.\n";
				continue;
			}
			menu.remove_food(id);
			if (recipes.find(id) != -1) recipes.remove_recipe(id);
			cout << "Food removed (and its recipe).\n";
			log_id("removed_food", id);
		}
		else if (choice == 5)
		{
			int i = menu.find_by_id(read_number("Food ID: ", 1, 100000));
			if (i == -1) cout << "Not found.\n";
			else print_food_line(i);
		}
		else if (choice == 6)
		{
			char part[NAME_LEN];
			read_name("Name contains: ", part, NAME_LEN);
			int i = menu.find_by_name(part, 0);
			if (i == -1) cout << "Not found.\n";
			while (i != -1)
			{
				print_food_line(i);
				i = menu.find_by_name(part, i + 1);
			}
		}
	}
}

// ============================================================
//  INTEGRATION: prepare an order
//  demand of the order -> check stock -> use stock (FEFO) -> mark COMPLETED
//  This is the ONLY place where an order uses up stock.
// ============================================================
bool prepare_order(int order_id)
{
	int index = orders.find_by_id(order_id);
	if (index == -1)
	{
		cout << "No such order.\n";
		return false;
	}
	Order& o = orders.at(index);
	if (o.get_status() != CONFIRMED)
	{
		cout << "Order " << order_id << " is " << status_text(o.get_status()) << ". Only CONFIRMED orders can be prepared.\n";
		return false;
	}

	HashTable need;                                      // ingredient ID -> quantity needed
	int missing = calculator.add_order(o, recipes, need);
	if (missing > 0) cout << "Warning: " << missing << " food(s) in this order have no recipe, so they use no stock.\n";
	Pair rows[TABLE_SIZE];
	int n = need.to_pairs(rows);

	if (!inventory.has_enough(rows, n))
	{
		cout << "Not enough usable stock for order " << order_id << ":\n";
		inventory.show_shortage(rows, n);
		cout << "\nSuggested purchases:\n";
		inventory.suggest_replenishment(rows, n);
		return false;
	}

	char note[WORD_LEN], number[24];
	copy_text(note, "order_", WORD_LEN);
	number_to_text(order_id, number);
	append_text(note, number, WORD_LEN);
	for (int i = 0; i < n; i++)
		inventory.use_stock(rows[i].key, rows[i].value, today, note, false);   // FEFO, no undo
	orders.complete_order(order_id);
	cout << "Order " << order_id << " prepared. Stock used (earliest expiry first). Status: COMPLETED.\n";
	log_id("prepared_order", order_id);
	return true;
}

// ============================================================
//  2. ORDERS
// ============================================================
void place_new_order()
{
	Order draft(0, today, PENDING);
	menu.show_menu(1);
	cout << "\nAdd foods to the order. Type food ID 0 when you are finished.\n";
	while (true)
	{
		int food_id = read_number("Food ID (0 = done): ", 0, 100000);
		if (food_id == 0) break;
		int i = menu.find_by_id(food_id);
		if (i == -1)
		{
			cout << "  No such food.\n";
			continue;
		}
		if (!menu.at(i).is_available())
		{
			cout << "  That food is unavailable.\n";
			continue;
		}
		int qty = read_number("Quantity: ", 1, 1000);
		if (!draft.add_line(food_id, qty, menu.at(i).get_price()))
			cout << "  Order is full (max " << MAX_LINES << " different foods).\n";
	}
	if (draft.get_line_count() == 0)
	{
		cout << "Nothing ordered.\n";
		return;
	}
	int id = orders.place_order(draft);
	if (id == 0) cout << "Order book is full.\n";
	else
	{
		cout << "Order #" << id << " placed. Status PENDING (added to the queue).\n";
		log_id("placed_order", id);
	}
}

void order_screen()
{
	if (!allowed(SEC_ORDERS)) return;
	while (true)
	{
		cout << "\n===== ORDERS =====\n";
		cout << "1. Place new order\n";
		cout << "2. Add a food to an order\n";
		cout << "3. Change quantity in an order (0 = remove the food)\n";
		cout << "4. Cancel an order\n";
		cout << "5. Confirm next PENDING order (queue: first come, first served)\n";
		cout << "6. Confirm a chosen order\n";
		cout << "7. Prepare a CONFIRMED order (use stock + print bill)\n";
		cout << "8. Show pending queue\n";
		cout << "9. Order history\n";
		cout << "10. Print a bill\n";
		cout << "0. Back\n";
		int choice = read_number("Choice: ", 0, 10);

		if (choice == 0) return;

		if (choice == 1) place_new_order();
		else if (choice == 2)
		{
			int id = read_number("Order ID: ", 1, 1000000);
			menu.show_menu(1);
			int food_id = read_number("Food ID to add: ", 1, 100000);
			int f = menu.find_by_id(food_id);
			if (f == -1 || !menu.at(f).is_available())
			{
				cout << "No such food, or it is unavailable.\n";
				continue;
			}
			int qty = read_number("Quantity: ", 1, 1000);
			if (orders.add_line_to_order(id, food_id, qty, menu.at(f).get_price()))
			{
				cout << "Food added to order " << id << ". (Check the stock again before preparing.)\n";
				log_id("changed_order", id);
			}
			else cout << "Cannot change that order (no such order, closed, or full).\n";
		}
		else if (choice == 3)
		{
			int id = read_number("Order ID: ", 1, 1000000);
			int food_id = read_number("Food ID in the order: ", 1, 100000);
			int qty = read_number("New quantity (0 = remove): ", 0, 1000);
			if (orders.update_order(id, food_id, qty))
			{
				cout << "Order updated.\n";
				log_id("changed_order", id);
			}
			else cout << "Cannot update (no such order or food, order closed, or it is the last food).\n";
		}
		else if (choice == 4)
		{
			int id = read_number("Order ID to cancel: ", 1, 1000000);
			if (orders.cancel_order(id))
			{
				cout << "Order cancelled. It no longer counts as demand.\n";
				log_id("cancelled_order", id);
			}
			else cout << "Cannot cancel (no such order, or it is already completed / cancelled).\n";
		}
		else if (choice == 5)
		{
			int id = orders.process_next();
			if (id == 0) cout << "No pending orders.\n";
			else
			{
				cout << "Order #" << id << " was first in the queue. It is now CONFIRMED.\n";
				log_id("confirmed_order", id);
			}
		}
		else if (choice == 6)
		{
			int id = read_number("Order ID to confirm: ", 1, 1000000);
			if (orders.confirm_order(id))
			{
				cout << "Order confirmed.\n";
				log_id("confirmed_order", id);
			}
			else cout << "Only PENDING orders can be confirmed.\n";
		}
		else if (choice == 7)
		{
			int id = read_number("Order ID: ", 1, 1000000);
			if (prepare_order(id)) print_bill(orders.at(orders.find_by_id(id)), menu);
		}
		else if (choice == 8) orders.show_queue();
		else if (choice == 9)
		{
			int f = read_number("Show 0 = all, 1 = pending, 2 = confirmed, 3 = cancelled, 4 = completed: ", 0, 4);
			int filter = -1;
			if (f == 1) filter = PENDING;
			if (f == 2) filter = CONFIRMED;
			if (f == 3) filter = CANCELLED;
			if (f == 4) filter = COMPLETED;
			orders.show_history(menu, filter);
		}
		else if (choice == 10)
		{
			int i = orders.find_by_id(read_number("Order ID: ", 1, 1000000));
			if (i == -1) cout << "No such order.\n";
			else if (orders.at(i).get_status() == CANCELLED) cout << "Cancelled orders have no bill.\n";
			else print_bill(orders.at(i), menu);
		}
	}
}

// ============================================================
//  3. RECIPES AND DEMAND
// ============================================================
void edit_recipe_screen()
{
	menu.show_menu(1);
	int food_id = read_number("Food ID of the recipe: ", 1, 100000);
	if (menu.find_by_id(food_id) == -1)
	{
		cout << "No such food.\n";
		return;
	}
	int r = recipes.find(food_id);
	if (r == -1) r = recipes.add_recipe(food_id);
	if (r == -1)
	{
		cout << "Recipe book is full.\n";
		return;
	}
	inventory.show_stock();
	cout << "\nQuantities are for ONE serving. Type ingredient ID 0 when finished.\n";
	while (true)
	{
		int ing_id = read_number("Ingredient ID (0 = done): ", 0, 100000);
		if (ing_id == 0) break;
		if (inventory.find_ingredient(ing_id) == -1)
		{
			cout << "  No such ingredient.\n";
			continue;
		}
		cout << "Quantity per serving in " << inventory.unit_of(ing_id) << " (0 = remove from recipe): ";
		int qty = read_number("", 0, 1000000);
		bool ok;
		if (qty == 0) ok = recipes.at(r).remove_item(ing_id);
		else ok = recipes.at(r).set_item(ing_id, qty);
		if (!ok) cout << "  Could not change the recipe (recipe full, or ingredient not in it).\n";
	}
	recipes.show_recipe(r, menu, inventory);
	log_id("edited_recipe", food_id);
}

void recipe_screen()
{
	if (!allowed(SEC_RECIPES)) return;
	while (true)
	{
		cout << "\n===== RECIPES AND DEMAND =====\n";
		cout << "1. Show all recipes\n";
		cout << "2. Add / edit a recipe\n";
		cout << "3. Delete a recipe\n";
		cout << "4. Ingredient demand of CONFIRMED orders\n";
		cout << "5. Top-K popular foods\n";
		cout << "6. Compare demand of two date ranges\n";
		cout << "7. Average daily demand (simple forecast)\n";
		cout << "0. Back\n";
		int choice = read_number("Choice: ", 0, 7);

		if (choice == 0) return;

		if (choice == 1) recipes.show_all(menu, inventory);
		else if (choice == 2) edit_recipe_screen();
		else if (choice == 3)
		{
			int id = read_number("Food ID of the recipe to delete: ", 1, 100000);
			if (recipes.remove_recipe(id))
			{
				cout << "Recipe deleted.\n";
				log_id("deleted_recipe", id);
			}
			else cout << "No recipe for that food.\n";
		}
		else if (choice == 4)
		{
			DemandReport demand(orders, recipes, inventory);
			demand.show();
		}
		else if (choice == 5)
		{
			PopularFoodsReport popular(orders, menu, read_number("How many top foods (K)? ", 1, 20));
			Report* r = &popular;                            // a Report pointer can point to any report
			r->show();
		}
		else if (choice == 6)
		{
			DemandReport demand(orders, recipes, inventory);
			int from1 = read_date("Period 1 start (YYYYMMDD): ");
			int to1 = read_date("Period 1 end   (YYYYMMDD): ");
			int from2 = read_date("Period 2 start (YYYYMMDD): ");
			int to2 = read_date("Period 2 end   (YYYYMMDD): ");
			if (to1 < from1 || to2 < from2) cout << "An end date must not be before its start date.\n";
			else demand.compare_demand(from1, to1, from2, to2);
		}
		else if (choice == 7)
		{
			DemandReport demand(orders, recipes, inventory);
			int from = read_date("Start date (YYYYMMDD): ");
			int to = read_date("End date   (YYYYMMDD): ");
			demand.daily_average(from, to);
		}
	}
}

// ============================================================
//  4. INVENTORY
// ============================================================
void inventory_screen()
{
	if (!allowed(SEC_INVENTORY)) return;
	while (true)
	{
		cout << "\n===== INVENTORY =====\n";
		cout << "1. Stock summary\n";
		cout << "2. Show batches (earliest expiry first)\n";
		cout << "3. Add ingredient\n";
		cout << "4. Update ingredient\n";
		cout << "5. Delete ingredient\n";
		cout << "6. Receive new stock (new batch)\n";
		cout << "7. Use stock (FEFO)\n";
		cout << "8. Record wastage of a batch\n";
		cout << "9. Write off ALL expired stock\n";
		cout << "10. Undo last stock operation (stack)\n";
		cout << "11. Low-stock and expiry alerts\n";
		cout << "12. Transaction history\n";
		cout << "0. Back\n";
		int choice = read_number("Choice: ", 0, 12);

		if (choice == 0) return;

		if (choice == 1) inventory.show_stock();
		else if (choice == 2) inventory.show_batches(0);
		else if (choice == 3)
		{
			char name[NAME_LEN], unit[8];
			int id = inventory.next_ingredient_id();
			cout << "New ingredient gets ID " << id << ".\n";
			read_name("Name (one word): ", name, NAME_LEN);
			read_name("Unit (g / ml / piece): ", unit, 8);
			long min_stock = read_number("Minimum stock level: ", 0, 1000000000);
			int cost = read_number("Cost in Rs per kg / litre / piece: ", 0, 1000000);
			if (inventory.add_ingredient(id, name, unit, min_stock, cost))
			{
				cout << "Ingredient added.\n";
				log_id("added_ingredient", id);
			}
			else cout << "Could not add (list full, or the unit is not g / ml / piece).\n";
		}
		else if (choice == 4)
		{
			int id = read_number("Ingredient ID to update: ", 1, 100000);
			if (inventory.find_ingredient(id) == -1)
			{
				cout << "No such ingredient.\n";
				continue;
			}
			char name[NAME_LEN];
			read_name("New name: ", name, NAME_LEN);
			long min_stock = read_number("New minimum stock level: ", 0, 1000000000);
			int cost = read_number("New cost (Rs per kg / litre / piece): ", 0, 1000000);
			inventory.update_ingredient(id, name, min_stock, cost);
			cout << "Ingredient updated.\n";
			log_id("updated_ingredient", id);
		}
		else if (choice == 5)
		{
			int id = read_number("Ingredient ID to delete: ", 1, 100000);
			bool used = false;                               // is it inside a recipe?
			for (int i = 0; i < recipes.size(); i++)
				if (recipes.at(i).find_item(id) != -1) used = true;
			if (used) cout << "Cannot delete: a recipe uses this ingredient.\n";
			else if (inventory.delete_ingredient(id))
			{
				cout << "Deleted.\n";
				log_id("deleted_ingredient", id);
			}
			else cout << "Cannot delete (no such ingredient, or it still has stock).\n";
		}
		else if (choice == 6)
		{
			inventory.show_stock();
			int id = read_number("Ingredient ID: ", 1, 100000);
			if (inventory.find_ingredient(id) == -1)
			{
				cout << "No such ingredient.\n";
				continue;
			}
			cout << "Quantity received in " << inventory.unit_of(id) << ": ";
			long qty = read_number("", 1, 1000000000);
			int expiry = read_date("Expiry date (YYYYMMDD): ");
			int batch_id = inventory.receive_stock(id, qty, expiry, today);
			if (batch_id != 0)
			{
				cout << "Stock received as batch " << batch_id << ".\n";
				log_id("received_batch", batch_id);
			}
			else cout << "Could not receive (expiry date must not be in the past, or batch list is full).\n";
		}
		else if (choice == 7)
		{
			int id = read_number("Ingredient ID: ", 1, 100000);
			if (inventory.find_ingredient(id) == -1)
			{
				cout << "No such ingredient.\n";
				continue;
			}
			cout << "Usable stock: " << inventory.usable_stock(id) << " " << inventory.unit_of(id) << "\n";
			long qty = read_number("Quantity to use: ", 1, 1000000000);
			if (inventory.use_stock(id, qty, today, "manual_use", true))
			{
				cout << "Stock used (batches that expire first were used first).\n";
				log_id("used_stock_of", id);
			}
			else cout << "Not enough usable (non-expired) stock. Nothing was changed.\n";
		}
		else if (choice == 8)
		{
			inventory.show_batches(0);
			int batch_id = read_number("Batch ID: ", 1, 1000000);
			long qty = read_number("Quantity wasted: ", 1, 1000000000);
			char reason[WORD_LEN];
			read_name("Reason (one word, example spoiled): ", reason, WORD_LEN);
			if (inventory.record_wastage(batch_id, qty, reason, today))
			{
				cout << "Wastage recorded.\n";
				log_id("wasted_batch", batch_id);
			}
			else cout << "Could not record (no such batch, or quantity is more than the batch has).\n";
		}
		else if (choice == 9)
		{
			int n = inventory.waste_expired(today);
			cout << n << " expired batch(es) written off.\n";
			if (n > 0) log_text("wasted_expired_stock");
		}
		else if (choice == 10)
		{
			int n = inventory.undo_last(today);
			if (n > 0)
			{
				cout << "Last stock operation undone (" << n << " record(s)).\n";
				log_text("undo_stock_operation");
			}
			else cout << "Nothing to undo.\n";
		}
		else if (choice == 11)
		{
			cout << "\nLow stock:\n";
			inventory.check_low_stock();
			cout << "\nExpired or expiring within " << ALERT_DAYS << " days:\n";
			inventory.check_expiry(ALERT_DAYS);
		}
		else if (choice == 12)
		{
			int n = read_number("How many latest records (0 = all)? ", 0, 1000);
			inventory.show_transactions(n);
		}
	}
}

// ============================================================
//  5. WORKFLOW  (demand + stock together)
// ============================================================
void workflow_screen()
{
	if (!allowed(SEC_WORKFLOW)) return;
	while (true)
	{
		cout << "\n===== WORKFLOW: DEMAND vs STOCK =====\n";
		cout << "1. Check stock against CONFIRMED orders (shortage)\n";
		cout << "2. What should I buy? (demand + minimum levels)\n";
		cout << "3. Daily check (alerts + shortage + purchases)\n";
		cout << "0. Back\n";
		int choice = read_number("Choice: ", 0, 3);
		if (choice == 0) return;

		HashTable need;
		int missing = calculator.add_confirmed_orders(orders, recipes, need);
		Pair rows[TABLE_SIZE];
		int n = need.to_pairs(rows);
		if (missing > 0) cout << "Note: " << missing << " ordered food line(s) have no recipe.\n";

		if (choice == 1)
		{
			int shorts = inventory.show_shortage(rows, n);
			cout << "\nIngredients that are short: " << shorts << "\n";
		}
		else if (choice == 2)
		{
			cout << "\nBuy list (demand + minimum level - usable stock):\n";
			inventory.suggest_replenishment(rows, n);
		}
		else if (choice == 3)
		{
			cout << "\n--- Low stock ---\n";
			inventory.check_low_stock();
			cout << "\n--- Expired / expiring within " << ALERT_DAYS << " days ---\n";
			inventory.check_expiry(ALERT_DAYS);
			cout << "\n--- Stock against CONFIRMED orders ---";
			inventory.show_shortage(rows, n);
			cout << "\n--- What to buy ---\n";
			inventory.suggest_replenishment(rows, n);
		}
	}
}

// ============================================================
//  6. REPORTS  (polymorphism: one array of Report pointers)
// ============================================================
void report_screen()
{
	if (!allowed(SEC_REPORTS)) return;
	SalesReport sales(orders, menu);
	StockReport stock(inventory);
	ExpiryReport expiry(inventory, ALERT_DAYS);
	LowStockReport low(inventory);
	WastageReport wastage(inventory);
	DemandReport demand(orders, recipes, inventory);
	PopularFoodsReport popular(orders, menu, 3);
	Report* list[7] = { &sales, &stock, &expiry, &low, &wastage, &demand, &popular };

	cout << "\n===== REPORTS =====\n";
	cout << "1. Sales   2. Stock value   3. Expiry   4. Low stock   5. Wastage\n";
	cout << "6. Demand  7. Popular foods (top 3)   8. All reports   0. Back\n";
	int choice = read_number("Choice: ", 0, 8);
	if (choice == 0) return;
	if (choice == 8)
	{
		for (int i = 0; i < 7; i++) list[i]->show();
	}
	else list[choice - 1]->show();
	log_text("viewed_report");
}

// ============================================================
//  7. SYSTEM
// ============================================================
void system_screen()
{
	if (!allowed(SEC_SYSTEM)) return;
	while (true)
	{
		cout << "\n===== SYSTEM =====\n";
		cout << "1. Save data now\n";
		cout << "2. Recent activity\n";
		cout << "3. Full audit log\n";
		cout << "4. Create backup\n";
		cout << "5. Restore from backup\n";
		cout << "6. Check data consistency\n";
		cout << "7. Add a user\n";
		cout << "8. Change today's date (for demo / testing)\n";
		cout << "0. Back\n";
		int choice = read_number("Choice: ", 0, 8);
		if (choice == 0) return;

		if (choice == 1) files.save_all(true);
		else if (choice == 2) audit.show_recent();
		else if (choice == 3) audit.show_all();
		else if (choice == 4)
		{
			files.save_all(false);
			int n = backup.create_backup();
			cout << n << " of " << FILE_COUNT << " files backed up (backup_*.txt).\n";
			log_text("created_backup");
		}
		else if (choice == 5)
		{
			if (!ask_yes_no("This replaces ALL current data with the backup. Continue")) continue;
			int n = backup.restore_backup();
			if (n < FILE_COUNT) cout << "Warning: only " << n << " of " << FILE_COUNT << " backup files were found.\n";
			files.load_all();
			cout << "Data restored from the backup.\n";
			log_text("restored_backup");
		}
		else if (choice == 6)
		{
			cout << "\nChecking...\n";
			files.validate_data();
		}
		else if (choice == 7)
		{
			char u[NAME_LEN], p[NAME_LEN], r[12];
			read_name("New username: ", u, NAME_LEN);
			read_name("Password: ", p, NAME_LEN);
			read_name("Role (ADMIN / ORDER / INVENTORY): ", r, 12);
			make_upper(r);
			if (logins.add_user(u, p, r))
			{
				cout << "User added.\n";
				log_text("added_user");
			}
			else cout << "Could not add (name used, list full, or wrong role).\n";
		}
		else if (choice == 8)
		{
			today = read_date("New date (YYYYMMDD): ");
			cout << "Today is now ";
			print_date(today);
			cout << " (until you close the program).\n";
			log_text("changed_date");
		}
	}
}

// ============================================================
//  main
// ============================================================
int main()
{
	cout << "==========================================\n";
	cout << "   SMART RESTAURANT FOOD INVENTORY SYSTEM\n";
	cout << "==========================================\n";
	today = read_date("Enter today's date (YYYYMMDD, example 20261105): ");
	files.load_all();

	if (!logins.login())
	{
		cout << "Login failed. Closing the program.\n";
		return 1;
	}
	cout << "\nWelcome " << logins.current_name() << " (" << logins.current_role() << "). Today is ";
	print_date(today);
	cout << ".\n";
	log_text("logged_in");

	while (true)
	{
		cout << "\n===== MAIN MENU =====\n";
		cout << "1. Food menu\n";
		cout << "2. Orders\n";
		cout << "3. Recipes and demand\n";
		cout << "4. Inventory\n";
		cout << "5. Workflow: demand vs stock\n";
		cout << "6. Reports\n";
		cout << "7. System (save, audit, backup, users)\n";
		cout << "0. Save and exit\n";
		int choice = read_number("Choice: ", 0, 7);

		if (choice == 0) break;

		if (choice == 1) food_screen();
		else if (choice == 2) order_screen();
		else if (choice == 3) recipe_screen();
		else if (choice == 4) inventory_screen();
		else if (choice == 5) workflow_screen();
		else if (choice == 6) report_screen();
		else if (choice == 7) system_screen();
		files.save_all(false);                               // save after every visit to a menu
	}

	log_text("logged_out");
	files.save_all(true);
	cout << "Goodbye!\n";
	return 0;
}
