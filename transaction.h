#ifndef TRANSACTION_H
#define TRANSACTION_H

/* The header definition for the Transaction data. */
class Transaction
{
	public:
		Transaction();
		Transaction(int acc, char tp, int amt);
		int account;
		char type;
		int amount;
};

#endif