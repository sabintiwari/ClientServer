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
7. Records.txt
8. Transactions.txt
9. Makefile
10. README.pdf

Make
----

To build the program, first run (only if there are already compiled files in the directory):
>```make clean```

Then, to create the executables, run:
>```make all```

This will create two excecutable files:
>```server```
>```client```


Run
---

To run the program, first run the server program with the port and the records filename, like so:
>```server 3000 Records.txt```

Once the server is running and waiting for requests, the client can be run in two ways:
1. User interactive terminal program by passing two arguments (address and port):
>```client localhost 3000```

2. Run a batch transactions file by passing three arguments (address, port, and filename):
>```client localhost 3000 transactions.txt```