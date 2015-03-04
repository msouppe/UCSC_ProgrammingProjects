Owner: Mariette Souppe
Class: CMPE 156 Computer Networking
Assignment 1
January 23, 2015

How to compile program:
Open two different sessions, one for the client and the server
make clean
make
Start the server side and enter the command "server"
Start the client side and enter the command "client 127.0.0.1"
[ A connection will be established and type in commands ]

File functionality:
server.c
	The purpose of this file initializes and sets up a connection 
	to connect with the with another host, in our case the client.
	When a connection has been established 
	
client.c
	This file is a simple program that makes a connection to another
	host, the server, and read and write data to the host until it
	decides to shut down. When the client shuts down the server is still
	running to make another connections with other hosts.
	
Makefile
	The Makefile creates the executable files and object files that are needed
	to make the program work. Additionally, the Makefile cleans the object
	files after their has been changes to the source file and the program
	is re-executed for testing from new changes on the source file.

design.txt
	The design file describes the purpose of the assignment, goes into detail about
	the data that's used in the program, the operations for the different functions
	that are used in the program, and the algorithm used on the client and server 
	parts of the program.
	
README.txt
	The README file tells a user how to compile the program, the purpose of 
	each file, and any experience that the program had during their process
	of completing their task.

Experience:
	Even though this program was probably the easiest program that we will
	have to program through out quarter, I thought it was okay. I should
	have started earlier and planned better. A lesson I should already know
	by now. One of the main things that was helpful that came to debugging the
	code was putting error and print statements everyone to make sure things were
	connecting/ binding/ accepting/ listening/ etc. I enjoyed this program because
	you get to learn some of the fundamentals between a client and server and how 
	the two communicate with one another. Even though it was only a simple program, 
	knowing and learning the skeleton makes me start to understand other applications
	where these are used. 