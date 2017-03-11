#include "transaction.h"

// Default constructor
Transaction::Transaction()
{

};
// Transaction constructor
Transaction::Transaction(int acc, char tp, int amt)
{
	account = acc;
	type = tp;
	amount = amt;
};