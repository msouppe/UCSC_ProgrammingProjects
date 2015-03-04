/* 
	Owner: Mariette Souppe
	Class: CMPE 156 Network Programming
	Assignment 1
	File: cmpe156/assign1/client.c 
*/

// Standard libraries
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>

// Socket libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

/* Miscellaneous constants */
#define	MAXLINE		4096	/* max text line length */
#define	BUFFSIZE	8192	/* buffer size for reads and writes */
#define PORT_NO		4000	/* Defining port number */

/* Error message function for debugging purposes */
void error(char *msg) {
    printf(msg);
	exit(1);
}

int main(int argc, char **argv) {
	int					sockfd, n;
	char				recvline[MAXLINE + 1];
	struct sockaddr_in	servaddr;

	/* Make sure that the executable name and IP address are 
	   the only two args in command line */
	if (argc != 2) {
		error("usage: a.out <IPaddress>\n");
	}
	
	/* Send out error message if error on socket */
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error("Socket error\n");
    }    
	
	/* Write zero-valued bytes to servaddr */
	bzero(&servaddr, sizeof(servaddr));
	//memset(&servaddr, 0, sizeof(servaddr));
	
	servaddr.sin_family = AF_INET;			/* Defines protocol*/
	servaddr.sin_port   = htons(4568);		/* Choosing port number */
	
	int i = inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	//printf("%d\n", i);
	if ( i <= 0) {
		error("Inet_pton error\n");
	}
	
	/* Sends out error message if can't make connection on socket */
	int c = connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	//printf("%d\n", c);
	if ( c < 0) {
		error("Connect error\n");
	} 
	
	while (1) {
		
		/* Client enters message to be sent to the server */
        printf("Please enter a command: ");

		/* Clears the buffer */
        bzero(recvline, MAXLINE);
        
		/* Reads line from stream */
        fgets(recvline, MAXLINE+1, stdin);

		/* Sends fgets to server */
        n = write(sockfd, recvline, strlen(recvline));

		/* Prints an error message when socket can not be written */
        if (n < 0) {
            error("ERROR writing to socket");
        }

		/* Shut-down client side */
        if (strcmp(recvline, "exit\n") == 0) {
            char msg[] = "Client side shutting down.";
            n = write(sockfd, msg, sizeof (msg));

			/* Closes connection with server */
            close(sockfd);
 
            exit(1);
          }

		/* Clears out the buffer */
        bzero(recvline, MAXLINE+1);

		/* Reads the system documentation from the server */
        while (recvline[0] == '\0' && recvline[0] != EOF) {
            n = read(sockfd, recvline, MAXLINE);
        }

		/* Prints an error message when the socket can not be read */
        if (n < 0) {
            error("ERROR reading from socket");
        }

        printf("%s\n", recvline);
        // Prints out message that the client typed in 
    } 
	
	exit(0);
}