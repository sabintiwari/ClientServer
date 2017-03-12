/*
	Sabin Raj Tiwari
	CMSC 621
	Project 1
*/

#ifndef LOGGER_H
#define LOGGER_H

/* The header definition for the Logger class.*/
class Logger
{
	public:
		Logger();
		Logger(std::string filename);
		void log(std::string message);
		void close();
		std::ofstream log_file;
};

#endif