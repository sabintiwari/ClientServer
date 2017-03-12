/*
	Sabin Raj Tiwari
	CMSC 621
	Project 1
*/

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "transaction.h"

using namespace std;

// Default constructor
Transaction::Transaction()
{

};
// Transaction constructor
Transaction::Transaction(int tm, int acc, std::string tp, int amt)
{
	time = tm;
	account = acc;
	type = tp;
	amount = amt;
};
//Transaction constructor with a string
Transaction::Transaction(std::string transaction)
{
	int i = 0;
	std::string line_arr[4];

	std::istringstream istr(transaction);
	/* Get each of the tokens in the stream to the array. */
	while(istr.good() && i < 4)
	{
		istr >> line_arr[i];
		++i;
	}

	try 
	{
		/* Convert the types to the required ones for Transaction. */
		time = atoi(line_arr[0].c_str());
		account = atoi(line_arr[1].c_str());
		amount = atoi(line_arr[3].c_str());
		type = line_arr[2];
	}
	catch (int e)
	{
		time = -1;
		account = -1;
		type = "";
		amount = -1.0;
	}	
};
//Method to check if the transaction data is valid.
int Transaction::is_valid()
{
	if(time > 0 && account > -1 && (type == "w" || type == "d" || type == "d") && amount > 0)
	{
		return 1;
	}
	return 0;
}