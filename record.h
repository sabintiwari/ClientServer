#ifndef RECORD_H
#define RECORD_H

/* The header definition for the Record data. */
class Record
{
	public:
		Record();
		Record(int acc, char *nm, int blnc);
		int account;
		char *name;
		int balance;
		int is_locked;
};

#endif