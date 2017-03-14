/*
	Sabin Raj Tiwari
	CMSC 621
	Project 1
*/

#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h> 
#include <sys/types.h>
#include <unistd.h>

#include "logger.h"
#include "transaction.h"

using namespace std;


/* Global Variables */
float request_rate = 0.5;
std::fstream transactions_file;
Logger* batch_log;
Logger* logger;


/* Get the string value from an int. */
std::string i_to_s(int value)
{
	std::stringstream str;
	str << value;
	return str.str();
}


/* Get the string value for money. */
std::string m_to_s(double value)
{
	std::ostringstream moneystream;
	moneystream << fixed << std::setprecision(2) << value;
	return moneystream.str();
}


/* Create a connection to the server and return the socket file descriptor. */
int connect_to_server(struct sockaddr_in server_address)
{
	/* Setup the socket. */
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		/* Show error if the socket descriptor fails. */
		logger->log("Socket is not formed.");
		exit(0);
	}

	/* Try to connect to the server using the address that was created. */
	int c = connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address));
	if(c < 0)
	{
		/* Show error and exit if the connection fails. */
		logger->log("Error connecting to the server. No connection could be made using the provided address and port.");
		exit(0);
	}

	return socket_fd;
}


/* Read the transactions from a file and perform multiple transactions. */
float batch_transactions(struct sockaddr_in server_address, std::string filename)
{
	/* Declare and initialize required variables. */
	Transaction* t_ptr = new Transaction(0, -1, "", -1.0);
	double code = -1.0;
	int account, socket_fd, r, w;
	int transaction_count = 0;
	int prev_time = 0;
	float total_time = 0.0;
	long seconds, useconds, mseconds;
	struct timeval start_time, end_time;

	/* Setup the string for reading and open the file. */
	std::string transaction;
	std::string message;
	transactions_file.open(filename.c_str(), ios::in);

	/* Read all the lines in the file. */
	if(transactions_file.is_open())
	{
		/* Call the connect function. */
		socket_fd = connect_to_server(server_address);

		while(std::getline(transactions_file, transaction))
		{
			/* Sleep based on the transaction time. */
			t_ptr->reset(transaction);
			if(t_ptr->time > prev_time)
			{
				usleep((t_ptr->time - prev_time)*(request_rate * 1000000));
				prev_time = t_ptr->time;
			}
			else
				usleep(request_rate * 1000000);

			/* Create the buffer. */
			// char buffer;
			// transaction.copy(buffer, transaction.size());

			/* Mark the start time. */
			gettimeofday(&start_time, 0);

			/* Send the rransaction data to the server and wait for a response. */
			w = write(socket_fd, transaction.c_str(), transaction.size());
			if(w < 0)
			{
				/* Show error if the writing to socket fails. */
				logger->log("Error writing the data to the socket.");
			}
			else 
			{
				/* Log the data being sent to the server. */
				message = "Data sent to server. Data: " + transaction;
				logger->log(message);

				/* Wait for the server to acknowledge that the transaction succeeded. */
				r = read(socket_fd, &code, sizeof(double));
				if(r < 0.0)
				{
					/* Show error when the reading from the server fails. */
					logger->log("Error reading data from the server.");
				}

				/* Mark the end time. */
				gettimeofday(&end_time, 0);

				/* Log the data being sent to the server. */

				message = "Data received from server. Data: " + m_to_s(code);
				logger->log(message);

				/* Create the time stamps into seconds and update the total time. */
				seconds = end_time.tv_sec - start_time.tv_sec;
				useconds = (seconds * 1000000) + end_time.tv_usec - start_time.tv_usec;
				std::ostringstream timestream;
				timestream << (useconds / 1000000.0);
				total_time += (useconds / 1000000.0);

				/* Log the data being sent to the server. */
				message = "Time taken for transaction: " + timestream.str() + " seconds.";
				logger->log(message);
			}

			/* Show error if the transaction successed. Else show success. */
			if(code >= 0.0)
			{
				batch_log->log("(" + transaction + ") completed successfully. Resulting balance: $" + m_to_s(code));	
			}
			else if(code == -1.0)
			{
				batch_log->log("(" + transaction + ") failed to complete. Account not found.");
			}
			else if(code == -2.0)
			{
				batch_log->log("(" + transaction + ") failed to complete. Insufficient funds.");
			}

			/* Update the number of transactions. */
			transaction_count++;
			logger->log("\n");
		}

		/* Close the file. */
		transactions_file.close();

		/* Signal the server to finish. */
		std::string signal = "finish";

		/* Send the rransaction data to the server and wait for a response. */
		w = write(socket_fd, signal.c_str(), 6);
		if(w < 0)
		{
			/* Show error if the writing to socket fails. */
			logger->log("Error writing the data to the socket.");
		}
		
		/* Close the connection. */
		close(socket_fd);
	}
	else
	{
		logger->log("Error! Failed to read from file: " + filename);
	}	

	return total_time / transaction_count;
}


/* Main function logic for the client program. */
int main(int argc, char *argv[])
{
	if(argc < 5)
	{
		/* Show error if the correct number of arguments were not passed. */
		cerr << "Usage: client <hostname> <port_number> <time_step> <transactions_file>\n";
		exit(1);
	}

	request_rate = atof(argv[3]);
	if(request_rate < 0.0)
	{
		/* Show error if the timestep is less than 0. */
		cerr << "Error! Timestep has to be greater than or equal to 0.0\n";
		exit(1);
	}

	/* Setup the log files for the current client. */
	int pid = ::getpid();
	logger = new Logger("./logs/" + i_to_s(pid) + "_client_log_file.txt");
	batch_log = new Logger("./logs/" + i_to_s(pid) + "_transactions_log_file.txt");

	/* Setup the connection information to the server. */
	struct hostent *server;
	int port_number;
	port_number = atoi(argv[2]);
	server = gethostbyname(argv[1]);
	if(server == NULL)
	{
		/* Show error if the server does not exist. */
		std::string err_msg("No host exists with the address: ");
		logger->log(err_msg + argv[1]);
		exit(0);
	}

	/* Get the address to the server. */
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	bcopy((char *)server -> h_addr, (char *)&server_address.sin_addr.s_addr, server -> h_length);
	server_address.sin_port = htons(port_number);

	/* Call the method that handles file transactions. */
	float avg_time = batch_transactions(server_address, argv[4]);

	/* Update the transactions log file with the average time and close the connection. */
	batch_log->log("\n");
	std::ostringstream timestream;
	timestream << avg_time;
	batch_log->log("Average transaction time: " + timestream.str() + " seconds");
	batch_log->close();


	logger->log("Process " + i_to_s(pid) + " completed. Please see logs/" + i_to_s(pid) + "_transactions_log_file.txt for the transaction log.");
	logger->close();
	cout << "\nSee logs/" + i_to_s(pid) + "_client_log_file.txt for the log file output of the client.\n\n";
	return 1;
}