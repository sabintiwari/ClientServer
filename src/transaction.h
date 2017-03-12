#ifndef TRANSACTION_H
#define TRANSACTION_H

/* The header definition for the Transaction data. */
class Transaction
{
	public:
		Transaction();
		Transaction(int tm, int acc, std::string tp, int amt);
		Transaction(std::string transaction);
		int is_valid();
		int time;
		int account;
		std::string type;
		int amount;
};

#endif