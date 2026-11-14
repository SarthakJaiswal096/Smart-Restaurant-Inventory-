#ifndef LOGIN_H
#define LOGIN_H

#include <iostream>
#include <string>
using namespace std;

class User
{
private:
    string username, password, role;

public:
    User(string u = "", string p = "", string r = "")
    {
        username = u;
        password = p;
        role = r;
    }
    bool check(string u, string p) { return username == u && password == p; }
    string getRole() { return role; }
};

class LoginManager
{
private:
    User users[3];

public:
    LoginManager()
    {
        users[0] = User("admin", "admin123", "ADMIN");
        users[1] = User("staff", "staff123", "STAFF");
        users[2] = User("stock", "stock123", "STOCK_FILLER");
    }

    string login(string username, string password)
    {
        for (int i = 0; i < 3; i++)
            if (users[i].check(username, password)) return users[i].getRole();
        return "";
    }
};

#endif
