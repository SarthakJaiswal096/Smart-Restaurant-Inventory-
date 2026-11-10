// transaction.h - STOCK TRANSACTION HISTORY
//
// A Transaction is a history record of every stock change.

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "common.h"

// ============================================================
//  Transaction : history record of every stock change.
//  type: RECEIVE, USE, WASTE, UNDO_RECEIVE, UNDO_USE, UNDO_WASTE
// ============================================================
class Transaction
{
private:
	int id;
	int date;
	char type[16];
	int ing_id;
	int batch_id;
	long qty;
	char note[WORD_LEN];
public:
	Transaction()
	{
		id = 0;
		date = 0;
		type[0] = '\0';
		ing_id = 0;
		batch_id = 0;
		qty = 0;
		note[0] = '\0';
	}
	Transaction(int i, int d, const char t[], int g, int b, long q, const char n[])
	{
		id = i;
		date = d;
		copy_text(type, t, 16);
		ing_id = g;
		batch_id = b;
		qty = q;
		copy_text(note, n, WORD_LEN);
	}
	int get_id(){ 
		return id; 
	}
	int get_date(){ 
		return date; 
	}
	const char* get_type(){ 
		return type; 
	}
	int get_ing_id(){ 
		return ing_id; 
	}
	int get_batch_id(){ 
		return batch_id; 
	}
	long get_qty(){ 
		return qty; 
	}
	const char* get_note(){ 
		return note; 
	}
};

#endif
