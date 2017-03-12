/*
	Sabin Raj Tiwari
	CMSC 621
	Project 1
*/

#define _BSD_SOURCE
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
#define MAX_THREADS 16
#define MAX_WAIT_FOR_THREAD 10
#define MAX_CONNECTIONS 100
#define INTEREST_RATE 5.0
#define INTEREST_INTERVAL 30

using namespace std;


/* Global variables. */
const char YES = 'Y';
const char NO = 'N';
int used_threads[MAX_THREADS];
int interest_transactions = 1;
std::fstream records_file;
std::vector<Record*> records;
Logger* logger = new Logger("./logs/_server_log_file.txt");
Record* null_record = new Record(-1, "", -1);


/* Structure that handles the sockets and threading. */
struct socket_data
{
	int listen_fd, socket_fd, port_number, is_file_locked;
	struct sockaddr_in server_address, client_address;
	pthread_mutex_t record_lock;
	pthread_mutex_t file_lock;
	pthread_cond_t file_cond;
	std::string filename;
};


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


/* Get the next available thread or wait for a certain time until returning error. */
int get_available_thread()
{
	int thread = -1;
	int iteration = 0;
	while(thread == -1 && iteration < MAX_WAIT_FOR_THREAD)
	{
		for(int i = 0; i < MAX_THREADS; i++)
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


/* Checks if the account with the id exists and returns the Record. */
Record* get_record_by_id(int id)
{
	for(int i = 0; i < records.size(); i++)
	{
		if(records[i]->account == id)
		{
			return records[i];
		}
	}
	return null_record;
}


/* Saves the records to the file and returns 1 if success full. */
int save_records(struct socket_data *data)
{
	int i = 0;
	std::string line;
	std::string message;

	/* Create the mutex lock point. */
	pthread_mutex_lock(&(data->file_lock));
	
	while(data->is_file_locked == 1)
	{
		message = "Records file is currently locked. Request is waiting.";
		logger->log(message);
		/* Wait for the lock to release if there is one. */
		pthread_cond_wait(&(data->file_cond), &(data->file_lock));
	} 

	records_file.open(data->filename.c_str(), ios::out | ios::trunc);
	data->is_file_locked = 1;

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


		/* Unlock the record after writing */
		data->is_file_locked = 0;
		pthread_cond_signal(&(data->file_cond));
		pthread_mutex_unlock(&(data->file_lock));

		return 1;
	}
	
	return 0;
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
double perform_transaction(Record* record, Transaction* transaction, struct socket_data *data)
{
	double response = -1.0;
	std::string message;

	/* If the account was found. */
	if(record->account != -1)
	{	
		/* Create the mutex lock point. */
		pthread_mutex_lock(&(data->record_lock));
		
		while(record->is_locked == 1)
		{
			message = "Account number " + i_to_s(record->account) + " is currently locked. Request is waiting.";
			logger->log(message);
			/* Wait for the lock to release if there is one. */
			pthread_cond_wait(&(record->in_use), &(data->record_lock));
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
		save_records(data);

		/* Unlock the record after writing */
		record->is_locked = 0;
		pthread_cond_signal(&(record->in_use));
		pthread_mutex_unlock(&(data->record_lock));
	}

	return response;
}


/* Accumulates interest in intervals. */
void *accumulate_interest(void *args)
{
	double response;
	std::string message;
	/* Get the stuct data with the socket information. */
	struct socket_data *data;
	data = (struct socket_data *) args;

	int interval = 0;
	while(1)
	{
		/* Increment the count for the interval and sleep for a second. */
		interval++;
		sleep(1);

		/* If the interval timeout is reached. Call the perform transaction for each record. */
		if(interval >= INTEREST_INTERVAL)
		{
			for(int i = 0; i < records.size(); i++)
			{
				/* Create the interface amount and perform the transaciton. */
				float amount = records[i]->balance * (INTEREST_RATE / 100.00);
				Transaction* transaction = new Transaction(++interest_transactions, records[i]->account, "i", amount);
				response = perform_transaction(records[i], transaction, data);

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
			interval = 0;
		}
	}
}


/* Handle the client request and perform the transaction. Send response to the client with success or failure. */
void *client_request(void *args)
{
	/* Socket data and int values to store responses. */
	double response, w, r;
	char buffer[MAX_DATASIZE];
	std::string client_str;
	
	
	struct socket_data *data;
	data = (struct socket_data *) args;

	/* Accept a client request. */
	socklen_t client_length = sizeof(data->client_address);
	data->socket_fd = accept(data->listen_fd, (struct sockaddr *)&(data->client_address), &client_length);
	if(data->socket_fd < 0)
	{
		logger->log("Error accepting client request.");
		return (void *)-1;
	}

	/* Create the string form of the client address. */
	client_str = inet_ntoa(data->client_address.sin_addr);

	/* Read in the transaction from the client. */
	memset(&buffer[0], 0, MAX_DATASIZE);
	r = read(data->socket_fd, &buffer, MAX_DATASIZE);
	if(r < 0)
	{
		/* Show error if the data failed to receive. */
		logger->log("Failed to receive data from client.");
		return (void *)-1;
	}

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
		response = perform_transaction(record, transaction, data);

		/* Write the aknowledgement for successful write. */
		w = write(data->socket_fd, &response, sizeof(double));
		if(w < 0)
		{
			logger->log("Error writing data to client.");
			return (void *)-1;
		}

		/* Log the data being sent to the client. */
		message = "Data sent. Client: " + client_str + " - Data: " + m_to_s(response);
		logger->log(message);
		logger->log("\n");
	}
	else
	{
		logger->log("Error! Transaction data is not valid.");
	}
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

	/* Declare the socket data. */
	struct socket_data *data;
	data = (struct socket_data*) malloc(sizeof(struct socket_data));

	/* Initialize the file name and call the load records method. */
	data->filename = argv[2];
	data->is_file_locked = 0;
	load_records(data->filename);

	/* Declare the thread objects and allocate memory. */
	pthread_t threads[MAX_THREADS];
	pthread_t interest_thread;

	/* Kick off the thread and handles the interest accumulation. */
	pthread_create(&interest_thread, NULL, accumulate_interest, (void*) data);
	
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

	/* Main loop for the server program. */
	while(1) 
	{
		/* Wait and listen for a request from a client. */
		listen(data->listen_fd, MAX_CONNECTIONS);

		/* Wait for an available thread and call the client request function. */
		int thread_index = get_available_thread();
		used_threads[thread_index] = 1;
		if(thread_index > -1)
		{
			pthread_create(&threads[thread_index], NULL, client_request, (void*) data);
			pthread_join(threads[thread_index], NULL);
			used_threads[thread_index] = 0;

			/* Close the socket once the request is complete. */
			close(data->socket_fd);
		}
	}

	/* Close the listener and end the program. */
	pthread_join(interest_thread, NULL);
	close(data->listen_fd);
	logger->close();
	return 0;
}