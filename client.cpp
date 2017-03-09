#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXDATASIZE 1024

/* Get the input from cin for the specified type. */
int get_int_input(char message[])
{
	/* Prompt the user enter the amount. */
	int amount;
	int end = 0;
	do
	{
		cout << message;
		cin >> amount;
		if(cin.fail())
		{
			/* Show message and clear cin if there is an error. */
			fprintf(stderr, "Error! Input is invalid.\n"):
	        cin.clear();
	        cin.ignore();
		}
		else
		{
			end = 1;
		}
	} while (end == 0);
}

/* Function that signals the server to withdraw. */
void withdraw(int socket_fd, struct sockaddr_in server_address)
{
	int amount = get_int_input("Enter the amount to withdraw:");
}

/* Function that signals the server to deposit. */
void deposit(int socket_fd, struct sockaddr_in server_address)
{
	int amount = get_int_input("Enter the amount to deposit:");
}`

/* Main function logic for the client program. */
int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		/* Show error if the correct number of arguments were not passed. */
		fprintf(stderr, "Usage: client <hostname> <port_numner>\n");
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
		fprintf(stderr, "No such host exists.\n");
		exit(1);
	}

	/* Get the address to the server. */
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	bcopy((char *)server -> h_addr, (char *)&server_address.sin_addr.s_addr, server -> h_length);
	server_address.sin_port = htons(portno);

	/* Setup the socket. */
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		/* Show error if the socket descriptor fails. */
		fprintf(stderr, "Socket is not formed.\n");
		exit(0);
	}

	/* Get the bank account number and verify that it exists by connecting to the server. */
	printf("Welcome to Banking Server!\n");
	int account = get_int_input("Enter the account number:");
	int c = connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if(c < 0)
	{
		/* Show error if the connection fails. */
		fprintf(stderr, "Error connecting to the server.\n");
	}
	/* Send the account number to the server and wait for a response. */
	int w = write(socket_fd, &account sizeof(int));
	if(w < 0)
	{
		/* Show error if the writing to socket fails. */
		fprintf(stderr, "Error writing the data to the socket.\n");
		exit(1);
	}

	

	/* Main loop that handles the client program. */
	while(1)
	{
		cout << "Please select an option:\n\t1. Deposit Money\n\t2. Withdraw Money\n\t3. Exit\n";
		int input;
		/* Take the input from the terminal. */
		cin >> input;
		if(input > 0 && input < 4)
		{
			switch(input)
			{
				case 1:
					deposit(socket_fd);
					break;
				case 2:
					withdraw(socket_fd);
					break;
				case 3:
					/* Exit the program. */
					cout << "\nFinished...";
					exit(0);
					break;
			}
		}
		else
		{
			/* Show error and continue the loop if the input is invalid. */
			fprintf(stderr, "Error! Input is invalid.\n");
		}
	}
}