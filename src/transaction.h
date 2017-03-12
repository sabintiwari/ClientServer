/*
	Sabin Raj Tiwari
	CMSC 621
	Project 1
*/

#ifndef TRANSACTION_H
#define TRANSACTION_H

/* The header definition for the Transaction data. */
class Transaction
{
	public:
		Transaction();
		Transaction(int tm, int acc, std::string tp, double amt);
		Transaction(std::string transaction);
		int is_valid();
		int time;
		int account;
		std::string type;
		double amount;
};

#endif