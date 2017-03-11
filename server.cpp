#include <cstring>
#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <pthread.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "record.h"
#include "transaction.h"

#define MAXDATASIZE 1024
#define MAXTHREADS 16
#define MAXWAITFORTHREAD 10
#define MAXCONNECTIONS 5

using namespace std;


/* Global variables. */
const char YES = 'Y';
const char NO = 'N';
int used_threads[MAXTHREADS];
std::fstream records_file;
std::vector<Record*> records;


/* Structure that handles the sockets and threading. */
struct socket_data
{
	int listen_fd, socket_fd, port_number;
	struct sockaddr_in server_address, client_address;
	Transaction data;
	pthread_mutex_t lock;
};


/* Get the next available thread or wait for a certain time until returning error. */
int get_available_thread()
{
	int thread = -1;
	int iteration = 0;
	while(thread == -1 && iteration < MAXWAITFORTHREAD)
	{
		for(int i = 0; i < MAXTHREADS; i++)
		{
			if(used_threads[i] == 0)
			{
				return i;
			}
		}
		sleep(1);
		iteration++;
	}
	return thread;
}


/* Function that checks if the account with the id exists and returns the Record. */
Record* get_record_by_id(int id)
{
	for(int i = 0; i < records.size(); i++)
	{
		if(records[i]->account == id)
		{
			return records[i];
		}
	}
	return new Record(-1, NULL, -1);
}


/* Loads all the records from the file in the records vector. */
void load_records(std::string filename)
{
	int i = 0;
	std::string line;
	std::string line_arr[3];
	records_file.open(filename.c_str(), ios::in);
	/* Read all the lines in the file. */
	if(records_file.is_open())
	{
		while(std::getline(records_file, line))
		{
			std::istringstream istr(line);
			/* Get each of the tokens in the stream to the array. */
			while(istr.good() && i < 3)
			{
				istr >> line_arr[i];
				++i;
			}
			
			try 
			{
				/* Convert the types to the required ones for Record. */
				int account = atoi(line_arr[0].c_str());
				int balance = atoi(line_arr[2].c_str());
				std::string name = line_arr[1];
				char *name_char;
				name_char = (char *)malloc(name.size() + 1);
				std::memcpy(name_char, name.c_str(), name.size() + 1);

				/* Add the record to the vector. */
				records.push_back(new Record(account, name_char, balance));
			}
			catch (int e)
			{
				cout << "Records initialization failed. ";
				cout << "Please make sure file has valid data with structure: <account> <name> <balance>\n";
			}			
		}
	}
	else
	{
		cout << "Error! Failed to read from file: " << filename << "\n";
		exit(0);
	}
}


/* Function that handles the withdrawal from a bank account. Return the current balance. -1 if not found. -2 if overdrawn. */
int withdraw(int account, int amount)
{
	Record* rec = get_record_by_id(account);
	if(rec->account != -1)
	{
		if(rec->balance >= amount)
		{
			rec->balance -= amount;
			return rec->balance;
		}
		return -2;
	}
	return -1;
}


/* Function that handles the deposits to a bank account. Returns the current balance. -1 if failure. */
int deposit(int account, int amount)
{
	Record* rec = get_record_by_id(account);
	if(rec->account != -1)
	{
		rec->balance += amount;
		return rec->balance;
	}
	return -1;
}


/* Functions that gets called when a client request comes through. */
void client_request(void *args)
{
	/* Socket data and int values to store responses. */
	int account, w, r;
	struct socket_data *data;
	data = (struct socket_data *) args;

	/* Accept a client request. */
	socklen_t client_length = sizeof(data->client_address);
	data->socket_fd = accept(data->listen_fd, (struct sockaddr *)&(data->client_address), &client_length);
	if(data->socket_fd < 0)
	{
		cerr << "Error accepting client request. Error code: " << data->socket_fd << "\n";
	}

	/* Once the request is accepted, read the data from the client. This should be the account number. */
	r = read(data->socket_fd, &account, sizeof(int));
	if(r < 0)
	{
		/* Show error if the data failed to receive. */
		cerr << "Error receiving data from client Error code: " << r << "\n";
	}
	
	/* Check to see if the provided account number exists. */
	Record* rec = get_record_by_id(account);		

	/* Send the acknowledgement with the account number of the record obtained. -1 if failed. */
	w = write(data->socket_fd, &(rec->account), sizeof(int));
}


/* Main function logic for the server program. */
int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		/* Show error if the correct number of arguments were not passed. */
		cerr << "Usage: server <port_numner> <records_file_name>\n";
		exit(1);
	}

	/* Initialize the file name and call the load records method. */
	std::string filename = argv[2];
	load_records(filename);
	
	/* Declare the socket data. */
	struct socket_data *data;
	data = (struct socket_data*) malloc(sizeof(struct socket_data));

	/* Declare the thread objects and allocate memory. */
	pthread_t threads[MAXTHREADS];
	
	/* Create a stream socket. */
	data->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(data->listen_fd < 0)
	{
		cerr << "Error creating socket. Error code: " << data->listen_fd << "\n";
		exit(0);
	}

	memset((char *)&(data->server_address), 0, sizeof(data->server_address));

	/* Setup the port and the server address. */
	data->port_number = atoi(argv[1]);
	data->server_address.sin_family = AF_INET;
	data->server_address.sin_addr.s_addr = INADDR_ANY;
	data->server_address.sin_port = htons(data->port_number);

	/* Bind the server address to the listen file descriptor. */
	int b = bind(data->listen_fd, (struct sockaddr *)&(data->server_address), sizeof(data->server_address));
	if(b < 0)
	{
		cerr << "Error binding socket. Error code: " << b << "\n";
		exit(0);
	}

	/* Buffers and variables to store the response and data received from the client. */
	std::string last_message;

	/* Main loop for the server program. */
	while(1) 
	{
		/* Wait and listen for a request from a client. */
		cout << "Waiting for client request...\n";
		listen(data->listen_fd, 5);

		/* Wait for an available thread and call the client request function. */
		int thread_index = get_available_thread();
		if(thread_index > -1)
		{
			pthread_create(&threads[thread_index], NULL, client_request, (void*) data);
			pthread_join(&threads[thread_index], NULL);
		}

		/* Close the socket once the request is complete. */
		close(data->socket_fd);
	}

	/* Close the listener and end the program. */
	close(data->listen_fd);
	return 0;
}