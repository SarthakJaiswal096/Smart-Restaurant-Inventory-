// backup_manager.h - BACKUP AND RESTORE
//
// Copies the data files to backup_<name> files and back, letter by letter.

#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include "common.h"

// ============================================================
//  BackupManager : copies the data files to backup_<name> files and back
// ============================================================
class BackupManager
{
private:
	// copies a file letter by letter
	bool copy_file(const char from[], const char to[])
	{
		ifstream in(from, ios::binary);
		if (!in) return false;
		ofstream out(to, ios::binary);
		if (!out) return false;
		char c;
		while (in.get(c)) out.put(c);
		return true;
	}
public:
	int create_backup()
	{
		int n = 0;
		for (int i = 0; i < FILE_COUNT; i++)
		{
			char name[WORD_LEN];
			copy_text(name, "data/backup_", WORD_LEN);
			// skip leading "data/" prefix (5 characters) from DATA_FILES[i]
			const char* filename = DATA_FILES[i];
			if (filename[0] == 'd' && filename[1] == 'a' && filename[2] == 't' && filename[3] == 'a' && filename[4] == '/')
				filename += 5;
			append_text(name, filename, WORD_LEN);
			if (copy_file(DATA_FILES[i], name)) n++;
		}
		return n;
	}

	// Copies backup files over the live files. The caller must load the data again afterwards.
	int restore_backup()
	{
		int n = 0;
		for (int i = 0; i < FILE_COUNT; i++)
		{
			char name[WORD_LEN];
			copy_text(name, "data/backup_", WORD_LEN);
			const char* filename = DATA_FILES[i];
			if (filename[0] == 'd' && filename[1] == 'a' && filename[2] == 't' && filename[3] == 'a' && filename[4] == '/')
				filename += 5;
			append_text(name, filename, WORD_LEN);
			if (copy_file(name, DATA_FILES[i])) n++;
		}
		return n;
	}
};

#endif
