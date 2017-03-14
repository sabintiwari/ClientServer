#	Sabin Raj Tiwari
#	CMSC 621
#	Project 1


compile: server client directories

compile_test: server client copy

clean: 
	rm -f server client
	rm -rf logs

clean_test:
	rm -f server client
	rm -f logs/*.*
	rm -f ./test_script.sh
	rm -f ./Records.txt
	rm -f ./Transactions.txt

server: 
	g++ -o server server.cpp transaction.cpp record.cpp logger.cpp -lpthread

client: 
	g++ -o client client.cpp transaction.cpp logger.cpp

directories:
	mkdir -p logs

copy:
	cp -r ../tests/test_script.sh ./test_script.sh
	cp -r ../tests/Records.txt ./Records.txt
	cp -r ../tests/Transactions.txt ./Transactions.txt