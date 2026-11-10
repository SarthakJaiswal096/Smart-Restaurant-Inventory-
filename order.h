// order.h - ORDERS AND BILLING
//
// OOP concepts : composition   (an Order HAS OrderLines, OrderBook HAS a CircularQueue)
//                encapsulation, constructors
// DSA concepts : circular queue (pending orders, first come first served)
//                linear search  (find an order)
// Money is kept in hundredths, so GST is exact.

#ifndef ORDER_H
#define ORDER_H

#include "common.h"
#include "menu.h"

// ============================================================
//  OrderLine : one food + quantity inside an order.
//  The price is copied when the order is made, so old bills never change.
// ============================================================
class OrderLine
{
private:
	int food_id;
	int qty;
	int price;
public:
	OrderLine()
	{
		food_id = 0;
		qty = 0;
		price = 0;
	}
	OrderLine(int f, int q, int p)
	{
		food_id = f;
		qty = q;
		price = p;
	}
	int get_food_id() { return food_id; }
	int get_qty() { return qty; }
	int get_price() { return price; }
	void set_qty(int q) { qty = q; }
	long total() { return (long)qty * price; }
};

// ============================================================
//  Order : id, date, status and a list of OrderLines (composition)
// ============================================================
class Order
{
private:
	int id;
	int date;
	int status;
	OrderLine lines[MAX_LINES];
	int line_count;
public:
	Order()
	{
		id = 0;
		date = 0;
		status = PENDING;
		line_count = 0;
	}
	Order(int i, int d, int s)
	{
		id = i;
		date = d;
		status = s;
		line_count = 0;
	}

	int get_id() { return id; }
	int get_date() { return date; }
	int get_status() { return status; }
	int get_line_count() { return line_count; }
	OrderLine& get_line(int i) { return lines[i]; }
	void set_id(int i) { id = i; }
	void set_status(int s) { status = s; }

	int find_line(int food_id)
	{
		for (int i = 0; i < line_count; i++)
			if (lines[i].get_food_id() == food_id) return i;
		return -1;
	}

	// Adds a food. If it is already in the order, the quantities are added together.
	bool add_line(int food_id, int qty, int price)
	{
		if (qty <= 0) return false;
		int i = find_line(food_id);
		if (i != -1)
		{
			lines[i].set_qty(lines[i].get_qty() + qty);
			return true;
		}
		if (line_count >= MAX_LINES) return false;
		lines[line_count] = OrderLine(food_id, qty, price);
		line_count++;
		return true;
	}

	// qty 0 removes the food, but the last food of an order cannot be removed.
	bool set_line_qty(int food_id, int qty)
	{
		int i = find_line(food_id);
		if (i == -1 || qty < 0) return false;
		if (qty > 0)
		{
			lines[i].set_qty(qty);
			return true;
		}
		if (line_count <= 1) return false;
		for (int j = i; j < line_count - 1; j++)
			lines[j] = lines[j + 1];
		line_count--;
		return true;
	}

	bool can_modify() { return status == PENDING || status == CONFIRMED; }

	long subtotal()
	{
		long sum = 0;
		for (int i = 0; i < line_count; i++)
			sum = sum + lines[i].total();
		return sum;
	}
};

// ============================================================
//  CircularQueue : first in, first out (FIFO) queue of order IDs.
//  When 'rear' reaches the end of the array it goes back to 0 (using %).
// ============================================================
class CircularQueue
{
private:
	int data[MAX_ORDERS];
	int front;          // index of the first item
	int rear;           // index where the next item will be stored
	int count;
public:
	CircularQueue()
	{
		front = 0;
		rear = 0;
		count = 0;
	}
	bool is_empty() { return count == 0; }
	bool is_full() { return count == MAX_ORDERS; }
	int size() { return count; }
	void clear()
	{
		front = 0;
		rear = 0;
		count = 0;
	}

	bool enqueue(int value)
	{
		if (is_full()) return false;
		data[rear] = value;
		rear = (rear + 1) % MAX_ORDERS;
		count++;
		return true;
	}

	bool dequeue(int& value)
	{
		if (is_empty()) return false;
		value = data[front];
		front = (front + 1) % MAX_ORDERS;
		count--;
		return true;
	}

	int item_at(int i) { return data[(front + i) % MAX_ORDERS]; }     // i-th from the front
};

// ============================================================
//  OrderBook : all orders + the queue of pending orders
// ============================================================
class OrderBook
{
private:
	Order orders[MAX_ORDERS];
	int count;
	int next_id;
	CircularQueue pending;
public:
	OrderBook()
	{
		count = 0;
		next_id = 1001;
	}

	int size() { return count; }
	Order& at(int i) { return orders[i]; }
	int pending_count() { return pending.size(); }

	void clear()
	{
		count = 0;
		next_id = 1001;
		pending.clear();
	}

	// linear search by order ID
	int find_by_id(int id)
	{
		for (int i = 0; i < count; i++)
			if (orders[i].get_id() == id) return i;
		return -1;
	}

	// New order: give it an ID, make it PENDING and put it in the queue.
	int place_order(Order draft)
	{
		if (count >= MAX_ORDERS || draft.get_line_count() == 0) return 0;
		draft.set_id(next_id);
		draft.set_status(PENDING);
		orders[count] = draft;
		count++;
		pending.enqueue(next_id);
		next_id++;
		return next_id - 1;
	}

	// Used when loading from a file: keeps the saved ID and status.
	bool add_loaded_order(Order o)
	{
		if (count >= MAX_ORDERS || find_by_id(o.get_id()) != -1) return false;
		orders[count] = o;
		count++;
		if (o.get_id() >= next_id) next_id = o.get_id() + 1;
		return true;
	}

	// After loading: put every PENDING order back into the queue.
	void rebuild_queue()
	{
		pending.clear();
		for (int i = 0; i < count; i++)
			if (orders[i].get_status() == PENDING) pending.enqueue(orders[i].get_id());
	}

	// change the quantity of one food in an order (0 removes it)
	bool update_order(int order_id, int food_id, int qty)
	{
		int i = find_by_id(order_id);
		if (i == -1 || !orders[i].can_modify()) return false;
		return orders[i].set_line_qty(food_id, qty);
	}

	bool add_line_to_order(int order_id, int food_id, int qty, int price)
	{
		int i = find_by_id(order_id);
		if (i == -1 || !orders[i].can_modify()) return false;
		return orders[i].add_line(food_id, qty, price);
	}

	// A cancelled order stays in the queue and is skipped later.
	bool cancel_order(int id)
	{
		int i = find_by_id(id);
		if (i == -1 || !orders[i].can_modify()) return false;
		orders[i].set_status(CANCELLED);
		return true;
	}

	bool confirm_order(int id)
	{
		int i = find_by_id(id);
		if (i == -1 || orders[i].get_status() != PENDING) return false;
		orders[i].set_status(CONFIRMED);
		return true;
	}

	bool complete_order(int id)
	{
		int i = find_by_id(id);
		if (i == -1 || orders[i].get_status() != CONFIRMED) return false;
		orders[i].set_status(COMPLETED);
		return true;
	}

	// FIFO: take the oldest PENDING order out of the queue and CONFIRM it.
	// Orders that were cancelled while waiting are skipped.
	// Returns the order ID, or 0 if nothing is waiting.
	int process_next()
	{
		int id;
		while (pending.dequeue(id))
		{
			int i = find_by_id(id);
			if (i != -1 && orders[i].get_status() == PENDING)
			{
				orders[i].set_status(CONFIRMED);
				return id;
			}
		}
		return 0;
	}

	void show_order(int i, FoodMenu& menu)
	{
		Order& o = orders[i];
		cout << "Order #" << o.get_id() << " | ";
		print_date(o.get_date());
		cout << " | " << status_text(o.get_status()) << "\n";
		for (int k = 0; k < o.get_line_count(); k++)
		{
			OrderLine& l = o.get_line(k);
			cout << "    ";
			menu.print_name(l.get_food_id(), 24);
			cout << " x";
			print_left_number(l.get_qty(), 4);
			cout << " @ ";
			print_left_number(l.get_price(), 5);
			cout << " = " << l.total() << "\n";
		}
		cout << "    Subtotal: " << o.subtotal() << "\n";
	}

	// filter: -1 = all orders, otherwise one status
	void show_history(FoodMenu& menu, int filter)
	{
		int shown = 0;
		for (int i = 0; i < count; i++)
		{
			if (filter != -1 && orders[i].get_status() != filter) continue;
			show_order(i, menu);
			shown++;
		}
		if (shown == 0) cout << "(no orders to show)\n";
	}

	void show_queue()
	{
		cout << "Pending queue (front = served first):\n";
		int shown = 0;
		for (int i = 0; i < pending.size(); i++)
		{
			int id = pending.item_at(i);
			int k = find_by_id(id);
			if (k != -1 && orders[k].get_status() == PENDING)
			{
				shown++;
				cout << "  " << shown << ". Order #" << id << "  (";
				print_date(orders[k].get_date());
				cout << ", subtotal " << orders[k].subtotal() << ")\n";
			}
		}
		if (shown == 0) cout << "  (queue is empty)\n";
	}
};

// ============================================================
//  print_bill : subtotal + GST = total
//  Money is kept in hundredths, so GST 5% of 5800 = 290.00 exactly.
// ============================================================
void print_bill(Order& o, FoodMenu& menu)
{
	long sub = o.subtotal() * 100;
	long tax = o.subtotal() * GST_PERCENT;
	cout << "\n================== BILL ==================\n";
	cout << "Order #" << o.get_id() << "   Date: ";
	print_date(o.get_date());
	cout << "   (" << status_text(o.get_status()) << ")\n";
	print_line(42);
	print_left("Item", 20);
	print_right("Qty", 5);
	print_right("Price", 8);
	print_right("Total", 9);
	cout << "\n";
	for (int i = 0; i < o.get_line_count(); i++)
	{
		OrderLine& l = o.get_line(i);
		menu.print_name(l.get_food_id(), 20);
		print_right_number(l.get_qty(), 5);
		print_right_number(l.get_price(), 8);
		print_right_number(l.total(), 9);
		cout << "\n";
	}
	print_line(42);
	print_left("Subtotal", 33);
	print_right_money(sub, 9);
	cout << "\n";
	cout << "GST " << GST_PERCENT << "%";
	print_spaces(27);
	print_right_money(tax, 9);
	cout << "\n";
	print_left("TOTAL (Rs)", 33);
	print_right_money(sub + tax, 9);
	cout << "\n==========================================\n";
}

#endif
