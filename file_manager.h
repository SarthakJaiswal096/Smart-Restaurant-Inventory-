// file_manager.h - FILE LOADING, SAVING AND VALIDATION
//
// OOP concepts : composition   (FileManager HAS references to all modules)
//
// All data is saved in text files. Every file starts with the number of records,
// then one record per line, fields separated by spaces (names are ONE word).

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "common.h"
#include "menu.h"
#include "order.h"
#include "recipe.h"
#include "inventory_manager.h"
#include "user.h"

// ============================================================
//  Reading words and numbers from a file
// ============================================================
bool file_word(ifstream& in, char dest[], int max)
{
	char c;
	int n = 0;
	bool found = false;
	while (in.get(c))
	{
		if (is_space(c))
		{
			if (found) break;
		}
		else
		{
			found = true;
			if (n < max - 1)
			{
				dest[n] = c;
				n++;
			}
		}
	}
	dest[n] = '\0';
	return found;
}

bool file_number(ifstream& in, long& value)
{
	char word[32];
	if (!file_word(in, word, 32)) return false;
	return text_to_number(word, value);
}

// ============================================================
//  FileManager : loads and saves the data of all modules
// ============================================================
class FileManager
{
private:
	FoodMenu& menu;
	OrderBook& orders;
	RecipeBook& recipes;
	Inventory& inv;
	LoginManager& logins;

	// opens a file and reads the record count. Returns false if the file is missing / empty.
	bool open_file(const char name[], ifstream& in, long& count)
	{
		in.open(name);
		if (!in)
		{
			cout << "  [" << name << "] not found - starting with no records.\n";
			return false;
		}
		if (!file_number(in, count))
		{
			cout << "  [" << name << "] is empty or damaged - starting with no records.\n";
			return false;
		}
		return true;
	}

	void report(const char name[], int ok, int bad)
	{
		cout << "  [" << name << "] " << ok << " record(s) loaded";
		if (bad > 0) cout << ", " << bad << " bad record(s) skipped";
		cout << "\n";
	}

	void load_foods()
	{
		ifstream in;
		long count;
		if (!open_file(FOOD_FILE, in, count)) return;
		int ok = 0, bad = 0;
		for (long i = 0; i < count; i++)
		{
			long id, price, available;
			char name[NAME_LEN];
			if (!(file_number(in, id) && file_word(in, name, NAME_LEN) && file_number(in, price) && file_number(in, available)))
			{
				bad++;
				break;
			}
			if (menu.add_food(id, name, price, available != 0)) ok++;
			else bad++;
		}
		report(FOOD_FILE, ok, bad);
	}

	// order file: first number = number of order LINES. One line = orderId date status foodId qty price
	void load_orders()
	{
		ifstream in;
		long count;
		if (!open_file(ORDER_FILE, in, count)) return;
		int ok = 0, bad = 0;
		Order current;
		bool have_order = false;
		for (long i = 0; i < count; i++)
		{
			long id, date, food, qty, price;
			char status[16];
			if (!(file_number(in, id) && file_number(in, date) && file_word(in, status, 16) &&
			      file_number(in, food) && file_number(in, qty) && file_number(in, price)))
			{
				bad++;
				break;
			}
			int st = status_from_text(status);
			if (st == -1 || id <= 0 || !valid_date(date) || qty <= 0 || price < 0)
			{
				bad++;
				continue;
			}
			if (!have_order || current.get_id() != id)       // a new order starts here
			{
				if (have_order && !orders.add_loaded_order(current)) bad++;
				current = Order(id, date, st);
				have_order = true;
			}
			if (current.add_line(food, qty, price)) ok++;
			else bad++;
		}
		if (have_order && !orders.add_loaded_order(current)) bad++;
		orders.rebuild_queue();
		report(ORDER_FILE, ok, bad);
	}

	void load_recipes()
	{
		ifstream in;
		long count;
		if (!open_file(RECIPE_FILE, in, count)) return;
		int ok = 0, bad = 0;
		for (long i = 0; i < count; i++)
		{
			long food, ing, qty;
			if (!(file_number(in, food) && file_number(in, ing) && file_number(in, qty)))
			{
				bad++;
				break;
			}
			int r = recipes.find(food);
			if (r == -1) r = recipes.add_recipe(food);
			if (r != -1 && recipes.at(r).set_item(ing, qty)) ok++;
			else bad++;
		}
		report(RECIPE_FILE, ok, bad);
	}

	void load_ingredients()
	{
		ifstream in;
		long count;
		if (!open_file(ING_FILE, in, count)) return;
		int ok = 0, bad = 0;
		for (long i = 0; i < count; i++)
		{
			long id, min_stock, cost;
			char name[NAME_LEN], unit[8];
			if (!(file_number(in, id) && file_word(in, name, NAME_LEN) && file_word(in, unit, 8) &&
			      file_number(in, min_stock) && file_number(in, cost)))
			{
				bad++;
				break;
			}
			if (inv.add_ingredient(id, name, unit, min_stock, cost)) ok++;
			else bad++;
		}
		report(ING_FILE, ok, bad);
	}

	void load_batches()
	{
		ifstream in;
		long count;
		if (!open_file(BATCH_FILE, in, count)) return;
		int ok = 0, bad = 0;
		for (long i = 0; i < count; i++)
		{
			long id, ing, qty, expiry;
			if (!(file_number(in, id) && file_number(in, ing) && file_number(in, qty) && file_number(in, expiry)))
			{
				bad++;
				break;
			}
			if (valid_date(expiry) && inv.add_batch_raw(id, ing, qty, expiry)) ok++;
			else bad++;
		}
		report(BATCH_FILE, ok, bad);
	}

	void load_transactions()
	{
		ifstream in;
		long count;
		if (!open_file(TXN_FILE, in, count)) return;
		int ok = 0, bad = 0;
		for (long i = 0; i < count; i++)
		{
			long id, date, ing, batch, qty;
			char type[16], note[WORD_LEN];
			if (!(file_number(in, id) && file_number(in, date) && file_word(in, type, 16) &&
			      file_number(in, ing) && file_number(in, batch) && file_number(in, qty) && file_word(in, note, WORD_LEN)))
			{
				bad++;
				break;
			}
			if (inv.add_txn_raw(id, date, type, ing, batch, qty, note)) ok++;
			else bad++;
		}
		report(TXN_FILE, ok, bad);
	}

	void load_users()
	{
		ifstream in;
		long count;
		if (!open_file(USER_FILE, in, count))
		{
			cout << "  Creating the default demo users.\n";
			logins.create_default_users();
			return;
		}
		int ok = 0, bad = 0;
		for (long i = 0; i < count; i++)
		{
			char u[NAME_LEN], p[NAME_LEN], r[12];
			if (!(file_word(in, u, NAME_LEN) && file_word(in, p, NAME_LEN) && file_word(in, r, 12)))
			{
				bad++;
				break;
			}
			make_upper(r);
			if (logins.add_user(u, p, r)) ok++;
			else bad++;
		}
		if (logins.size() == 0) logins.create_default_users();
		report(USER_FILE, ok, bad);
	}
public:
	FileManager(FoodMenu& m, OrderBook& o, RecipeBook& r, Inventory& i, LoginManager& l)
		: menu(m), orders(o), recipes(r), inv(i), logins(l) {}

	// Reads every file. A missing file gives empty data, a bad record is skipped.
	void load_all()
	{
		cout << "\nLoading data files...\n";
		char who[NAME_LEN];
		copy_text(who, logins.current_name(), NAME_LEN);     // remember who is logged in
		menu.clear();
		orders.clear();
		recipes.clear();
		inv.clear();
		logins.clear();
		load_users();
		logins.resume_session(who);
		load_foods();
		load_orders();
		load_recipes();
		load_ingredients();                                  // ingredients before batches
		load_batches();
		load_transactions();
	}

	// Writes every file. Returns true if all files could be written.
	bool save_all(bool talk)
	{
		bool ok = true;
		ofstream out;

		out.open(FOOD_FILE);
		if (out)
		{
			out << menu.size() << "\n";
			for (int i = 0; i < menu.size(); i++)
				out << menu.at(i).get_id() << " " << menu.at(i).get_name() << " " << menu.at(i).get_price()
				    << " " << (menu.at(i).is_available() ? 1 : 0) << "\n";
		}
		else ok = false;
		out.close();

		out.open(ORDER_FILE);
		if (out)
		{
			int lines = 0;
			for (int i = 0; i < orders.size(); i++) lines += orders.at(i).get_line_count();
			out << lines << "\n";
			for (int i = 0; i < orders.size(); i++)
			{
				Order& o = orders.at(i);
				for (int j = 0; j < o.get_line_count(); j++)
					out << o.get_id() << " " << o.get_date() << " " << status_text(o.get_status()) << " "
					    << o.get_line(j).get_food_id() << " " << o.get_line(j).get_qty() << " "
					    << o.get_line(j).get_price() << "\n";
			}
		}
		else ok = false;
		out.close();

		out.open(RECIPE_FILE);
		if (out)
		{
			int items = 0;
			for (int i = 0; i < recipes.size(); i++) items += recipes.at(i).get_count();
			out << items << "\n";
			for (int i = 0; i < recipes.size(); i++)
			{
				Recipe& r = recipes.at(i);
				for (int j = 0; j < r.get_count(); j++)
					out << r.get_food_id() << " " << r.get_ing_id(j) << " " << r.get_qty(j) << "\n";
			}
		}
		else ok = false;
		out.close();

		out.open(ING_FILE);
		if (out)
		{
			out << inv.ingredient_count() << "\n";
			for (int i = 0; i < inv.ingredient_count(); i++)
			{
				Ingredient& g = inv.ingredient_at(i);
				out << g.get_id() << " " << g.get_name() << " " << g.get_unit() << " "
				    << g.get_min_stock() << " " << g.get_cost() << "\n";
			}
		}
		else ok = false;
		out.close();

		out.open(BATCH_FILE);
		if (out)
		{
			int kept = 0;
			for (int i = 0; i < inv.batch_total(); i++)
				if (inv.batch_at(i).get_qty() > 0) kept++;
			out << kept << "\n";
			for (int i = 0; i < inv.batch_total(); i++)
			{
				Batch& b = inv.batch_at(i);
				if (b.get_qty() > 0)
					out << b.get_id() << " " << b.get_ing_id() << " " << b.get_qty() << " " << b.get_expiry() << "\n";
			}
		}
		else ok = false;
		out.close();

		out.open(TXN_FILE);
		if (out)
		{
			out << inv.txn_total() << "\n";
			for (int i = 0; i < inv.txn_total(); i++)
			{
				Transaction& t = inv.txn_at(i);
				out << t.get_id() << " " << t.get_date() << " " << t.get_type() << " " << t.get_ing_id() << " "
				    << t.get_batch_id() << " " << t.get_qty() << " " << t.get_note() << "\n";
			}
		}
		else ok = false;
		out.close();

		out.open(USER_FILE);
		if (out)
		{
			out << logins.size() << "\n";
			for (int i = 0; i < logins.size(); i++)
				out << logins.at(i).get_username() << " " << logins.at(i).get_password() << " "
				    << logins.at(i).get_role() << "\n";
		}
		else ok = false;
		out.close();

		if (talk) cout << (ok ? "All data saved.\n" : "Some files could not be saved!\n");
		return ok;
	}

	// Checks that the records agree with each other. Prints every problem, returns how many.
	int validate_data()
	{
		int problems = 0;
		for (int i = 0; i < orders.size(); i++)
		{
			Order& o = orders.at(i);
			for (int j = 0; j < o.get_line_count(); j++)
				if (menu.find_by_id(o.get_line(j).get_food_id()) == -1)
				{
					cout << "  Order " << o.get_id() << " uses unknown food ID " << o.get_line(j).get_food_id() << "\n";
					problems++;
				}
		}
		for (int i = 0; i < recipes.size(); i++)
		{
			Recipe& r = recipes.at(i);
			if (menu.find_by_id(r.get_food_id()) == -1)
			{
				cout << "  Recipe for unknown food ID " << r.get_food_id() << "\n";
				problems++;
			}
			for (int j = 0; j < r.get_count(); j++)
				if (inv.find_ingredient(r.get_ing_id(j)) == -1)
				{
					cout << "  Recipe of food " << r.get_food_id() << " uses unknown ingredient ID " << r.get_ing_id(j) << "\n";
					problems++;
				}
		}
		for (int i = 0; i < inv.batch_total(); i++)
		{
			Batch& b = inv.batch_at(i);
			if (inv.find_ingredient(b.get_ing_id()) == -1)
			{
				cout << "  Batch " << b.get_id() << " belongs to unknown ingredient " << b.get_ing_id() << "\n";
				problems++;
			}
		}
		for (int i = 0; i < menu.size(); i++)
			if (menu.at(i).is_available() && recipes.find(menu.at(i).get_id()) == -1)
			{
				cout << "  Warning: food " << menu.at(i).get_name() << " has no recipe (its orders create no demand)\n";
				problems++;
			}
		if (problems == 0) cout << "  All records are consistent.\n";
		return problems;
	}
};

#endif
