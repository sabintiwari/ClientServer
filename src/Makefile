#	Sabin Raj Tiwari
#	CMSC 621
#	Project 1


compile:server client

server: 
	g++ -o server server.cpp transaction.cpp record.cpp logger.cpp -lpthread

client: 
	g++ -o client client.cpp transaction.cpp logger.cpp

clean: 
	rm server client
	rm ../logs/*.*