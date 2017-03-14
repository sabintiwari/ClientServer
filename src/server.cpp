/*
	Sabin Raj Tiwari
	CMSC 621
	Project 1
*/

#include <arpa/inet.h>
#include <cstring>
#include <ctime>
#include <iomanip>
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

#include "logger.h"
#include "record.h"
#include "transaction.h"

#define MAX_DATASIZE 1024
#define MAX_THREADS 100
#define MAX_WAIT_FOR_THREAD 10
#define MAX_CONNECTIONS 100
#define INTEREST_RATE 5.0
#define INTEREST_INTERVAL 30

using namespace std;

/* Structure that handles the socket data for the server. */
struct socket_data
{
	int listen_fd, port_number, socket_fd, is_file_locked;
	struct sockaddr_in server_address, client_address;
	
};

/* Structure that handles the sockets data for the client. */
struct client_data
{
	int socket_fd;
	struct sockaddr_in client_address;
};

/* Structure that handles the locking and mutex for reading the records and writing to the file. Also has data for clients. */
struct thread_data
{
	int is_records_locked, is_file_locked;
	std::string filename;
	pthread_t threads[MAX_THREADS];
	pthread_mutex_t records_lock;
	pthread_mutex_t file_lock;
	pthread_cond_t records_cond;
	pthread_cond_t file_cond;
};


/* Global variables. */
const char YES = 'Y';
const char NO = 'N';

int used_threads = 0;
int interest_transactions = 1;

struct thread_data *threads;
std::fstream records_file;
std::vector<Record*> records;
Logger* logger = new Logger("./logs/_server_log_file.txt");
Record* null_record = new Record(-1, "", -1);


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


/* Checks if the account with the id exists and returns the Record. */
Record* get_record_by_id(int id)
{
	/* Lock the records once a thread tries to access it. */
	Record* record = null_record;
	pthread_mutex_lock(&(threads->records_lock));
	std::string message;

	while(threads->is_records_locked == 1)
	{
		message = "Records file is currently locked. Request is waiting.";
		logger->log(message);
		/* Wait for the lock to release if there is one. */
		pthread_cond_wait(&(threads->records_cond), &(threads->records_lock));
	}

	threads->is_records_locked = 1;

	/* Perform the check. */
	for(int i = 0; i < records.size(); i++)
	{
		if(records[i]->account == id)
		{
			record = records[i];
			break;
		}
	}

	/* Remove the lock. */
	threads->is_records_locked = 0;
	pthread_cond_signal(&(threads->records_cond));
	pthread_mutex_unlock(&(threads->records_lock));

	return record;
}


/* Saves the records to the file and returns 1 if success full. */
int save_records()
{
	int i = 0;
	int response = 0;
	std::string line;
	std::string message;

	/* Create the mutex lock point. */
	pthread_mutex_lock(&(threads->file_lock));
	
	if(threads->is_file_locked == 1)
	{
		message = "Records file is currently locked. Request is waiting.";
		logger->log(message);
		/* Wait for the lock to release if there is one. */
		pthread_cond_wait(&(threads->file_cond), &(threads->file_lock));
	} 

	records_file.open(threads->filename.c_str(), ios::out | ios::trunc);
	threads->is_file_locked = 1;

	/* Write all the Records to the file */
	if(records_file.is_open())
	{
		for(int i = 0; i < records.size(); i++)
		{
			line = i_to_s(records[i]->account) + " " + records[i]->name + " " + m_to_s(records[i]->balance);
			records_file << line << endl;
		}

		/* Close the file */
		records_file.close();
		response = 1;
	}
	
	/* Unlock the record after writing */
	threads->is_file_locked = 0;
	pthread_cond_signal(&(threads->file_cond));
	pthread_mutex_unlock(&(threads->file_lock));

	return response;
}


/* Loads all the records from the file in the records vector. */
void load_records(std::string filename)
{
	int i = 0;
	std::string line;
	records_file.open(filename.c_str(), ios::in);
	
	/* Read all the lines in the file. */
	if(records_file.is_open())
	{
		/* Read the data from the file and add to the records. */
		while(std::getline(records_file, line))
		{
			Record* rec = new Record(line);
			if(rec->account > -1)
			{
				records.push_back(rec);
			}
			else
			{
				logger->log("Error! Invalid data in file: " + filename);
			}
		}

		/* Close the file. */
		records_file.close();
	}
	else
	{
		logger->log("Error! Failed to read from file: " + filename);
		exit(0);
	}
}


/* Writes data for the transaction. Handles locking as well. */
double perform_transaction(Record* record, Transaction* transaction)
{
	double response = -1.0;
	std::string message;

	/* If the account was found. */
	if(record->account != -1)
	{	
		/* Create the mutex lock point. */
		pthread_mutex_lock(&(record->lock));

		if(record->is_locked == 1)
		{
			message = "Account number " + i_to_s(record->account) + " is currently locked. Request is waiting.";
			logger->log(message);
			/* Wait for the lock to release if there is one. */
			pthread_cond_wait(&(record->cond), &(record->lock));
		} 

		/* If record is not locked, lock it and perform the operation. */
		record->is_locked = 1;

		if(transaction->type == "w")
		{
			/* Withdraw if the type is w */
			if(record->balance >= transaction->amount)
			{
				record->balance -= transaction->amount;
				response = record->balance;
			}
			else
			{
				response = -2.0;
			}
		}
		else
		{
			/* Deposit if the type is d or i*/
			record->balance += transaction->amount;
			response = record->balance;
		}

		/* Save the file. */
		save_records();

		/* Unlock the record after writing */
		record->is_locked = 0;
		pthread_cond_signal(&(record->cond));
		pthread_mutex_unlock(&(record->lock));
	}

	return response;
}


/* Accumulates interest in intervals. */
void *accumulate_interest(void *args)
{
	/* Variables that store responses and messages. */
	double response;
	std::string message;

	while(1)
	{
		/* Sleep for the interval count. */
		sleep(INTEREST_INTERVAL);

		for(int i = 0; i < records.size(); i++)
		{
			/* Create the interface amount and perform the transaciton. */
			float amount = records[i]->balance * (INTEREST_RATE / 100.00);
			Transaction* transaction = new Transaction(++interest_transactions, records[i]->account, "i", amount);
			response = perform_transaction(records[i], transaction);

			/* Create and log the message based on the response. */
			message = "Interest transaction for account number " + i_to_s(records[i]->account);
			if(response > -1)
			{
				message += " complete successfully. Resulting balance: $" + m_to_s(response);
			}
			else if(response == -1)
			{
				message += " failed to complete. Account not found.";
			}
			else if(response == -2)
			{
				message += " failed to complete. Insufficient funds.";
			}

			/* Log the message to the server file. */
			logger->log(message);
		}

		logger->log("\n");
	}
}


/* Handle the client request and perform the transaction. Send response to the client with success or failure. */
void *client_request(void *args)
{
	/* Socket data and int values to store responses. */
	double response;
	int socket_fd, w, r;
	char buffer[MAX_DATASIZE];
	std::string buffer_str;
	std::string client_str;
	
	/* Get the socket data. */
	struct client_data *data;
	data = (struct client_data *) args;
	int end = 0;

	/* Create the string form of the client address. */
	client_str = inet_ntoa(data->client_address.sin_addr);

	/* Run the read write loop until the client sends a finish request. */
	while (end == 0)
	{
		/* Read in the transaction from the client. */
		memset(&buffer[0], 0, MAX_DATASIZE);
		r = read(data->socket_fd, &buffer, MAX_DATASIZE);
		if(r < 0)
		{
			/* Show error if the data failed to receive. */
			logger->log("Failed to receive data from client.");
		}

		/* Check if the signal was for end connection. */
		buffer[r] = '\0';
		buffer_str = buffer;
		if(buffer_str == "" || buffer_str == "finish")
		{
			/* End the loop if yes. */
			end = 1;
		}
		else
		{
			/* Check if the transaction is valid and perform it. */
			Transaction* transaction = new Transaction(buffer);
			if(transaction->is_valid())
			{
				/* Log the transaction data. */
				std::string message(buffer);
				message = "Data received. Client: " + client_str + " - Data: " + buffer;
				logger->log(message);

				/* Get the record with the id provided. */
				Record* record = get_record_by_id(transaction->account);

				/* Perform the transaction. */
				response = perform_transaction(record, transaction);

				/* Write the aknowledgement for successful write. */
				w = write(data->socket_fd, &response, sizeof(double));
				if(w < 0)
				{
					logger->log("Error writing data to client.");
				}
				else
				{
					/* Log the data being sent to the client. */
					message = "Data sent. Client: " + client_str + " - Data: " + m_to_s(response);
					logger->log(message);
					logger->log("\n");
				}	
			}
			else
			{
				/* Log that the transaction was not valid. */
				logger->log("Error! Transaction data is not valid: " + buffer_str);
			}
		}
	} 

	/* Close the socket once the request is complete. */
	close(data->socket_fd);
	free(data);
	used_threads--;
	logger->log("Used threads (-): " + i_to_s(used_threads));
}


/* Wait for connection to come in from the client. */
void *wait_for_connection(void *args)
{
	struct socket_data *data;
	data = (struct socket_data *) args;

	/* Main loop for the server program. */
	while(1) 
	{
		struct client_data *client;
		client = (struct client_data*) malloc(sizeof(struct client_data));

		/* Accept a client request. */
		socklen_t client_length = sizeof(client->client_address);
		client->socket_fd = accept(data->listen_fd, (struct sockaddr *)&(client->client_address), &client_length);
		if(data->socket_fd < 0)
		{
			logger->log("Error accepting client request.");
			return (void *)-1;
		}

		/* Create a thread call the client request function. */
		pthread_create(&threads->threads[used_threads], NULL, client_request, (void*) client);
		used_threads++;
		logger->log("Used threads (+): " + i_to_s(used_threads));
	}
}


/* Main function logic for the server program. */
int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		/* Show error if the correct number of arguments were not passed. */
		cerr << "Usage: server <port_number> <records_file_name>\n";
		exit(1);
	}

	/* Declare the socket data. */
	struct socket_data *data;
	data = (struct socket_data*) malloc(sizeof(struct socket_data));

	/* Declare the thread objects and allocate memory. */
	pthread_t interest_thread; // calculates interest in a fixed interval
	pthread_t wait_thread; // waits for connections to come in from the client

	/* Initialize the threads mutex and lock and initialize the file name and call the load records method. */
	threads = (struct thread_data*) malloc(sizeof(struct thread_data));
	threads->filename = argv[2];
	threads->is_records_locked = 0;
	threads->is_file_locked = 0;
	pthread_cond_init(&(threads->records_cond), NULL);
	pthread_cond_init(&(threads->file_cond), NULL);
	pthread_mutex_init(&(threads->records_lock), NULL);
	pthread_mutex_init(&(threads->file_lock), NULL);
	load_records(threads->filename);

	/* Kick off the thread and handles the interest accumulation. */
	pthread_create(&interest_thread, NULL, accumulate_interest, NULL);
	
	/* Create a stream socket. */
	data->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(data->listen_fd < 0)
	{
		logger->log("Error creating socket.");
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
		logger->log("Error binding socket.");
		exit(0);
	}

	/* Log the message that the server is initialized. */
	cout << "\nSee logs/_server_log_file.txt for the log file output of the server.\n";
	logger->log("Server has been started. Listening for clients...\n");

	/* Wait and listen for a request from a client. */
	listen(data->listen_fd, MAX_CONNECTIONS);

	/* Kick of the connection thread. */
	pthread_create(&wait_thread, NULL, wait_for_connection, (void*) data);

	/* Close the listener and end the program. */
	pthread_join(interest_thread, NULL);
	pthread_join(wait_thread, NULL);
	close(data->listen_fd);
	logger->close();
	return 0;
}
