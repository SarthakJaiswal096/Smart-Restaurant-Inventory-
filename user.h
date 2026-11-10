// user.h - LOGIN AND ROLES
//
// NOTE: the login is a college demo. Passwords are stored as plain text, which is NOT
//       secure. A real system would store a hashed password.
// DSA concepts : linear search (find a user)

#ifndef USER_H
#define USER_H

#include "common.h"

// ---------- menu sections (used for role permissions) ----------
const int SEC_FOOD = 1;
const int SEC_ORDERS = 2;
const int SEC_RECIPES = 3;
const int SEC_INVENTORY = 4;
const int SEC_WORKFLOW = 5;
const int SEC_REPORTS = 6;
const int SEC_SYSTEM = 7;

// ============================================================
//  User + LoginManager
// ============================================================
class User
{
private:
	char username[NAME_LEN];
	char password[NAME_LEN];
	char role[12];                  // ADMIN, ORDER or INVENTORY
public:
	User()
	{
		username[0] = '\0';
		password[0] = '\0';
		role[0] = '\0';
	}
	User(const char u[], const char p[], const char r[])
	{
		copy_text(username, u, NAME_LEN);
		copy_text(password, p, NAME_LEN);
		copy_text(role, r, 12);
	}
	const char* get_username(){ 
		return username; 
	}
	const char* get_password(){ 
		return password; 
	}
	const char* get_role(){ 
		return role; 
	}
};

class LoginManager
{
private:
	User users[MAX_USERS];
	int count;
	int current;                    // index of the logged-in user, -1 = nobody
public:
	LoginManager()
	{
		count = 0;
		current = -1;
	}
	int size() { 
		return count; 
	}
	User& at(int i) { 
		return users[i]; 
	}
	void clear()
	{
		count = 0;
		current = -1;
	}

	int find_user(const char name[])                         // linear search
	{
		for (int i = 0; i < count; i++){
			if (same_text(users[i].get_username(), name)) return i;
		}
		return -1;
	}

	bool add_user(const char u[], const char p[], const char r[])
	{
		if (count >= MAX_USERS || u[0] == '\0' || p[0] == '\0') return false;
		if (!same_text(r, "ADMIN") && !same_text(r, "ORDER") && !same_text(r, "INVENTORY")) return false;
		if (find_user(u) != -1) return false;
		users[count] = User(u, p, r);
		count++;
		return true;
	}

	// used when the users file does not exist
	void create_default_users()
	{
		add_user("admin", "admin123", "ADMIN");
		add_user("order", "order123", "ORDER");
		add_user("inventory", "inv123", "INVENTORY");
	}

	// Asks for username and password (3 attempts). Returns true when the login is correct.
	bool login()
	{
		char u[NAME_LEN], p[NAME_LEN];
		cout << "\n----- LOGIN -----  (demo accounts: see README.txt)\n";
		for (int attempt = 1; attempt <= 3; attempt++)
		{
			read_name("Username: ", u, NAME_LEN);
			read_name("Password: ", p, NAME_LEN);
			int i = find_user(u);
			if (i != -1 && same_text(users[i].get_password(), p))
			{
				current = i;
				return true;
			}
			cout << "Wrong username or password (" << attempt << "/3).\n";
			if (input_ended) return false;
		}
		return false;
	}

	// after the users are loaded again (restore), keep the same person logged in
	void resume_session(const char name[]) { current = find_user(name); }

	const char* current_name()
	{
		if (current < 0) return "nobody";
		return users[current].get_username();
	}
	const char* current_role()
	{
		if (current < 0) return "NONE";
		return users[current].get_role();
	}

	// ADMIN can do everything. ORDER staff: food, orders, recipes, workflow, reports.
	// INVENTORY staff: recipes, inventory, workflow, reports.
	bool check_permission(int section)
	{
		const char* r = current_role();
		if (same_text(r, "ADMIN")) return true;
		if (same_text(r, "ORDER"))
			return section == SEC_FOOD || section == SEC_ORDERS || section == SEC_RECIPES ||
			       section == SEC_WORKFLOW || section == SEC_REPORTS;
		if (same_text(r, "INVENTORY"))
			return section == SEC_RECIPES || section == SEC_INVENTORY ||
			       section == SEC_WORKFLOW || section == SEC_REPORTS;
		return false;
	}
};

#endif
