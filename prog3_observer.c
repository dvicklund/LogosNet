/* demo_client.c - code for example client program that uses TCP */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*------------------------------------------------------------------------
* Program: prog3_observer
*
* Purpose: allocate a socket, connect to a server, and print all output
*
* Syntax: ./prog3_observer server_address observer_port
*
* server_address - name of a computer on which server is executing
* observer_port  - observer port number server is using
*
*------------------------------------------------------------------------
*/
int main( int argc, char **argv) {
	struct hostent *ptrh; /* pointer to a host table entry */
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold an IP address */
	int sd; /* socket descriptor */
	int port; /* protocol port number */
	char *host; /* pointer to host name */
	int n; /* number of characters read */
	char buf[1000]; /* buffer for data from the server */
	char sendBuf[50]; /* buffer for data to send to the server */
	int ready;
	fd_set fdSet;

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./client server_address server_port\n");
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[2]); /* convert to binary */
	if (port > 0) /* test for legal value */
		sad.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"Error: bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	host = argv[1]; /* if host argument specified */

	/* Convert host name to equivalent IP address and copy to sad. */
	ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		fprintf(stderr,"Error: Invalid host: %s\n", host);
		exit(EXIT_FAILURE);
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Map TCP transport protocol name to protocol number. */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket. */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* TODO: Connect the socket to the specified server. You have to pass correct parameters to the connect function.*/
	if (connect(sd, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		fprintf(stderr,"connect failed\n");
		exit(EXIT_FAILURE);
	}

	printf("Type '/exit' to quit\n");

	/* Repeatedly read data from socket and write to user's screen. */
	while (1) {
		n = recv(sd, buf, sizeof(buf), 0);
		// n = recv(sd, buf, sizeof(buf), 0);
		write(1,buf,n);
		printf("\n");
		
		ready = select(1, &fdSet, NULL, NULL, NULL);

		if (ready) {
			scanf("%s", sendBuf);
			if(strcmp(sendBuf, "/exit") == 0) {
				close(sd);
				exit(EXIT_SUCCESS);
			} else {
				fflush(STDIN_FILENO);
				// fflush(STDOUT_FILENO);
			}
		} 
	}



	// close(sd);

	// exit(EXIT_SUCCESS);
}

// Helper function to receive a string
int recvString(int sd2, void* string, int size, int flag) {
	
	int n;
	int totalBits = size;
	int bytesRead = 0;
	//printf("Start Recieve\n");
	while(totalBits > 0) {
		//printf("Start loop");
		if((n = recv(sd2, string, totalBits, flag)) < 0) {
			printf("%i\n", n);
			perror("recv()");
			exit(1);
        }
		else if(n == 0) {
			close(sd2);
			break;
		}
		//printf("Read %u bits\n", n);
		bytesRead += n;
		totalBits -= n;
		//printf("Totalbits = %u\n", totalBits);
        string += n;
	}
	//printf("%p", string);
	//printf("Successful Receive\n");
	return 0;
}

//Helper function to send strings and make sure the proper amount of bits are sent
int sendString(int sd2, void* string, int size, int flag) {
	
	int n;
	int totalBits = size;
	int bytesRead = 0;
	while(totalBits > 0) {
		if((n = send(sd2, string, totalBits, flag)) < 0) {
			exit(1);
		}
		else if(n == 0) {
			close(sd2);
			break;
		}
		bytesRead += n;
		totalBits -= n;
        string += n;
	}
	return 0;
}
