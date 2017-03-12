# Bank Service

## Sabin Raj Tiwari
### CMSC 621 Project 1

Files:
-------------------------
The zip file contains the following files.
1. server.cpp
2. server (excecutable file)
3. client.cpp
4. client (excecutable file)
5. transactions.cpp
6. transaction.h
7. records.cpp
8. records.h
9. logger.cpp
10. logger.h
11. Records.txt
12. Transactions.txt
13. Makefile
14. README.pdf

Make
----

To build the program, first run (only if there are already compiled files in the directory):
>```make clean```

Then, to create the executables, run:
>```make compile```

This will create two excecutable files:
>```server```
>```client```


Run
---

To run the program, first run the server program with the port and the records filename, like so:
>```server 3000 Records.txt```

Once the server is running and waiting for requests, run the client by passing three arguments (address, port, and filename):
>```client localhost 3000 Transactions.txt```
