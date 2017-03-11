all:compile
compile:server client
server: 
	g++ -o server server.cpp transaction.cpp record.cpp
client: 
	g++ -o client client.cpp transaction.cpp
clean: 
	rm server client