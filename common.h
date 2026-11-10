// common.h - shared constants and OUR OWN helper functions
//
// Only <iostream> and <fstream> are used (cin, cout and files).
// Everything else (text handling, number <-> text, dates, printing in columns,
// reading input) is written by us below, so nothing is hidden inside a library.
//
// Agreed rules:
//   * IDs are whole numbers.   * Dates are whole numbers like 20261105 (YYYYMMDD).
//   * Quantities are whole numbers in g, ml or piece (1 kg = 1000 g).
//   * Names are ONE word (use _ instead of a space, example Chicken_Biryani).
//   * Order status: PENDING, CONFIRMED, CANCELLED, COMPLETED.

#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>
using namespace std;

// ---------- fixed sizes (plain arrays, no STL containers) ----------
const int NAME_LEN = 24;            // a name can have 23 letters (+ 1 end mark)
const int WORD_LEN = 48;            // for notes, actions, log lines
const int MAX_FOODS = 50;
const int MAX_ORDERS = 200;
const int MAX_LINES = 10;           // different foods in one order
const int MAX_RECIPES = 50;
const int MAX_RECIPE_ITEMS = 10;    // ingredients in one recipe
const int MAX_INGREDIENTS = 50;
const int MAX_BATCHES = 200;
const int MAX_TXNS = 1000;
const int MAX_USERS = 10;
const int GST_PERCENT = 5;
const int ALERT_DAYS = 7;           // "expiring soon" window

// ---------- data file names ----------
// ---------- file names ----------
const char* const FOOD_FILE = "data/food_data.txt";
const char* const ORDER_FILE = "data/order_data.txt";
const char* const RECIPE_FILE = "data/recipe_data.txt";
const char* const ING_FILE = "data/ingredient_data.txt";
const char* const BATCH_FILE = "data/batches_data.txt";
const char* const TXN_FILE = "data/transactions_data.txt";
const char* const USER_FILE = "data/users_data.txt";
const char* const AUDIT_FILE = "data/audit_log.txt";
const char* const SALES_FILE = "data/sales_report.txt";

const int FILE_COUNT = 7;
const char* const DATA_FILES[FILE_COUNT] = { FOOD_FILE, ORDER_FILE, RECIPE_FILE, ING_FILE, BATCH_FILE, TXN_FILE, USER_FILE };

// ---------- order status ----------
const int PENDING = 0;
const int CONFIRMED = 1;
const int CANCELLED = 2;
const int COMPLETED = 3;

// ---------- a (key, value) pair ----------
struct Pair
{
	int key;
	long value;
};

// ============================================================
//  TEXT FUNCTIONS  (text = array of char that ends with '\0')
// ============================================================
int text_length(const char t[])
{
	int n = 0;
	while (t[n] != '\0') n++;
	return n;
}

// copies 'from' into 'to' (at most max-1 letters)
void copy_text(char to[], const char from[], int max)
{
	int i = 0;
	while (from[i] != '\0' && i < max - 1)
	{
		to[i] = from[i];
		i++;
	}
	to[i] = '\0';
}

// adds 'add' to the end of 'target'
void append_text(char target[], const char add[], int max)
{
	int n = text_length(target);
	int i = 0;
	while (add[i] != '\0' && n < max - 1)
	{
		target[n] = add[i];
		n++;
		i++;
	}
	target[n] = '\0';
}

bool same_text(const char a[], const char b[])
{
	int i = 0;
	while (a[i] != '\0' && a[i] == b[i]) i++;
	return a[i] == b[i];
}

char lower_char(char c)
{
	if (c >= 'A' && c <= 'Z') return c + 32;
	return c;
}

char upper_char(char c)
{
	if (c >= 'a' && c <= 'z') return c - 32;
	return c;
}

void make_upper(char t[])
{
	for (int i = 0; t[i] != '\0'; i++) t[i] = upper_char(t[i]);
}

// < 0 if a comes first, 0 if equal, > 0 if b comes first (capital letters ignored)
int compare_text(const char a[], const char b[])
{
	int i = 0;
	while (a[i] != '\0' && lower_char(a[i]) == lower_char(b[i])) i++;
	return lower_char(a[i]) - lower_char(b[i]);
}

// true if 'part' is somewhere inside 'text' (capital letters ignored)
bool contains_text(const char text[], const char part[])
{
	int tl = text_length(text), pl = text_length(part);
	for (int start = 0; start + pl <= tl; start++)
	{
		int k = 0;
		while (k < pl && lower_char(text[start + k]) == lower_char(part[k])) k++;
		if (k == pl) return true;
	}
	return false;
}

// status number -> text
const char* status_text(int s)
{
	if (s == PENDING) return "PENDING";
	if (s == CONFIRMED) return "CONFIRMED";
	if (s == CANCELLED) return "CANCELLED";
	return "COMPLETED";
}

// text -> status number (-1 if the text is not a status)
int status_from_text(const char t[])
{
	if (same_text(t, "PENDING")) return PENDING;
	if (same_text(t, "CONFIRMED")) return CONFIRMED;
	if (same_text(t, "CANCELLED")) return CANCELLED;
	if (same_text(t, "COMPLETED")) return COMPLETED;
	return -1;
}

// ============================================================
//  NUMBER <-> TEXT
// ============================================================
void number_to_text(long n, char out[])
{
	char temp[24];
	int len = 0, k = 0;
	bool negative = false;
	if (n < 0)
	{
		negative = true;
		n = -n;
	}
	if (n == 0)
	{
		temp[len] = '0';
		len++;
	}
	while (n > 0)
	{
		temp[len] = '0' + n % 10;
		len++;
		n = n / 10;
	}
	if (negative)
	{
		out[k] = '-';
		k++;
	}
	while (len > 0)
	{
		len--;
		out[k] = temp[len];
		k++;
	}
	out[k] = '\0';
}

// text -> number. Returns false if the text is not a whole number.
bool text_to_number(const char t[], long& result)
{
	int i = 0;
	bool negative = false;
	if (t[0] == '-')
	{
		negative = true;
		i = 1;
	}
	if (t[i] == '\0' || text_length(t) > 12) return false;
	long value = 0;
	while (t[i] != '\0')
	{
		if (t[i] < '0' || t[i] > '9') return false;
		value = value * 10 + (t[i] - '0');
		i++;
	}
	if (negative) value = -value;
	result = value;
	return true;
}

// money is kept in hundredths (paise): 12345 -> "123.45"
void money_to_text(long hundredths, char out[])
{
	char whole[24];
	long a = hundredths;
	int k = 0, i = 0;
	if (a < 0) a = -a;
	number_to_text(a / 100, whole);
	if (hundredths < 0)
	{
		out[k] = '-';
		k++;
	}
	while (whole[i] != '\0')
	{
		out[k] = whole[i];
		k++;
		i++;
	}
	out[k] = '.';
	out[k + 1] = '0' + (a % 100) / 10;
	out[k + 2] = '0' + a % 10;
	out[k + 3] = '\0';
}

// ============================================================
//  PRINTING IN COLUMNS
// ============================================================
void print_spaces(int n)
{
	for (int i = 0; i < n; i++) cout << ' ';
}

void print_left(const char t[], int width)
{
	cout << t;
	print_spaces(width - text_length(t));
}

void print_right(const char t[], int width)
{
	print_spaces(width - text_length(t));
	cout << t;
}

void print_left_number(long n, int width)
{
	char t[24];
	number_to_text(n, t);
	print_left(t, width);
}

void print_right_number(long n, int width)
{
	char t[24];
	number_to_text(n, t);
	print_right(t, width);
}

void print_right_money(long hundredths, int width)
{
	char t[32];
	money_to_text(hundredths, t);
	print_right(t, width);
}

void print_line(int n)
{
	for (int i = 0; i < n; i++) cout << '-';
	cout << '\n';
}

// ============================================================
//  DATES  (YYYYMMDD whole numbers)
// ============================================================
int today = 20261105;               // set when the program starts

bool is_leap_year(int y)
{
	return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int days_in_month(int y, int m)
{
	if (m == 2) return is_leap_year(y) ? 29 : 28;
	if (m == 4 || m == 6 || m == 9 || m == 11) return 30;
	return 31;
}

bool valid_date(long d)
{
	if (d < 20000101L || d > 20991231L) return false;
	int y = d / 10000, m = (d / 100) % 100, day = d % 100;
	if (m < 1 || m > 12) return false;
	return day >= 1 && day <= days_in_month(y, m);
}

// number of days from 1 Jan 2000 to the date (so we can subtract dates)
long day_number(int date)
{
	int year = date / 10000, month = (date / 100) % 100, day = date % 100;
	long count = 0;
	for (int y = 2000; y < year; y++) count += is_leap_year(y) ? 366 : 365;
	for (int m = 1; m < month; m++) count += days_in_month(year, m);
	return count + day;
}

long days_between(int from, int to)
{
	return day_number(to) - day_number(from);
}

// date + n days (n is small, so we simply step one day at a time)
int add_days(int date, int n)
{
	int year = date / 10000, month = (date / 100) % 100, day = date % 100;
	for (int i = 0; i < n; i++)
	{
		day++;
		if (day > days_in_month(year, month))
		{
			day = 1;
			month++;
			if (month > 12)
			{
				month = 1;
				year++;
			}
		}
	}
	return year * 10000 + month * 100 + day;
}

// 20261105 -> "2026-11-05"
void date_to_text(int d, char out[])
{
	int y = d / 10000, m = (d / 100) % 100, day = d % 100;
	out[0] = '0' + (y / 1000) % 10;
	out[1] = '0' + (y / 100) % 10;
	out[2] = '0' + (y / 10) % 10;
	out[3] = '0' + y % 10;
	out[4] = '-';
	out[5] = '0' + m / 10;
	out[6] = '0' + m % 10;
	out[7] = '-';
	out[8] = '0' + day / 10;
	out[9] = '0' + day % 10;
	out[10] = '\0';
}

void print_date(int d)
{
	char t[12];
	date_to_text(d, t);
	cout << t;
}

void print_date_left(int d, int width)
{
	char t[12];
	date_to_text(d, t);
	print_left(t, width);
}

// ============================================================
//  READING INPUT  (we read one letter at a time with cin.get)
// ============================================================
bool input_ended = false;        // becomes true when there is no more input

bool is_space(char c)
{
	return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

// Reads one word. Returns false if the input has ended.
bool read_word(char dest[], int max)
{
	char c;
	int n = 0;
	bool found = false;
	while (cin.get(c))
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
	if (!found) input_ended = true;
	return found;
}

// Reads a whole number from low to high. Asks again when the input is wrong.
// If the input has ended it returns 'low' so that menus unwind and the program stops.
long read_number(const char message[], long low, long high)
{
	char word[32];
	long value;
	while (true)
	{
		cout << message;
		if (!read_word(word, 32))
		{
			cout << "\n[input ended]\n";
			return low;
		}
		if (text_to_number(word, value) && value >= low && value <= high) return value;
		cout << "  Please enter a whole number from " << low << " to " << high << ".\n";
	}
}

// Reads one word (no spaces) into dest.
void read_name(const char message[], char dest[], int max)
{
	cout << message;
	if (!read_word(dest, max)) copy_text(dest, "none", max);
}

int read_date(const char message[])
{
	while (true)
	{
		long d = read_number(message, 20000101L, 20991231L);
		if (valid_date(d) || input_ended) return d;
		cout << "  That is not a real date. Use YYYYMMDD, example 20261125.\n";
	}
}

bool ask_yes_no(const char message[])
{
	char word[8];
	while (true)
	{
		cout << message << " (y/n): ";
		if (!read_word(word, 8)) return false;
		if (word[0] == 'y' || word[0] == 'Y') return true;
		if (word[0] == 'n' || word[0] == 'N') return false;
	}
}

// ============================================================
//  Report : abstract base class for every report / analysis.
//  show() prints a title, then calls generate(). Each derived class has
//  its own generate()  ->  POLYMORPHISM.
// ============================================================
class Report
{
public:
	virtual const char* title() = 0;
	virtual void generate() = 0;
	void show()
	{
		cout << "\n########## " << title() << " ##########\n";
		generate();
	}
	virtual ~Report() {}
};

#endif
