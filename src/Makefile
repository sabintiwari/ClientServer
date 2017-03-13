#	Sabin Raj Tiwari
#	CMSC 621
#	Project 1


compile: server client copy

server: 
	g++ -o server server.cpp transaction.cpp record.cpp logger.cpp -lpthread

client: 
	g++ -o client client.cpp transaction.cpp logger.cpp

copy:
	cp -r server ../dist/server
	cp -r client ../dist/client
	cp -r ../tests/test_script.sh ../dist/test_script.sh
	cp -r ../tests/Records.txt ../dist/Records.txt
	cp -r ../tests/Transactions.txt ../dist/Transactions.txt

clean: 
	rm -f server client
	rm -f logs/*.*
	rm -f ../dist/server ../dist/client
	rm -f ../dist/test_script.sh
	rm -f ../dist/Records.txt
	rm -f ../dist/Transactions.txt
	rm -f ../dist/logs/*.*