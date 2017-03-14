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
		Record(int acc, std::string nm, double blnc);
		Record(std::string  record);
		int account;
		std::string name;
		double balance;
		int is_locked;
		pthread_mutex_t lock;
		pthread_cond_t cond;
};

#endif