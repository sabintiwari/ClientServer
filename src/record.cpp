#include <cstring>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
/*
	Sabin Raj Tiwari
	CMSC 621
	Project 1
*/

#include "record.h"

using namespace std;

// Default constructor
Record::Record()
{

};
// Record constructor
Record::Record(int acc, std::string nm, double blnc)
{
	account = acc;
	name = nm;
	balance = blnc;
	is_locked = 0;
	pthread_cond_init(&in_use, NULL);
};
//Records constructor with a string
Record::Record(std::string record)
{
	int i = 0;
	std::string line_arr[3];

	std::istringstream istr(record);
	/* Get each of the tokens in the stream to the array. */
	while(istr.good() && i < 3)
	{
		istr >> line_arr[i];
		++i;
	}

	try 
	{
		/* Convert the types to the required ones for Record. */
		account = atoi(line_arr[0].c_str());
		balance = atoi(line_arr[2].c_str());
		name = line_arr[1];
		is_locked = 0;
		pthread_cond_init(&in_use, NULL);
	}
	catch (int e)
	{
		account = -1;
		name = "";
		balance = -1.0;
	}	
};