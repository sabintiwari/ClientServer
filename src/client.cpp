#include <cstring>
#include <ctime>
#include <iostream>
#include <fstream>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h> 
#include <unistd.h>

#include "logger.h"
#include "transaction.h"

#define MAXDATASIZE 1024

using namespace std;


/* Global Variables */
int transaction_count = 0;
std::fstream transactions_file;
Logger* batch_log = new Logger("logs/transactions_log_file.txt");
Logger* logger = new Logger("logs/client_log_file.txt");


/* Function that handles creating a connection to the server. */
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


/* Function that handles the batch transactions from a file. */
void batch_transactions(struct sockaddr_in server_address, std::string filename)
{
	int account, socket_fd, r, w;
	int code = -1;
	char buffer[MAXDATASIZE];
	std::string transaction;
	transactions_file.open(filename.c_str(), ios::in);

	/* Read all the lines in the file. */
	if(transactions_file.is_open())
	{
		while(std::getline(transactions_file, transaction))
		{
			/* Call the connect function. */
			socket_fd = connect_to_server(server_address);

			/* Clear the buffer. */
			memset(&buffer[0], 0, MAXDATASIZE);
			strncpy(buffer, transaction.c_str(), transaction.size());

			/* Send the rransaction data to the server and wait for a response. */
			w = write(socket_fd, &buffer, strlen(buffer));
			if(w < 0)
			{
				/* Show error if the writing to socket fails. */
				logger->log("Error writing the data to the socket.");
			}
			else 
			{
				/* Wait for the server to acknowledge that the transaction succeeded. */
				r = read(socket_fd, &code, sizeof(int));
				if(r < 0)
				{
					/* Show error when the reading from the server fails. */
					logger->log("Error reading data from the server.");
				}
				
			}

			/* Show error if the transaction successed. Else show success. */
			if(code < 0)
			{
				batch_log->log("(" + transaction + ") failed to complete.");
			}
			else
			{
				batch_log->log("(" + transaction + ") completed successfully. Resulting balance: " + logger->i_to_s(code));
			}

			/* Close the connection. */
			close(socket_fd);
		}
		/* Close the file. */
		transactions_file.close();

	}
	else
	{
		logger->log("Error! Failed to read from file: " + filename);
	}

	
}


/* Main function logic for the client program. */
int main(int argc, char *argv[])
{
	if(argc < 4)
	{
		/* Show error if the correct number of arguments were not passed. */
		cerr << "Usage: client <hostname> <port_number> <transactions_file>\n";
		exit(1);
	}

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
	batch_transactions(server_address, argv[3]);

	batch_log->close();
	logger->log("Process completed. Please see transaction_log_file.txt for the transaction log.");
	logger->close();
}