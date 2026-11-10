// stock_structures.h - DATA STRUCTURES USED BY THE INVENTORY
//
// DSA concepts : min-heap   (FEFO: always use the batch that expires first)
//                hash table (find an ingredient by ID)
//                stack      (undo the last stock operation)

#ifndef STOCK_STRUCTURES_H
#define STOCK_STRUCTURES_H

#include "common.h"
#include "ingredient.h"
#include "transaction.h"

// ============================================================
//  MinHeap : a binary tree stored in an array.
//  The root (index 0) is always the batch that expires FIRST.
//  Parent of i = (i-1)/2,  children of i = 2i+1 and 2i+2.
// ============================================================
struct HeapNode
{
	int expiry;
	int batch_id;
	int index;               // position of the batch in the inventory's batch array
};

class MinHeap
{
private:
	HeapNode a[MAX_BATCHES + 1];
	int n;
	// is x earlier than y ?  Same expiry date -> smaller batch ID first.
	bool earlier(HeapNode x, HeapNode y)
	{
		if (x.expiry != y.expiry) return x.expiry < y.expiry;
		return x.batch_id < y.batch_id;
	}
	void swap_nodes(int i, int j)
	{
		HeapNode t = a[i];
		a[i] = a[j];
		a[j] = t;
	}
public:
	MinHeap(){ 
		n = 0; 
	}
	bool is_empty(){ 
		return n == 0; 
	}
	void insert(HeapNode x)
	{
		if (n >= MAX_BATCHES) return;
		a[n] = x;
		int i = n;
		n++;
		while (i > 0 && earlier(a[i], a[(i - 1) / 2]))        // move up while smaller than the parent
		{
			swap_nodes(i, (i - 1) / 2);
			i = (i - 1) / 2;
		}
	}
	HeapNode extract_min()
	{
		HeapNode top = a[0];
		n--;
		a[0] = a[n];
		int i = 0;
		while (true)                                           // move down while bigger than a child
		{
			int left = 2 * i + 1, right = 2 * i + 2, smallest = i;
			if (left < n && earlier(a[left], a[smallest])) smallest = left;
			if (right < n && earlier(a[right], a[smallest])) smallest = right;
			if (smallest == i) break;
			swap_nodes(i, smallest);
			i = smallest;
		}
		return top;
	}
};

// ============================================================
//  IdTable : hash table  ingredient ID -> position in the ingredient array.
//  hash function: key % 127.  Collision -> try the next slot (linear probing).
// ============================================================
const int ID_TABLE_SIZE = 127;

class IdTable
{
private:
	int keys[ID_TABLE_SIZE];
	int values[ID_TABLE_SIZE];
	bool used[ID_TABLE_SIZE];
public:
	IdTable() { clear(); }

	void clear()
	{
		for (int i = 0; i < ID_TABLE_SIZE; i++)
		{
			used[i] = false;
			keys[i] = 0;
			values[i] = -1;
		}
	}

	void insert(int key, int value)
	{
		int slot = key % ID_TABLE_SIZE;
		for (int step = 0; step < ID_TABLE_SIZE; step++)
		{
			if (!used[slot] || keys[slot] == key)
			{
				used[slot] = true;
				keys[slot] = key;
				values[slot] = value;
				return;
			}
			slot = (slot + 1) % ID_TABLE_SIZE;
		}
	}

	int find(int key)
	{
		int slot = key % ID_TABLE_SIZE;
		for (int step = 0; step < ID_TABLE_SIZE; step++)
		{
			if (!used[slot]) return -1;                        // empty slot: not in the table
			if (keys[slot] == key) return values[slot];
			slot = (slot + 1) % ID_TABLE_SIZE;
		}
		return -1;
	}
};

// ============================================================
//  UndoStack : last in, first out (LIFO).
//  Every stock change is pushed; "undo" pops the newest one.
//  Entries made by ONE action share a 'group' number and are undone together.
// ============================================================
const int UNDO_SIZE = 100;

struct UndoEntry
{
	char type;               // 'R' = received, 'U' = used, 'W' = wastage
	int batch_id;
	int ing_id;
	long qty;
	int group;
};

class UndoStack
{
private:
	UndoEntry data[UNDO_SIZE];
	int top;                 // how many entries are stored
public:
	UndoStack() { top = 0; }
	bool is_empty() { return top == 0; }
	void clear() { top = 0; }

	void push(UndoEntry e)
	{
		if (top == UNDO_SIZE)                                  // full: forget the oldest entry
		{
			for (int i = 1; i < UNDO_SIZE; i++) data[i - 1] = data[i];
			top--;
		}
		data[top] = e;
		top++;
	}
	UndoEntry peek() { return data[top - 1]; }
	UndoEntry pop()
	{
		top--;
		return data[top];
	}
};

#endif
