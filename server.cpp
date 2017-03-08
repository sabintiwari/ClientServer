#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

using namespace std;


int main(int argc, char *argv[])
{
	int listen_fd, socket_fd, port_number;
	struct sockaddr_in server_address, client_address;

	if(argc < 2)
	{
		/* Show error if the correct number of arguments were not passed. */
		fprintf(stderr, "Usage: server <port_numner>\n");
		exit(1);
	}

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

	/* Main loop for the server program. */
	while(1) 
	{
		listen(listen_fd, 5);

		/* Accept a client request. */
		socklet_t client_length = sizeof(client_address);
		socket_fd = accept(listen_fd, (struct sockaddr *)&client_address, &client_length);
		if(socket_fd < 0)
		{
			fprintf(stderr, "Error accepting client request.\n");
		}

		close(socket_fd);
	}

	/* Close the listener and end the program. */
	close(listen_fd);
	return 0;
}