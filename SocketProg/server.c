/* 
	Owner: Mariette Souppe
	Class: CMPE 156 Network Programming
	Assignment 1
	File: cmpe156/assign1/server.c 
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
#define	MAXLINE		4096	/* Max text line length */
#define LISTENQ		1024
#define PORT_NO		4000	/* Defining port number */

/* Error message function for debugging purposes */
void error(char *msg) {
    printf(msg);
	exit(1);
}

int main(int argc, char **argv) {
	int					listenfd, connfd, n;
	struct sockaddr_in	servaddr;
	char				line[MAXLINE];			/* Buffer for contents from each line in the file*/
	char				printAll[MAXLINE];		/* Buffer for all contents from entire file*/
	char				recvline[MAXLINE + 1];	/* Buffer for input from client */
	FILE 				*file;					/* Initialize file */	

	/* Makes socket */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	/* Send out error message if error on socket */
	if ( listenfd < 0) {
		error("Socket error\n");
    }
	
	/* Write zero-valued bytes to servaddr */
	bzero(&servaddr, sizeof(servaddr));
	
	servaddr.sin_family      = AF_INET;				/* Defines protocol*/
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	/* Defines socket type _ IPv4*/
	servaddr.sin_port        = htons(4568);			/* Choosing port number */

	/* Bind name on a socket */
	int b = bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	//printf("%d\n", b);
	
	/* Listen for connection on socket */
	int l = listen(listenfd, LISTENQ);	
	//printf("%d\n", l);
	
	bzero(recvline, MAXLINE+1);
	bzero(line, MAXLINE+1);
	bzero(printAll, MAXLINE+1);
	
	/* Accept a connection on a socket */
		connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
		
	/* Send out error message if error on accept */
	if (connfd < 0) {
		error("ERROR on accept");
	}
	
	for ( ; ; ) {
		
		n = read(connfd, recvline, MAXLINE);
		/* Reading data from the client until condition isn't met */
		//while (() > 0) {
			//recvline[n] = 0;				/* Null terminate character */
			file = popen(recvline, "r");	/* Open the file of command*/
			
			if (strcmp(recvline, "exit\n") == 0) {
				shutdown(connfd, 1);
				return 0;
			}
			
			/* Goes through every line in the file until no more lines in the file */
			while(fgets(line, sizeof(line)-1, file) != NULL) {
				
				/* Concatenates all of the lines in the file and puts then into one file */
				strcat(printAll, line);
			}
			
			/* Writes data to the client */
			n = write(connfd, printAll, strlen(printAll));
			
			/* Close the file */
			pclose(file);
			
			printf("Message from client: %s\n", recvline);
			
			bzero(recvline, MAXLINE+1);
			bzero(line, MAXLINE+1);
			bzero(printAll, MAXLINE+1);			
	
		//} 
		
	}
	
	/* Close the connection with the client */
	close(connfd);
}
