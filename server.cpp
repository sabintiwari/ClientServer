#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define MAXDATASIZE 1024
#define MAXTHREADS 3

using namespace std;

/* Structure that handles the bank information. */
struct bank_data 
{
	int number;
	char name[20];
	int balance;
};
/* Structure that handles that threading. */
struct thread_data
{
	struct bank_data *bank_info;
	pthread_mutex_t lock;
	pthread_cond_t condition;
}

/* Function that handles the withdrawal from a bank account. */
void *withdraw(void *args)
{

}

/* Function that handles the deposits to a bank account. */
void *deposit(void *args)
{

}

/* Main function logic for the server program. */
int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		/* Show error if the correct number of arguments were not passed. */
		fprintf(stderr, "Usage: server <port_numner>\n");
		exit(1);
	}
	
	/* Declare the socket threads. */
	int listen_fd, socket_fd, port_number;
	struct sockaddr_in server_address, client_address;
	/* Declare the thread objects and allocate memory. */
	pthread_t threads[MAXTHREADS];
	struct thread_data *data;
	data = malloc(sizeof(struct thread_data));

	/* Create a stream socket. */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd < 0)
	{
		fprintf(stderr, "Error creating socket.\n");
	}

	bzero((char *)&server_address, sizeof(server_address));

	/* Setup the port and the server address. */
	port_number = atoi(argv[1]);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port_number);

	/* Bind the server address to the listen file descriptor. */
	int b = bind(listen_fd, (struct sockaddr *)&server_address, sizeof(server_address));
	if(b < 0)
	{
		fprintf(stderr, "Error binding socket.\n");
	}

	/* Buffers to store the data received from the client. */
	char buffer[size];
	bzero(buffer, size);

	/* Main loop for the server program. */
	while(1) 
	{
		/* Wait and listen for a request from a client. */
		listen(listen_fd, 5);

		/* Accept a client request. */
		socklet_t client_length = sizeof(client_address);
		socket_fd = accept(listen_fd, (struct sockaddr *)&client_address, &client_length);
		if(socket_fd < 0)
		{
			fprintf(stderr, "Error accepting client request.\n");
		}

		/* Once the request is accepted, read the data from the client. */
		int r = read(socket_fd, &buffer, MAXDATASIZE);
		if(r < 0)
		{
			/* Show error if the data failed to receive. */
			fprintf(stderr, "Error receiving data from client.\n");
		} 
		else 
		{
			/* Perform the transaction if data is received. */
			printf("Received Message: %s\n", buffer);

		}

		close(socket_fd);
	}

	/* Close the listener and end the program. */
	close(listen_fd);
	return 0;
}