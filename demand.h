// demand.h - INGREDIENT DEMAND
//
// DSA concepts : hash table (add up the quantity of each ingredient quickly)
//                merge sort (rank ingredients / foods from biggest to smallest)
//
// Rules: ingredient demand = recipe quantity x servings ordered.
//        only CONFIRMED orders count as active demand. Cancelled orders never count.
//        calculating demand NEVER changes the stock.

#ifndef DEMAND_H
#define DEMAND_H

#include "common.h"
#include "menu.h"
#include "order.h"
#include "recipe.h"
#include "inventory_manager.h"

// ============================================================
//  HashTable : key = ID, value = total quantity.
//  The hash function key % 211 tells us which slot to use.
//  If that slot holds another key (a "collision") we try the next slot
//  (LINEAR PROBING).
// ============================================================
const int TABLE_SIZE = 211;          // a prime number spreads the keys evenly

class HashTable
{
private:
	int keys[TABLE_SIZE];
	long values[TABLE_SIZE];
	bool used[TABLE_SIZE];
	int count;

	// slot where 'key' is stored, or -1 if it is not in the table
	int find_slot(int key)
	{
		int slot = key % TABLE_SIZE;
		for (int step = 0; step < TABLE_SIZE; step++)
		{
			if (!used[slot]) return -1;                 // empty slot: the key is not here
			if (keys[slot] == key) return slot;
			slot = (slot + 1) % TABLE_SIZE;             // collision: try the next slot
		}
		return -1;
	}
public:
	HashTable()
	{
		for (int i = 0; i < TABLE_SIZE; i++)
		{
			used[i] = false;
			keys[i] = 0;
			values[i] = 0;
		}
		count = 0;
	}

	bool has(int key) { return find_slot(key) != -1; }

	long get(int key)
	{
		int slot = find_slot(key);
		if (slot == -1) return 0;
		return values[slot];
	}

	// Adds 'amount' to the total of this key (creates the key if it is new).
	void add(int key, long amount)
	{
		int slot = find_slot(key);
		if (slot != -1)
		{
			values[slot] += amount;
			return;
		}
		if (count >= TABLE_SIZE - 1) return;            // table full
		slot = key % TABLE_SIZE;
		while (used[slot])
			slot = (slot + 1) % TABLE_SIZE;
		used[slot] = true;
		keys[slot] = key;
		values[slot] = amount;
		count++;
	}

	// Copies all (key, value) pairs into an array. Returns how many pairs.
	int to_pairs(Pair out[])
	{
		int n = 0;
		for (int i = 0; i < TABLE_SIZE; i++)
		{
			if (used[i])
			{
				out[n].key = keys[i];
				out[n].value = values[i];
				n++;
			}
		}
		return n;
	}
};

// ============================================================
//  MERGE SORT (biggest value first, same value -> smaller key first)
//  1. split the array in two halves   2. sort each half   3. merge them
// ============================================================
void merge_sort(Pair a[], Pair temp[], int low, int high)
{
	if (low >= high) return;                            // 0 or 1 item: already sorted
	int mid = (low + high) / 2;
	merge_sort(a, temp, low, mid);
	merge_sort(a, temp, mid + 1, high);

	int i = low, j = mid + 1, k = low;
	while (i <= mid && j <= high)                       // merge: take the bigger front item
	{
		if (a[i].value > a[j].value || (a[i].value == a[j].value && a[i].key <= a[j].key))
		{
			temp[k] = a[i];
			i++;
		}
		else
		{
			temp[k] = a[j];
			j++;
		}
		k++;
	}
	while (i <= mid)
	{
		temp[k] = a[i];
		i++;
		k++;
	}
	while (j <= high)
	{
		temp[k] = a[j];
		j++;
		k++;
	}
	for (k = low; k <= high; k++) a[k] = temp[k];
}

void sort_pairs(Pair a[], int n)
{
	Pair temp[2 * TABLE_SIZE];
	if (n > 1) merge_sort(a, temp, 0, n - 1);
}

// ============================================================
//  DemandCalculator : orders -> how much of each ingredient is needed
// ============================================================
class DemandCalculator
{
public:
	// Adds the demand of ONE order into the hash table.
	// Returns how many foods of the order have no recipe (they are skipped).
	int add_order(Order& o, RecipeBook& book, HashTable& table)
	{
		int missing = 0;
		for (int i = 0; i < o.get_line_count(); i++)
		{
			OrderLine& line = o.get_line(i);
			int r = book.find(line.get_food_id());
			if (r == -1)
			{
				missing++;
				continue;
			}
			Recipe& recipe = book.at(r);
			for (int k = 0; k < recipe.get_count(); k++)
				table.add(recipe.get_ing_id(k), (long)recipe.get_qty(k) * line.get_qty());   // per serving x servings
		}
		return missing;
	}

	// ACTIVE demand = CONFIRMED orders only.
	int add_confirmed_orders(OrderBook& orders, RecipeBook& book, HashTable& table)
	{
		int missing = 0;
		for (int i = 0; i < orders.size(); i++)
			if (orders.at(i).get_status() == CONFIRMED)
				missing += add_order(orders.at(i), book, table);
		return missing;
	}

	// Demand of all NOT cancelled orders dated from..to (for comparison and averages).
	int add_orders_in_range(OrderBook& orders, RecipeBook& book, int from, int to, HashTable& table)
	{
		int missing = 0;
		for (int i = 0; i < orders.size(); i++)
		{
			Order& o = orders.at(i);
			if (o.get_status() == CANCELLED) continue;
			if (o.get_date() < from || o.get_date() > to) continue;
			missing += add_order(o, book, table);
		}
		return missing;
	}
};

#endif
