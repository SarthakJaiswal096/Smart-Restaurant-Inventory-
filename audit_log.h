// audit_log.h - AUDIT LOG
//
// DSA concepts : stack (the last 10 actions are kept newest on top)

#ifndef AUDIT_LOG_H
#define AUDIT_LOG_H

#include "common.h"
// ============================================================
//  AuditLog : writes important actions to audit_log.txt and keeps the
//  last 10 in a STACK (newest on top) for the "recent activity" view.
// ============================================================
const int RECENT_MAX = 10;
class AuditLog
{
private:
	char recent[RECENT_MAX][WORD_LEN * 2];
	int top;                        // how many entries are stored
public:
	AuditLog() {
		top = 0; 
	}

	// action is one word, for example added_food_7
	void write(const char user[], const char action[])
	{
		char line[WORD_LEN * 2], date[12];
		date_to_text(today, date);
		copy_text(line, date, WORD_LEN * 2);
		append_text(line, " ", WORD_LEN * 2);
		append_text(line, user, WORD_LEN * 2);
		append_text(line, " ", WORD_LEN * 2);
		append_text(line, action, WORD_LEN * 2);

		ofstream out(AUDIT_FILE, ios::app);                  // ios::app = add at the end of the file
		if (out) out << line << "\n";
		out.close();

		if (top == RECENT_MAX)                               // stack full: forget the oldest entry
		{
			for (int i = 1; i < RECENT_MAX; i++) copy_text(recent[i - 1], recent[i], WORD_LEN * 2);
			top--;
		}
		copy_text(recent[top], line, WORD_LEN * 2);
		top++;
	}

	void show_recent()                                       // pop order: newest first
	{
		cout << "\nRecent activity (this session, newest first):\n";
		for (int i = top - 1; i >= 0; i--) cout << "  " << recent[i] << "\n";
		if (top == 0) cout << "  (nothing yet)\n";
	}

	void show_all()
	{
		ifstream in(AUDIT_FILE);
		if (!in)
		{
			cout << "(no audit log yet)\n";
			return;
		}
		cout << "\nFull audit log (date user action):\n";
		char c;
		while (in.get(c)) cout << c;                         // copy the file to the screen
		in.close();
	}
};

#endif
