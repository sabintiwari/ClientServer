#ifndef LOGGER_H
#define LOGGER_H

/* The header definition for the Logger class. */
class Logger
{
	public:
		Logger();
		Logger(std::string filename);
		void log(std::string message);
		void close();
		std::string i_to_s(int value);
		std::ofstream log_file;
};

#endif