#ifndef TRANSACTION_H
#define TRANSACTION_H

/* The header definition for the Transaction data. */
class Transaction
{
	public:
		Transaction(int account, char type, int amount);
		int account;
		char type;
		int amount;
};

#endif