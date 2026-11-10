// report.h - REPORTS
//
// OOP concepts : inheritance + polymorphism (every report derives from Report in common.h)
// DSA concepts : hash table + merge sort (rank foods by sales / demand), Top-K,
//                insertion sort (sales by date), linear search (report rows)

#ifndef REPORT_H
#define REPORT_H

#include "common.h"
#include "menu.h"
#include "order.h"
#include "recipe.h"
#include "inventory_manager.h"
#include "demand.h"

// ============================================================
//  SalesReport : money from COMPLETED orders (per day + per food)
// ============================================================
class SalesReport : public Report
{
private:
	OrderBook& orders;
	FoodMenu& menu;
public:
	SalesReport(OrderBook& o, FoodMenu& m) : orders(o), menu(m) {}

	const char* title() { return "SALES REPORT (completed orders)"; }

	void generate()
	{
		int dates[MAX_ORDERS];
		long cnt[MAX_ORDERS], sub[MAX_ORDERS];
		int nd = 0;
		long grand = 0;
		HashTable revenue;                                   // food ID -> money earned

		for (int i = 0; i < orders.size(); i++)
		{
			Order& o = orders.at(i);
			if (o.get_status() != COMPLETED) continue;
			int d = 0;                                       // linear search for this date
			while (d < nd && dates[d] != o.get_date()) d++;
			if (d == nd)
			{
				dates[nd] = o.get_date();
				cnt[nd] = 0;
				sub[nd] = 0;
				nd++;
			}
			cnt[d]++;
			sub[d] += o.subtotal();
			grand += o.subtotal();
			for (int j = 0; j < o.get_line_count(); j++)
				revenue.add(o.get_line(j).get_food_id(), o.get_line(j).total());
		}

		for (int i = 1; i < nd; i++)                         // insertion sort by date
		{
			int d = dates[i];
			long c = cnt[i], s = sub[i];
			int j = i - 1;
			while (j >= 0 && dates[j] > d)
			{
				dates[j + 1] = dates[j];
				cnt[j + 1] = cnt[j];
				sub[j + 1] = sub[j];
				j--;
			}
			dates[j + 1] = d;
			cnt[j + 1] = c;
			sub[j + 1] = s;
		}

		print_left("Date", 14);
		print_right("Orders", 8);
		print_right("Sales (Rs)", 14);
		print_right("With GST", 12);
		cout << "\n";
		print_line(48);
		ofstream csv(SALES_FILE);
		if (csv) csv << nd << "\n";
		for (int i = 0; i < nd; i++)
		{
			print_date_left(dates[i], 14);
			print_right_number(cnt[i], 8);
			print_right_number(sub[i], 14);
			print_right_money(sub[i] * 100 + sub[i] * GST_PERCENT, 12);
			cout << "\n";
			if (csv) csv << dates[i] << " " << cnt[i] << " " << sub[i] << "\n";
		}
		csv.close();
		if (nd == 0) cout << "(no completed orders yet)\n";
		print_line(48);
		cout << "Total sales before GST: Rs " << grand << "\n";

		Pair rows[TABLE_SIZE];
		int n = revenue.to_pairs(rows);
		sort_pairs(rows, n);                                 // merge sort: best seller first
		cout << "\nFoods ranked by revenue:\n";
		for (int i = 0; i < n; i++)
		{
			cout << "  " << i + 1 << ". ";
			menu.print_name(rows[i].key, 24);
			cout << "Rs " << rows[i].value << "\n";
		}
		if (nd > 0) cout << "(daily figures also saved to " << SALES_FILE << ")\n";
	}
};

// ============================================================
//  StockReport : value of the stock on the shelf (in hundredths of a rupee)
// ============================================================
long stock_value(long qty, Ingredient& g)
{
	// cost is Rs per kg / litre (1000 g / ml) or Rs per piece
	if (same_text(g.get_unit(), "piece")) return qty * g.get_cost() * 100;
	return qty * g.get_cost() / 10;
}

class StockReport : public Report
{
private:
	Inventory& inv;
public:
	StockReport(Inventory& i) : inv(i) {}

	const char* title() { return "STOCK REPORT (with valuation)"; }

	void generate()
	{
		long total = 0, expired_value = 0;
		print_left("Ingredient", 20);
		print_right("On hand", 12);
		print_right("Value (Rs)", 14);
		cout << "\n";
		print_line(46);
		for (int i = 0; i < inv.ingredient_count(); i++)
		{
			Ingredient& g = inv.ingredient_at(i);
			long all = inv.total_stock(g.get_id());
			long usable = inv.usable_stock(g.get_id());
			total += stock_value(all, g);
			expired_value += stock_value(all - usable, g);
			print_left(g.get_name(), 20);
			print_right_number(all, 12);
			print_right_money(stock_value(all, g), 14);
			cout << "\n";
		}
		print_line(46);
		cout << "Total stock value      : Rs ";
		print_right_money(total, 0);
		cout << "\nValue of expired stock : Rs ";
		print_right_money(expired_value, 0);
		cout << "\n";
	}
};

// ============================================================
//  ExpiryReport, LowStockReport, WastageReport
// ============================================================
class ExpiryReport : public Report
{
private:
	Inventory& inv;
	int days;
public:
	ExpiryReport(Inventory& i, int d) : inv(i), days(d) {}

	const char* title() { return "EXPIRY REPORT (expired or expiring soon)"; }

	void generate()
	{
		int n = inv.check_expiry(days);
		cout << "Batches needing attention: " << n << "\n";
	}
};

class LowStockReport : public Report
{
private:
	Inventory& inv;
public:
	LowStockReport(Inventory& i) : inv(i) {}

	const char* title() { return "LOW-STOCK REPORT"; }

	void generate()
	{
		int n = inv.check_low_stock();
		if (n > 0)
		{
			cout << "\nSuggested purchases to restore the minimum levels:\n";
			Pair none[1];
			inv.suggest_replenishment(none, 0);              // no demand: only the minimum level counts
		}
	}
};

class WastageReport : public Report
{
private:
	Inventory& inv;
public:
	WastageReport(Inventory& i) : inv(i) {}

	const char* title() { return "WASTAGE REPORT"; }

	void generate()
	{
		long waste[MAX_INGREDIENTS];
		for (int i = 0; i < MAX_INGREDIENTS; i++) waste[i] = 0;
		for (int t = 0; t < inv.txn_total(); t++)            // read all transactions (linear)
		{
			Transaction& x = inv.txn_at(t);
			int g = inv.find_ingredient(x.get_ing_id());
			if (g == -1) continue;
			if (same_text(x.get_type(), "WASTE")) waste[g] += x.get_qty();
			else if (same_text(x.get_type(), "UNDO_WASTE")) waste[g] -= x.get_qty();
		}
		long lost = 0;
		int rows = 0;
		for (int i = 0; i < inv.ingredient_count(); i++)
		{
			if (waste[i] <= 0) continue;
			Ingredient& g = inv.ingredient_at(i);
			cout << "  ";
			print_left(g.get_name(), 20);
			print_right_number(waste[i], 10);
			cout << " " << g.get_unit() << "    Rs ";
			print_right_money(stock_value(waste[i], g), 0);
			cout << "\n";
			lost += stock_value(waste[i], g);
			rows++;
		}
		if (rows == 0) cout << "  No wastage recorded.\n";
		cout << "Total value wasted: Rs ";
		print_right_money(lost, 0);
		cout << "\n";
	}
};

// ============================================================
//  DemandReport : a Report about ingredient demand
// ============================================================
class DemandReport : public Report
{
private:
	OrderBook& orders;
	RecipeBook& recipes;
	Inventory& inv;
	DemandCalculator calc;
public:
	DemandReport(OrderBook& o, RecipeBook& r, Inventory& i) : orders(o), recipes(r), inv(i) {}

	const char* title() { return "INGREDIENT DEMAND (CONFIRMED orders)"; }

	void generate()
	{
		HashTable table;
		int missing = calc.add_confirmed_orders(orders, recipes, table);
		Pair rows[TABLE_SIZE];
		int n = table.to_pairs(rows);
		sort_pairs(rows, n);                              // biggest demand first
		print_left("Ingredient", 24);
		cout << "Quantity needed\n";
		print_line(50);
		for (int i = 0; i < n; i++)
		{
			inv.print_name(rows[i].key, 24);
			cout << rows[i].value << " " << inv.unit_of(rows[i].key) << "\n";
		}
		if (n == 0) cout << "(no demand)\n";
		if (missing > 0) cout << "Note: " << missing << " ordered food line(s) have no recipe and were not counted.\n";
		cout << "(Demand is only a calculation. Stock has NOT been changed.)\n";
	}

	// Compare the total demand of two date ranges, ingredient by ingredient.
	void compare_demand(int from1, int to1, int from2, int to2)
	{
		HashTable a, b;
		calc.add_orders_in_range(orders, recipes, from1, to1, a);
		calc.add_orders_in_range(orders, recipes, from2, to2, b);

		Pair pa[TABLE_SIZE], pb[TABLE_SIZE], all[2 * TABLE_SIZE];
		int na = a.to_pairs(pa), nb = b.to_pairs(pb), n = 0;
		for (int i = 0; i < na; i++)                      // all keys of period 1 ...
		{
			all[n] = pa[i];
			n++;
		}
		for (int i = 0; i < nb; i++)                      // ... plus the keys only in period 2
		{
			if (!a.has(pb[i].key))
			{
				all[n] = pb[i];
				n++;
			}
		}
		sort_pairs(all, n);

		cout << "\nPeriod 1: ";
		print_date(from1);
		cout << " to ";
		print_date(to1);
		cout << "\nPeriod 2: ";
		print_date(from2);
		cout << " to ";
		print_date(to2);
		cout << "\n";
		print_left("Ingredient", 20);
		print_right("Period 1", 12);
		print_right("Period 2", 12);
		print_right("Change", 12);
		cout << "\n";
		print_line(58);
		for (int i = 0; i < n; i++)
		{
			int key = all[i].key;
			long v1 = a.get(key), v2 = b.get(key);
			inv.print_name(key, 20);
			print_right_number(v1, 12);
			print_right_number(v2, 12);
			if (v2 - v1 > 0) cout << "          +";
			else cout << "           ";
			cout << v2 - v1 << "  " << inv.unit_of(key) << "\n";
		}
		if (n == 0) cout << "(no orders in either period)\n";
	}

	// Simple forecast: average demand per day over a past period.
	void daily_average(int from, int to)
	{
		long days = days_between(from, to) + 1;
		if (days <= 0)
		{
			cout << "The end date must not be before the start date.\n";
			return;
		}
		HashTable table;
		calc.add_orders_in_range(orders, recipes, from, to, table);
		Pair rows[TABLE_SIZE];
		int n = table.to_pairs(rows);
		sort_pairs(rows, n);
		cout << "\nAverage daily demand from ";
		print_date(from);
		cout << " to ";
		print_date(to);
		cout << " (" << days << " day(s)):\n";
		for (int i = 0; i < n; i++)
		{
			cout << "  ";
			inv.print_name(rows[i].key, 22);
			cout << rows[i].value / days << " " << inv.unit_of(rows[i].key) << " per day\n";
		}
		if (n == 0) cout << "  (no orders in this period)\n";
		cout << "Simple estimate = total demand / number of days. Expect rough numbers only.\n";
	}
};

// ============================================================
//  PopularFoodsReport : the K foods that were ordered most
// ============================================================
class PopularFoodsReport : public Report
{
private:
	OrderBook& orders;
	FoodMenu& menu;
	int k;
public:
	PopularFoodsReport(OrderBook& o, FoodMenu& m, int top_k) : orders(o), menu(m), k(top_k) {}

	const char* title() { return "POPULAR FOODS (servings ordered, cancelled orders excluded)"; }

	void generate()
	{
		HashTable table;                                  // food ID -> servings ordered
		for (int i = 0; i < orders.size(); i++)
		{
			Order& o = orders.at(i);
			if (o.get_status() == CANCELLED) continue;
			for (int j = 0; j < o.get_line_count(); j++)
				table.add(o.get_line(j).get_food_id(), o.get_line(j).get_qty());
		}
		Pair rows[TABLE_SIZE];
		int n = table.to_pairs(rows);
		sort_pairs(rows, n);                              // merge sort, then take the first K (Top-K)
		int shown = (k < n) ? k : n;
		for (int i = 0; i < shown; i++)
		{
			cout << "  " << i + 1 << ". ";
			menu.print_name(rows[i].key, 24);
			cout << rows[i].value << " servings\n";
		}
		if (n == 0) cout << "  (no orders yet)\n";
	}
};

#endif
