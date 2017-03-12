/*
	Sabin Raj Tiwari
	CMSC 621
	Project 1
*/

#ifndef RECORD_H
#define RECORD_H

/* The header definition for the Record data. */
class Record
{
	public:
		Record();
		Record(int acc, std::string nm, int blnc);
		Record(std::string  record);
		int account;
		std::string name;
		int balance;
		int is_locked;
		pthread_cond_t in_use;
};

#endif