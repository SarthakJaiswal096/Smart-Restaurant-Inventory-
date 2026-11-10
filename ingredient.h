// ingredient.h - INGREDIENTS AND BATCHES
//
// OOP concepts : inheritance   (Ingredient is an Item)
//                encapsulation, constructors, validation
// Rule         : stock is kept in BATCHES (one delivery = one batch with its own expiry date).

#ifndef INGREDIENT_H
#define INGREDIENT_H

#include "common.h"
#include "menu.h"

bool is_valid_unit(const char u[])
{
	return same_text(u, "g") || same_text(u, "ml") || same_text(u, "piece");
}

// ============================================================
//  Ingredient : inherits id and name from Item
// ============================================================
class Ingredient : public Item
{
private:
	char unit[8];            // "g", "ml" or "piece"
	long min_stock;          // warn when usable stock is below this
	int cost;                // Rs per kg / litre / piece
public:
	Ingredient() : Item()
	{
		copy_text(unit, "g", 8);
		min_stock = 0;
		cost = 0;
	}
	Ingredient(int i, const char n[], const char u[], long m, int c) : Item(i, n)
	{
		copy_text(unit, u, 8);
		min_stock = m;
		cost = c;
	}
	const char* get_unit() { return unit; }
	long get_min_stock() { return min_stock; }
	int get_cost() { return cost; }
	void set_min_stock(long m) { min_stock = m; }
	void set_cost(int c) { cost = c; }
};

// ============================================================
//  Batch : one delivery of one ingredient, with its own expiry date
// ============================================================
class Batch
{
private:
	int id;
	int ing_id;
	long qty;
	int expiry;              // YYYYMMDD
public:
	Batch()
	{
		id = 0;
		ing_id = 0;
		qty = 0;
		expiry = 0;
	}
	Batch(int i, int g, long q, int e)
	{
		id = i;
		ing_id = g;
		qty = q;
		expiry = e;
	}
	int get_id() { return id; }
	int get_ing_id() { return ing_id; }
	long get_qty() { return qty; }
	int get_expiry() { return expiry; }
	void set_qty(long q) { qty = q; }
	bool is_expired() { return expiry < today; }
};

#endif
