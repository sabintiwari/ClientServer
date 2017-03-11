#include "record.h"

// Default constructor
Record::Record()
{

};
// Record constructor
Record::Record(int acc, char* nm, int blnc)
{
	account = acc;
	name = nm;
	balance = blnc;
	is_locked = 0;
};