// recipe.h - RECIPES
//
// OOP concepts : composition   (a Recipe HAS ingredient entries, RecipeBook HAS Recipes)
//                encapsulation, constructors
// DSA concepts : linear search (find a recipe)
// Rule         : recipe quantities are PER ONE SERVING.

#ifndef RECIPE_H
#define RECIPE_H

#include "common.h"
#include "menu.h"
#include "inventory_manager.h"

// ============================================================
//  Recipe : what ONE serving of a food needs
// ============================================================
class Recipe
{
private:
	int food_id;
	int count;
	int ing_ids[MAX_RECIPE_ITEMS];
	int qtys[MAX_RECIPE_ITEMS];
public:
	Recipe()
	{
		food_id = 0;
		count = 0;
	}
	Recipe(int f)
	{
		food_id = f;
		count = 0;
	}
	int get_food_id() { return food_id; }
	int get_count() { return count; }
	int get_ing_id(int i) { return ing_ids[i]; }
	int get_qty(int i) { return qtys[i]; }

	int find_item(int ing_id)
	{
		for (int i = 0; i < count; i++)
			if (ing_ids[i] == ing_id) return i;
		return -1;
	}

	// Adds an ingredient, or changes the quantity if it is already in the recipe.
	bool set_item(int ing_id, int qty)
	{
		if (qty <= 0) return false;
		int i = find_item(ing_id);
		if (i != -1)
		{
			qtys[i] = qty;
			return true;
		}
		if (count >= MAX_RECIPE_ITEMS) return false;
		ing_ids[count] = ing_id;
		qtys[count] = qty;
		count++;
		return true;
	}

	bool remove_item(int ing_id)
	{
		int i = find_item(ing_id);
		if (i == -1) return false;
		for (int j = i; j < count - 1; j++)
		{
			ing_ids[j] = ing_ids[j + 1];
			qtys[j] = qtys[j + 1];
		}
		count--;
		return true;
	}
};

// ============================================================
//  RecipeBook : all recipes (one recipe per food)
// ============================================================
class RecipeBook
{
private:
	Recipe recipes[MAX_RECIPES];
	int count;
public:
	RecipeBook() { count = 0; }
	int size() { return count; }
	Recipe& at(int i) { return recipes[i]; }
	void clear() { count = 0; }

	// linear search by food ID, returns the index or -1
	int find(int food_id)
	{
		for (int i = 0; i < count; i++)
			if (recipes[i].get_food_id() == food_id) return i;
		return -1;
	}

	// Creates an empty recipe and returns its index (-1 if it exists already or the book is full).
	int add_recipe(int food_id)
	{
		if (count >= MAX_RECIPES || find(food_id) != -1) return -1;
		recipes[count] = Recipe(food_id);
		count++;
		return count - 1;
	}

	bool remove_recipe(int food_id)
	{
		int i = find(food_id);
		if (i == -1) return false;
		for (int j = i; j < count - 1; j++)
			recipes[j] = recipes[j + 1];
		count--;
		return true;
	}

	void show_recipe(int i, FoodMenu& menu, Inventory& inv)
	{
		Recipe& r = recipes[i];
		cout << "Recipe for ";
		menu.print_name(r.get_food_id(), 0);
		cout << " (food #" << r.get_food_id() << ", per 1 serving):\n";
		for (int k = 0; k < r.get_count(); k++)
		{
			int g = r.get_ing_id(k);
			cout << "   ";
			inv.print_name(g, 20);
			cout << r.get_qty(k) << " " << inv.unit_of(g) << "\n";
		}
		if (r.get_count() == 0) cout << "   (no ingredients yet)\n";
	}

	void show_all(FoodMenu& menu, Inventory& inv)
	{
		for (int i = 0; i < count; i++) show_recipe(i, menu, inv);
		if (count == 0) cout << "(no recipes yet)\n";
	}
};

#endif
