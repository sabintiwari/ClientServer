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
Logger* batch_log = new Logger("transactions_log_file.txt");
Logger* logger = new Logger("client_log_file.txt");


/* Get the input from cin for the specified type. */
int get_int_input(std::string message)
{
	/* Prompt the user enter the amount. */
	std::string input;
	int value;
	int end = 0;
	do
	{
		cout << message;
		cin >> input;
		value = atoi(input.c_str());
		/* If the value converted is greater than 0, return the value. */
		if(value == 0) 
		{
			cout << "Error! Invalid input.\n";	
		}
		else
		{
			end = 1;
		}
	} while (end == 0);
	return value;
}


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


/* Function that calls the server with the transaction and gets the response. */
int perform_transaction(std::string transaction, int socket_fd, struct sockaddr_in server_address)
{
	int code = -2;
	char buffer[MAXDATASIZE];
	strncpy(buffer, transaction.c_str(), transaction.size());

	/* Send the rransaction data to the server and wait for a response. */
	int w = write(socket_fd, &buffer, strlen(buffer));
	if(w < 0)
	{
		/* Show error if the writing to socket fails. */
		logger->log("Error writing the data to the socket.");
	}
	else 
	{
		/* Wait for the server to acknowledge that the transaction succeeded. */
		int r = read(socket_fd, &code, sizeof(int));
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
	
	return code;
}


/* Function that handles user interaction and performs one action at a time. */
void user_interaction(struct sockaddr_in server_address)
{
	int end = 0;
	std::string transaction;

	/* Get the bank account number and verify that it exists by connecting to the server. */
	printf("\nWelcome to Banking Service!\n");

	/* Call the connect function. */
	int socket_fd = connect_to_server(server_address);

	/* Main loop that handles the client program. */
	while(end == 0)
	{
		/* Get the input from the I/O and process the information. */
		int input = get_int_input("\nPlease select an option:\n\t1. Enter Query\n\t2. Exit\n");
		switch(input)
		{
			case 1:
				/* Send the transaction. */
				cout << "Transaction types: \"w\" - withdraw; \"d\" - deposit\n";
				cout << "Type query <account_number> <type> <amount>: ";
				std::getline(std::cin, transaction);
				perform_transaction(transaction, socket_fd, server_address);
				break;
			case 2:
				/* Exit the program. */
				end = 1;
				break;
		}
	}
}


/* Function that handles the batch transactions from a file. */
void batch_transactions(struct sockaddr_in server_address, std::string filename)
{
	int account, socket_fd;
	std::string transaction;
	transactions_file.open(filename.c_str(), ios::in);

	/* Call the connect function. */
	socket_fd = connect_to_server(server_address);

	/* Read all the lines in the file. */
	if(transactions_file.is_open())
	{
		while(std::getline(transactions_file, transaction))
		{
			perform_transaction(transaction, socket_fd, server_address);
		}
		/* Close the file. */
		transactions_file.close();
	}
	else
	{
		logger->log("Error! Failed to read from file: " + filename);
	}

	/* Close the connection. */
	close(socket_fd);
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