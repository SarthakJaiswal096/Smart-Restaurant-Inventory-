// inventory_manager.h - INVENTORY MANAGER
//
// OOP concepts : composition   (Inventory HAS ingredients, batches, transactions,
//                               a hash table and an undo stack)
//                encapsulation, constructors, validation
// DSA concepts : min-heap, hash table, stack (see stock_structures.h)
//                insertion sort (batches by expiry date, shortage by ID), linear search
//
// Rules: FEFO = "First Expiring, First Out".   Expired batches are never used.
//        Stock never goes below zero. If there is not enough stock NOTHING changes.
//        Stock changes only through: receive, use, wastage, undo.

#ifndef INVENTORY_MANAGER_H
#define INVENTORY_MANAGER_H

#include "common.h"
#include "menu.h"
#include "ingredient.h"
#include "transaction.h"
#include "stock_structures.h"

// ============================================================
//  Inventory : ingredients + batches + transactions
// ============================================================
class Inventory
{
private:
	Ingredient ings[MAX_INGREDIENTS];
	int ing_count;
	Batch batches[MAX_BATCHES];
	int batch_count;
	Transaction txns[MAX_TXNS];
	int txn_count;
	IdTable id_table;
	UndoStack undo;
	int next_batch_id;
	int next_txn_id;
	int next_group;

	void rebuild_table()
	{
		id_table.clear();
		for (int i = 0; i < ing_count; i++) id_table.insert(ings[i].get_id(), i);
	}

	int find_batch(int batch_id)
	{
		for (int i = 0; i < batch_count; i++)
			if (batches[i].get_id() == batch_id) return i;
		return -1;
	}

	void add_txn(int date, const char type[], int ing_id, int batch_id, long qty, const char note[])
	{
		if (txn_count >= MAX_TXNS)                             // full: drop the oldest record
		{
			for (int i = 1; i < MAX_TXNS; i++) txns[i - 1] = txns[i];
			txn_count--;
		}
		txns[txn_count] = Transaction(next_txn_id, date, type, ing_id, batch_id, qty, note);
		txn_count++;
		next_txn_id++;
	}

	// removes batches that are empty (only used when the batch array is full)
	void purge_empty_batches()
	{
		int w = 0;
		for (int i = 0; i < batch_count; i++)
			if (batches[i].get_qty() > 0)
			{
				batches[w] = batches[i];
				w++;
			}
		batch_count = w;
		undo.clear();                                          // undo entries may point to removed batches
	}

	// how much of ingredient 'id' is needed according to a Pair list
	long needed(Pair list[], int n, int id)
	{
		for (int i = 0; i < n; i++)
			if (list[i].key == id) return list[i].value;
		return 0;
	}

	const char* undo_type_text(char t)
	{
		if (t == 'R') return "UNDO_RECEIVE";
		if (t == 'U') return "UNDO_USE";
		return "UNDO_WASTE";
	}
public:
	Inventory()
	{
		ing_count = 0;
		batch_count = 0;
		txn_count = 0;
		next_batch_id = 101;
		next_txn_id = 1;
		next_group = 1;
	}

	void clear()
	{
		ing_count = 0;
		batch_count = 0;
		txn_count = 0;
		next_batch_id = 101;
		next_txn_id = 1;
		next_group = 1;
		id_table.clear();
		undo.clear();
	}

	// ---------- access (used by file saving and reports) ----------
	int ingredient_count() { return ing_count; }
	Ingredient& ingredient_at(int i) { return ings[i]; }
	int batch_total() { return batch_count; }
	Batch& batch_at(int i) { return batches[i]; }
	int txn_total() { return txn_count; }
	Transaction& txn_at(int i) { return txns[i]; }

	// ---------- loading helpers (no transaction is created) ----------
	bool add_batch_raw(int id, int ing_id, long qty, int expiry)
	{
		if (batch_count >= MAX_BATCHES || qty < 0 || find_ingredient(ing_id) == -1) return false;
		batches[batch_count] = Batch(id, ing_id, qty, expiry);
		batch_count++;
		if (id >= next_batch_id) next_batch_id = id + 1;
		return true;
	}

	bool add_txn_raw(int id, int date, const char type[], int ing_id, int batch_id, long qty, const char note[])
	{
		if (txn_count >= MAX_TXNS) return false;
		txns[txn_count] = Transaction(id, date, type, ing_id, batch_id, qty, note);
		txn_count++;
		if (id >= next_txn_id) next_txn_id = id + 1;
		return true;
	}

	// ---------- ingredients ----------
	int find_ingredient(int id) { return id_table.find(id); }        // hash table lookup

	int find_by_name(const char part[], int start)                   // linear search
	{
		for (int i = start; i < ing_count; i++)
			if (contains_text(ings[i].get_name(), part)) return i;
		return -1;
	}

	int next_ingredient_id()
	{
		int biggest = 0;
		for (int i = 0; i < ing_count; i++)
			if (ings[i].get_id() > biggest) biggest = ings[i].get_id();
		return biggest + 1;
	}

	void print_name(int id, int width)
	{
		int i = find_ingredient(id);
		if (i == -1)
		{
			cout << "Ingredient#" << id;
			print_spaces(width - 11);
		}
		else print_left(ings[i].get_name(), width);
	}

	const char* unit_of(int id)
	{
		int i = find_ingredient(id);
		if (i == -1) return "g";
		return ings[i].get_unit();
	}

	bool add_ingredient(int id, const char name[], const char unit[], long min_stock, int cost)
	{
		if (ing_count >= MAX_INGREDIENTS || id <= 0 || name[0] == '\0' || !is_valid_unit(unit)) return false;
		if (min_stock < 0 || cost < 0 || find_ingredient(id) != -1) return false;
		ings[ing_count] = Ingredient(id, name, unit, min_stock, cost);
		id_table.insert(id, ing_count);
		ing_count++;
		return true;
	}

	bool update_ingredient(int id, const char name[], long min_stock, int cost)
	{
		int i = find_ingredient(id);
		if (i == -1 || name[0] == '\0' || min_stock < 0 || cost < 0) return false;
		ings[i].set_name(name);
		ings[i].set_min_stock(min_stock);
		ings[i].set_cost(cost);
		return true;
	}

	// An ingredient that still has stock cannot be deleted.
	bool delete_ingredient(int id)
	{
		int i = find_ingredient(id);
		if (i == -1 || total_stock(id) > 0) return false;
		for (int j = i; j < ing_count - 1; j++)
			ings[j] = ings[j + 1];
		ing_count--;
		int w = 0;                                             // also remove its empty batches
		for (int b = 0; b < batch_count; b++)
			if (batches[b].get_ing_id() != id)
			{
				batches[w] = batches[b];
				w++;
			}
		batch_count = w;
		undo.clear();
		rebuild_table();
		return true;
	}

	// ---------- stock amounts ----------
	long total_stock(int ing_id)             // everything on the shelf, even expired
	{
		long sum = 0;
		for (int i = 0; i < batch_count; i++)
			if (batches[i].get_ing_id() == ing_id) sum += batches[i].get_qty();
		return sum;
	}

	long usable_stock(int ing_id)            // only batches that are NOT expired
	{
		long sum = 0;
		for (int i = 0; i < batch_count; i++)
			if (batches[i].get_ing_id() == ing_id && !batches[i].is_expired()) sum += batches[i].get_qty();
		return sum;
	}

	// ---------- receive new stock ----------
	// Returns the new batch ID, or 0 if it failed.
	int receive_stock(int ing_id, long qty, int expiry, int date)
	{
		if (find_ingredient(ing_id) == -1 || qty <= 0 || !valid_date(expiry)) return 0;
		if (expiry < today) return 0;                          // cannot receive expired stock
		if (batch_count >= MAX_BATCHES) purge_empty_batches();
		if (batch_count >= MAX_BATCHES) return 0;
		int id = next_batch_id;
		next_batch_id++;
		batches[batch_count] = Batch(id, ing_id, qty, expiry);
		batch_count++;
		add_txn(date, "RECEIVE", ing_id, id, qty, "stock_received");
		UndoEntry e = { 'R', id, ing_id, qty, next_group };
		next_group++;
		undo.push(e);
		return id;
	}

	// ---------- use stock : FEFO with the min-heap ----------
	// ALL OR NOTHING: if usable stock is smaller than qty, nothing changes.
	// can_undo is false when an order uses the stock (so undo cannot break the order status).
	bool use_stock(int ing_id, long qty, int date, const char note[], bool can_undo)
	{
		if (find_ingredient(ing_id) == -1 || qty <= 0) return false;
		if (usable_stock(ing_id) < qty) return false;

		MinHeap heap;                                          // put usable batches in the heap
		for (int i = 0; i < batch_count; i++)
		{
			if (batches[i].get_ing_id() == ing_id && batches[i].get_qty() > 0 && !batches[i].is_expired())
			{
				HeapNode node = { batches[i].get_expiry(), batches[i].get_id(), i };
				heap.insert(node);
			}
		}

		long remaining = qty;
		int group = next_group;
		next_group++;
		while (remaining > 0 && !heap.is_empty())
		{
			HeapNode node = heap.extract_min();                // the batch that expires first
			long have = batches[node.index].get_qty();
			long take = (have < remaining) ? have : remaining;
			batches[node.index].set_qty(have - take);
			remaining = remaining - take;
			add_txn(date, "USE", ing_id, node.batch_id, take, note);
			if (can_undo)
			{
				UndoEntry e = { 'U', node.batch_id, ing_id, take, group };
				undo.push(e);
			}
		}
		return true;
	}

	// ---------- wastage ----------
	bool record_wastage(int batch_id, long qty, const char reason[], int date)
	{
		int b = find_batch(batch_id);
		if (b == -1 || qty <= 0 || qty > batches[b].get_qty()) return false;
		batches[b].set_qty(batches[b].get_qty() - qty);
		int ing = batches[b].get_ing_id();
		add_txn(date, "WASTE", ing, batch_id, qty, reason);
		UndoEntry e = { 'W', batch_id, ing, qty, next_group };
		next_group++;
		undo.push(e);
		return true;
	}

	// Writes off every expired batch that still has stock. Returns how many.
	int waste_expired(int date)
	{
		int group = next_group, n = 0;
		next_group++;
		for (int i = 0; i < batch_count; i++)
		{
			if (batches[i].is_expired() && batches[i].get_qty() > 0)
			{
				long q = batches[i].get_qty();
				batches[i].set_qty(0);
				add_txn(date, "WASTE", batches[i].get_ing_id(), batches[i].get_id(), q, "expired");
				UndoEntry e = { 'W', batches[i].get_id(), batches[i].get_ing_id(), q, group };
				undo.push(e);
				n++;
			}
		}
		return n;
	}

	// ---------- undo (uses the stack) ----------
	// Reverses the newest action = every entry with the same group number.
	// Returns how many records were undone.
	int undo_last(int date)
	{
		if (undo.is_empty()) return 0;
		int group = undo.peek().group, done = 0;
		while (!undo.is_empty() && undo.peek().group == group)
		{
			UndoEntry e = undo.pop();
			int b = find_batch(e.batch_id);
			if (b == -1) continue;
			if (e.type == 'R')                                 // undo a receive: take the stock out again
			{
				long q = batches[b].get_qty() - e.qty;
				batches[b].set_qty(q < 0 ? 0 : q);
			}
			else                                               // undo a use or wastage: put the stock back
			{
				batches[b].set_qty(batches[b].get_qty() + e.qty);
			}
			add_txn(date, undo_type_text(e.type), e.ing_id, e.batch_id, e.qty, "undo");
			done++;
		}
		return done;
	}

	// ---------- displays ----------
	void show_stock()
	{
		cout << "\n";
		print_left("ID", 5);
		print_left("Ingredient", 20);
		print_right("Usable", 10);
		print_right("Expired", 10);
		print_right("Minimum", 10);
		cout << "  Unit   Alert\n";
		print_line(76);
		for (int i = 0; i < ing_count; i++)
		{
			int id = ings[i].get_id();
			long usable = usable_stock(id);
			long expired = total_stock(id) - usable;
			print_left_number(id, 5);
			print_left(ings[i].get_name(), 20);
			print_right_number(usable, 10);
			print_right_number(expired, 10);
			print_right_number(ings[i].get_min_stock(), 10);
			cout << "  ";
			print_left(ings[i].get_unit(), 7);
			if (usable < ings[i].get_min_stock()) cout << "LOW STOCK ";
			if (expired > 0) cout << "(has expired stock)";
			cout << "\n";
		}
		if (ing_count == 0) cout << "(no ingredients)\n";
	}

	// Insertion sort of batch positions by expiry date (earliest first).
	int sort_batches(int order[], int ing_id, int days_limit)
	{
		int n = 0;
		int limit = (days_limit >= 0) ? add_days(today, days_limit) : 0;
		for (int i = 0; i < batch_count; i++)
		{
			if (batches[i].get_qty() <= 0) continue;
			if (ing_id != 0 && batches[i].get_ing_id() != ing_id) continue;
			if (days_limit >= 0 && batches[i].get_expiry() > limit) continue;
			order[n] = i;
			n++;
		}
		for (int i = 1; i < n; i++)
		{
			int key = order[i];
			int j = i - 1;
			while (j >= 0 && batches[order[j]].get_expiry() > batches[key].get_expiry())
			{
				order[j + 1] = order[j];
				j--;
			}
			order[j + 1] = key;
		}
		return n;
	}

	// ing_id 0 = all ingredients
	void show_batches(int ing_id)
	{
		int order[MAX_BATCHES];
		int n = sort_batches(order, ing_id, -1);
		cout << "\n";
		print_left("Batch", 7);
		print_left("Ingredient", 20);
		print_right("Quantity", 10);
		cout << "  ";
		print_left("Unit", 7);
		print_left("Expiry", 12);
		cout << "Status\n";
		print_line(76);
		for (int i = 0; i < n; i++)
		{
			Batch& b = batches[order[i]];
			long left = days_between(today, b.get_expiry());
			print_left_number(b.get_id(), 7);
			print_name(b.get_ing_id(), 20);
			print_right_number(b.get_qty(), 10);
			cout << "  ";
			print_left(unit_of(b.get_ing_id()), 7);
			print_date_left(b.get_expiry(), 12);
			if (left < 0) cout << "EXPIRED";
			else if (left == 0) cout << "expires TODAY";
			else if (left <= ALERT_DAYS) cout << "expires in " << left << " day(s)";
			else cout << "OK";
			cout << "\n";
		}
		if (n == 0) cout << "(no batches with stock)\n";
	}

	// Prints ingredients whose usable stock is below the minimum. Returns how many.
	int check_low_stock()
	{
		int n = 0;
		for (int i = 0; i < ing_count; i++)
		{
			long usable = usable_stock(ings[i].get_id());
			if (usable < ings[i].get_min_stock())
			{
				cout << "  LOW: ";
				print_left(ings[i].get_name(), 20);
				cout << "usable " << usable << " " << ings[i].get_unit()
				     << ", minimum " << ings[i].get_min_stock() << "\n";
				n++;
			}
		}
		if (n == 0) cout << "  No low-stock ingredients.\n";
		return n;
	}

	// Prints batches that are expired or expire within 'days' days. Returns how many.
	int check_expiry(int days)
	{
		int order[MAX_BATCHES];
		int n = sort_batches(order, 0, days);
		for (int i = 0; i < n; i++)
		{
			Batch& b = batches[order[i]];
			long left = days_between(today, b.get_expiry());
			cout << "  " << (left < 0 ? "EXPIRED  " : "EXPIRING ") << "batch " << b.get_id() << ": ";
			print_name(b.get_ing_id(), 18);
			cout << b.get_qty() << " " << unit_of(b.get_ing_id()) << ", expiry ";
			print_date(b.get_expiry());
			if (left == 0) cout << " (TODAY)";
			else if (left > 0) cout << " (in " << left << " day(s))";
			cout << "\n";
		}
		if (n == 0) cout << "  No expired or soon-to-expire batches.\n";
		return n;
	}

	// ---------- demand compared with stock ----------
	// 'demand' is a list of (ingredient ID, quantity needed) pairs.

	bool has_enough(Pair demand[], int n)
	{
		for (int i = 0; i < n; i++)
			if (usable_stock(demand[i].key) < demand[i].value) return false;
		return true;
	}

	// Prints needed / usable / shortage (sorted by ingredient ID). Does NOT change stock.
	int show_shortage(Pair demand[], int n)
	{
		for (int i = 1; i < n; i++)                            // insertion sort by ingredient ID
		{
			Pair key = demand[i];
			int j = i - 1;
			while (j >= 0 && demand[j].key > key.key)
			{
				demand[j + 1] = demand[j];
				j--;
			}
			demand[j + 1] = key;
		}
		int short_count = 0;
		cout << "\n";
		print_left("Ingredient", 20);
		print_right("Needed", 10);
		print_right("Usable", 10);
		print_right("Shortage", 10);
		cout << "  Status\n";
		print_line(62);
		for (int i = 0; i < n; i++)
		{
			long usable = usable_stock(demand[i].key);
			long shortage = demand[i].value > usable ? demand[i].value - usable : 0;
			if (shortage > 0) short_count++;
			print_name(demand[i].key, 20);
			print_right_number(demand[i].value, 10);
			print_right_number(usable, 10);
			print_right_number(shortage, 10);
			cout << "  " << (shortage > 0 ? "SHORT" : "ok") << "  [" << unit_of(demand[i].key) << "]\n";
		}
		if (n == 0) cout << "(no demand to check)\n";
		return short_count;
	}

	// How much to buy:  demand + minimum level - usable stock   (only if above 0)
	int suggest_replenishment(Pair demand[], int n)
	{
		int shown = 0;
		for (int i = 0; i < ing_count; i++)
		{
			int id = ings[i].get_id();
			long target = needed(demand, n, id) + ings[i].get_min_stock();
			long usable = usable_stock(id);
			if (usable < target)
			{
				cout << "  Buy " << target - usable << " " << ings[i].get_unit() << " of " << ings[i].get_name() << "\n";
				shown++;
			}
		}
		if (shown == 0) cout << "  No replenishment needed.\n";
		return shown;
	}

	// last_n = 0 shows everything
	void show_transactions(int last_n)
	{
		int start = 0;
		if (last_n > 0 && txn_count > last_n) start = txn_count - last_n;
		cout << "\n";
		print_left("ID", 6);
		print_left("Date", 12);
		print_left("Type", 14);
		print_left("Ingredient", 18);
		print_left("Batch", 7);
		print_right("Qty", 9);
		cout << "  Note\n";
		print_line(80);
		for (int i = start; i < txn_count; i++)
		{
			Transaction& t = txns[i];
			print_left_number(t.get_id(), 6);
			print_date_left(t.get_date(), 12);
			print_left(t.get_type(), 14);
			print_name(t.get_ing_id(), 18);
			print_left_number(t.get_batch_id(), 7);
			print_right_number(t.get_qty(), 9);
			cout << "  " << t.get_note() << "\n";
		}
		if (txn_count == 0) cout << "(no transactions)\n";
	}
};

#endif
