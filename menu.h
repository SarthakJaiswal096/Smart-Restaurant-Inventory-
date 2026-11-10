// menu.h - FOOD MENU
//
// OOP concepts : inheritance   (Food is an Item; Ingredient also inherits from Item)
//                encapsulation (private data + public functions), constructors
// DSA concepts : binary search  (find a food by ID, the menu array is always sorted by ID)
//                linear search  (find a food by name)
//                insertion sort (menu sorted by name / price)

#ifndef MENU_H
#define MENU_H

#include "common.h"

// ============================================================
//  Item : base class (an ID and a name).
//  Food (here) and Ingredient (ingredient.h) both inherit from it.
// ============================================================
class Item
{
protected:                          // protected = child classes can use it
	int id;
	char name[NAME_LEN];
public:
	Item()
	{
		id = 0;
		name[0] = '\0';
	}
	Item(int i, const char n[])
	{
		id = i;
		copy_text(name, n, NAME_LEN);
	}
	int get_id() { return id; }
	const char* get_name() { return name; }
	void set_name(const char n[]) { copy_text(name, n, NAME_LEN); }
};

// ============================================================
//  Food : a dish on the menu
// ============================================================
class Food : public Item
{
private:
	int price;
	bool available;
public:
	Food() : Item()
	{
		price = 0;
		available = true;
	}
	Food(int i, const char n[], int p, bool a) : Item(i, n)
	{
		price = p;
		available = a;
	}
	int get_price() { return price; }
	bool is_available() { return available; }
	void set_price(int p) { price = p; }
	void set_available(bool a) { available = a; }
};

// ============================================================
//  FoodMenu : array of foods that is ALWAYS sorted by ID,
//  so we can use binary search for ID lookups.
// ============================================================
class FoodMenu
{
private:
	Food foods[MAX_FOODS];
	int count;

	// should food a be placed after food b ?   how: 1 = ID, 2 = name, 3 = price
	bool is_after(Food& a, Food& b, int how)
	{
		if (how == 2) return compare_text(a.get_name(), b.get_name()) > 0;
		if (how == 3) return a.get_price() > b.get_price();
		return a.get_id() > b.get_id();
	}
public:
	FoodMenu() { count = 0; }

	int size() { return count; }
	Food& at(int i) { return foods[i]; }
	void clear() { count = 0; }

	// BINARY SEARCH: look at the middle, throw away half, repeat.
	int find_by_id(int id)
	{
		int low = 0, high = count - 1;
		while (low <= high)
		{
			int mid = (low + high) / 2;
			if (foods[mid].get_id() == id) return mid;
			if (foods[mid].get_id() < id) low = mid + 1;
			else high = mid - 1;
		}
		return -1;
	}

	// LINEAR SEARCH by name (part of the name is enough). 'start' lets us find the next match.
	int find_by_name(const char part[], int start)
	{
		for (int i = start; i < count; i++)
			if (contains_text(foods[i].get_name(), part)) return i;
		return -1;
	}

	// Adds a food and keeps the array sorted by ID (shift bigger IDs right, drop it in the gap).
	bool add_food(int id, const char name[], int price, bool available)
	{
		if (count >= MAX_FOODS || id <= 0 || price < 0 || name[0] == '\0') return false;
		if (find_by_id(id) != -1) return false;
		int i = count - 1;
		while (i >= 0 && foods[i].get_id() > id)
		{
			foods[i + 1] = foods[i];
			i--;
		}
		foods[i + 1] = Food(id, name, price, available);
		count++;
		return true;
	}

	bool update_food(int id, const char name[], int price, bool available)
	{
		int i = find_by_id(id);
		if (i == -1 || price < 0 || name[0] == '\0') return false;
		foods[i].set_name(name);
		foods[i].set_price(price);
		foods[i].set_available(available);
		return true;
	}

	bool remove_food(int id)
	{
		int i = find_by_id(id);
		if (i == -1) return false;
		for (int j = i; j < count - 1; j++)
			foods[j] = foods[j + 1];
		count--;
		return true;
	}

	int next_food_id()
	{
		int biggest = 0;
		for (int i = 0; i < count; i++)
			if (foods[i].get_id() > biggest) biggest = foods[i].get_id();
		return biggest + 1;
	}

	// prints the name of a food (used by other modules)
	void print_name(int id, int width)
	{
		int i = find_by_id(id);
		if (i == -1)
		{
			cout << "Food#" << id;
			print_spaces(width - 5);
		}
		else print_left(foods[i].get_name(), width);
	}

	// how: 1 = by ID, 2 = by name, 3 = by price.
	// INSERTION SORT on a copy, so the real menu stays sorted by ID.
	void show_menu(int how)
	{
		Food copy[MAX_FOODS];
		for (int i = 0; i < count; i++) copy[i] = foods[i];

		for (int i = 1; i < count; i++)
		{
			Food key = copy[i];
			int j = i - 1;
			while (j >= 0 && is_after(copy[j], key, how))
			{
				copy[j + 1] = copy[j];
				j--;
			}
			copy[j + 1] = key;
		}

		cout << "\n";
		print_left("ID", 6);
		print_left("Food", 26);
		print_left("Price", 8);
		cout << "Status\n";
		print_line(50);
		for (int i = 0; i < count; i++)
		{
			print_left_number(copy[i].get_id(), 6);
			print_left(copy[i].get_name(), 26);
			print_left_number(copy[i].get_price(), 8);
			cout << (copy[i].is_available() ? "Available" : "Unavailable") << "\n";
		}
		if (count == 0) cout << "(menu is empty)\n";
	}
};

#endif
