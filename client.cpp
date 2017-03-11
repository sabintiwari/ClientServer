#include <cstring>
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
#include <unistd.h>


#define MAXDATASIZE 1024

using namespace std;


/* Global Variables */
int transaction_count = 0;
std::fstream transactions_file;
std::ofstream log_file;


/* Get the input from cin for the specified type. */
int get_int_input(std::string message)
{
	/* Prompt the user enter the amount. */
	int input;
	int end = 0;
	do
	{
		cout << message;
		cin >> input;
		if(cin.fail())
		{
			/* Show message and clear cin if there is an error. */
			cout << "Error! Input is invalid.\n";
	        cin.clear();
	        cin.ignore();
		}
		else
		{
			/* Exit the error loop if the input is valid. */
			end = 1;
		}
	} while (end == 0);
	return input;
}


/* Function that handles creating a connection to the server. */
int connect_to_server(int socket_fd, struct sockaddr_in server_address)
{
	/* Try to connect to the server using the address that was created. */
	int c = connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address));
	if(c < 0)
	{
		/* Show error and exit if the connection fails. */
		cout << "Error connecting to the server. No connection could be made using the provided address and port.\n";
		exit(0);
	}
}


/* Function that calls the server with the transaction and gets the response. */
int perform_transaction(std::string transaction, int socket_fd, struct sockaddr_in server_address)
{
	int code;

	/* Send the rransaction data to the server and wait for a response. */
	int w = write(socket_fd, &transaction, sizeof(transaction));
	if(w < 0)
	{
		/* Show error if the writing to socket fails. */
		cout << "Error writing the data to the socket. Error code: " << w << "\n";
		exit(1);
	}

	/* Wait for the server to acknowledge that the transaction succeeded. */
	int r = read(socket_fd, &code, sizeof(int));
	if(r < 0)
	{
		/* Show error when the reading from the server fails. */
		cout << "Error reading data from the server. Error code: " << r << "\n";
	}

	/* Show error if the transaction successed. Else show success. */
	if(code == -1)
	{
		cout << "Failed to complete transaction: " << transaction << "\n";
	}
	else
	{
		cout << "Successfully completed transaction: " << transaction << "\n";
	}

	return code;
}


/* Function that signals the server to withdraw using the account number. */
void withdraw_money(int account, int socket_fd, struct sockaddr_in server_address)
{
	/* Build the transaction string. */
	int amount = get_int_input("Enter the amount to withdraw:");
	transaction_count++;
	std::stringstream transaction;
	transaction << transaction_count << " " << account << " w " << amount;

	/* Call the perform function. */
	amount = perform_transaction(transaction.str(), socket_fd, server_address);
}


/* Function that signals the server to deposit using the account number. */
void deposit_money(int account, int socket_fd, struct sockaddr_in server_address)
{
	/* Build the transaction string. */
	int amount = get_int_input("Enter the amount to deposit:");
	transaction_count++;
	std::stringstream transaction;
	transaction << transaction_count << " " << account << " d " << amount;
	
	/* Call the perform function. */
	amount = perform_transaction(transaction.str(), socket_fd, server_address);
}


/* Function that handles user interaction and performs one action at a time. */
void user_interaction(int socket_fd, struct sockaddr_in server_address)
{
	/* Call the connect function. */
	connect_to_server(socket_fd, server_address);

	/* Get the bank account number and verify that it exists by connecting to the server. */
	printf("Welcome to Banking Service!\n");
	int account = get_int_input("Enter the account number to connect with: ");

	/* Send the account number to the server and wait for a response. */
	int w = write(socket_fd, &account, sizeof(int));
	if(w < 0)
	{
		/* Show error if the writing to socket fails. */
		cout << "Error writing the data to the socket. Error code: " << w << "\n";
		exit(1);
	}

	/* Wait for the server to acknowledge the account number exists. */
	int r = read(socket_fd, &account, sizeof(int));
	if(r < 0)
	{
		/* Show error when the reading from the server fails. */
		cout << "Error reading data from the server. Error code: " << r << "\n";
	}

	/* Check if the account was found. */
	if(account == -1)
	{
		cout << "Error! The account number was not found. Server response: " << account << "\n";
	}
	else
	{
		cout << account;
		/* Main loop that handles the client program. */
		while(1)
		{
			/* Get the input from the I/O and process the information. */
			int input = get_int_input("Please select an option:\n\t1. Deposit Money\n\t2. Withdraw Money\n\t3. Exit\n");
			switch(input)
			{
				case 1:
					deposit_money(account, socket_fd, server_address);
					break;
				case 2:
					withdraw_money(account, socket_fd, server_address);
					break;
				case 3:
					/* Exit the program. */
					cout << "\nFinished...\n";
					exit(0);
					break;
			}
		}
	}	
}


/* Function that handles the batch transactions from a file. */
void batch_transactions(int socket_fd, struct sockaddr_in server_address, std::string filename)
{
	int code = 0;
	std::string transaction;
	transactions_file.open(filename.c_str(), ios::in);

	/* Read all the lines in the file. */
	if(transactions_file.is_open())
	{
		while(std::getline(transactions_file, transaction))
		{
			/* Call the perform function. */
			perform_transaction(transaction, socket_fd, server_address);
		}
	}
	else
	{
		cout << "Error! Failed to read from file: " << filename << "\n";
		exit(0);
	}
}


/* Main function logic for the client program. */
int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		/* Show error if the correct number of arguments were not passed. */
		cout << "Usage: client <hostname> <port_number>\n";
		exit(1);
	}

	/* Clear the log file at start. */
	log_file.open("client_log_file.txt", ios::out | ios::trunc);
	if(log_file.is_open())
	{
		log_file << "";
	}
	else
	{
		cout << "Failed to open log file. Please see terminal for output.\n";
	}

	/* Setup the connection information to the server. */
	struct hostent *server;
	int port_number;
	port_number = atoi(argv[2]);
	server = gethostbyname(argv[1]);
	if(server == NULL)
	{
		/* Show error if the server does not exist. */
		cout << "No host exists with the address: " << argv[1] << "\n";
		exit(0);
	}

	/* Get the address to the server. */
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	bcopy((char *)server -> h_addr, (char *)&server_address.sin_addr.s_addr, server -> h_length);
	server_address.sin_port = htons(port_number);

	/* Setup the socket. */
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		/* Show error if the socket descriptor fails. */
		cout << "Socket is not formed. Error code: " << socket_fd << "\n";
		exit(0);
	}

	/* Check the number of arguments and perform the respective function. */
	if(argc == 3)
	{
		/* Call the method that handles user interactive if 3 arguments are passed. */
		user_interaction(socket_fd, server_address);
	}
	else
	{
		/* Call the method that handles file transactions. */
		batch_transactions(socket_fd, server_address, argv[3]);
	}
}