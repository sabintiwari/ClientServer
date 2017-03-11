all: compile
compile: server client
	server: g++ -o server server.cpp transaction.cpp
	client: g++ -o client client.cpp
clean: rm structures server client