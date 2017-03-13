#	Sabin Raj Tiwari
#	CMSC 621
#	Project 1


compile:server client

server: 
	g++ -o server server.cpp transaction.cpp record.cpp logger.cpp -lpthread
	cp -r server ../dist/server

client: 
	g++ -o client client.cpp transaction.cpp logger.cpp
	cp -r client ../dist/client

clean: 
	rm server client
	rm -f logs/*.*
	rm ../dist/server ../dist/client