/* demo_server.c - code for example server program that uses TCP */

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define QLEN 6 /* size of request queue */
int visits_part = 0; /* counts client connections */
int visits_obs = 0;
	
void handleConnect(int socket, int* socket2, char* buffer, struct sockaddr_in clientAddress, int addressLength);
int recvString(int sd2, void* string, int size, int flag);
int sendString(int sd2, void* string, int size, int flag);
/*------------------------------------------------------------------------
* Program: prog3_server
*
* Purpose: Accept connections for messaging system
*
* Syntax: ./prog3_server.c participant_port observer_port
*
* participant_port - participant port number to use
* observer_port - observer port number to use
*
*------------------------------------------------------------------------
*/

int main(int argc, char **argv) {
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad_participant; /* structure to hold server's participant address */
	struct sockaddr_in sad_observer; /* structure to hold server's observer address */
	struct sockaddr_in cad_participant; /* structure to hold participant client's address */
	struct sockaddr_in cad_observer; /* structure to hold observer client's address */
	int sd_part_listen, sd_obs_listen, sd_part_send, sd_obs_send; /* socket descriptors */
	int participant_port; /* participant port number */
    int observer_port; /* observer port number */
	int alen_part; /* length of participant address */
    int alen_obs; /* length of observer address */
	int optval = 1; /* boolean value when we set socket option */
	char buf_part[1000]; /* buffer for string the server sends */
    char buf_obs[1000];
    fd_set readfds;
    int maximum_connections = 255;
    int maxfd = -1;
	int observer_fds[255];
	int participant_fds[255];

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server participant_port observer_port\n");
		exit(EXIT_FAILURE);
	}

	memset((char *)&sad_participant,0,sizeof(sad_participant)); /* clear sockaddr structure */
    memset((char *)&sad_observer,0,sizeof(sad_observer)); /* clear sockaddr structure */

	//TODO: Set socket family to AF_INET
	sad_participant.sin_family = AF_INET;
    sad_observer.sin_family = AF_INET;
	
	//TODO: Set local IP address to listen to all IP addresses this server can assume. You can do it by using INADDR_ANY
	sad_participant.sin_addr.s_addr = INADDR_ANY;
    sad_observer.sin_addr.s_addr = INADDR_ANY;
     
	participant_port = atoi(argv[1]); /* convert argument to binary */
	if (participant_port > 0) { /* test for illegal value */
		//TODO: set participant_port number. The data type is u_short
		sad_participant.sin_port = htons(participant_port);
	} else { /* print error message and exit */
		fprintf(stderr,"Error: Bad participant port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}

    observer_port = atoi(argv[2]); /* convert argument to binary */
	if (observer_port > 0) { /* test for illegal value */
		//TODO: set observer_port number. The data type is u_short
		sad_observer.sin_port = htons(observer_port);
	} else { /* print error message and exit */
		fprintf(stderr,"Error: Bad observer port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* TODO: Create a socket with AF_INET as domain, protocol type as SOCK_STREAM, and protocol as ptrp->p_proto. This call returns a socket descriptor named sd. */

	sd_part_listen = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd_part_listen < 0) {
		fprintf(stderr, "Error: Participant socket creation failed\n");
		exit(EXIT_FAILURE);
	}

    sd_obs_listen = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd_obs_listen < 0) {
		fprintf(stderr, "Error: Observer socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Allow reuse of ports - avoid "Bind failed" issues */
	if( setsockopt(sd_part_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting participant socket option failed\n");
		exit(EXIT_FAILURE);
	}

    if( setsockopt(sd_obs_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting observer socket option failed\n");
		exit(EXIT_FAILURE);
	}

	/* Bind a local address to the socket. For this, you need to pass correct parameters to the bind function. */
	if (bind(sd_part_listen, (struct sockaddr*) &sad_participant, sizeof(sad_participant)) < 0) {
		fprintf(stderr,"Error: Bind to participant listen port failed\n");
		exit(EXIT_FAILURE);
	}

    if (bind(sd_obs_listen, (struct sockaddr*) &sad_observer, sizeof(sad_observer)) < 0) {
		fprintf(stderr,"Error: Bind to observer listen port failed\n");
		exit(EXIT_FAILURE);
	}

	/* TODO: Specify size of request queue. Listen take 2 parameters -- socket descriptor and QLEN, which has been set at the top of this code. */
	if (listen(sd_part_listen, QLEN) < 0) {
		fprintf(stderr,"Error: Listen for participants failed\n");
		exit(EXIT_FAILURE);
	}

    if (listen(sd_obs_listen, QLEN) < 0) {
		fprintf(stderr,"Error: Listen for observers failed\n");
		exit(EXIT_FAILURE);
	}

    int status;

	for(int i = 0; i < 255; i++) {
		observer_fds[i] = 0;
		participant_fds[i] = 0;
	}

	/* Main server loop - accept and handle requests */
	while (1) {
		alen_part = sizeof(cad_participant);
		alen_obs = sizeof(cad_observer);

        FD_ZERO(&readfds);

        FD_SET(sd_part_listen, &readfds);
        FD_SET(sd_obs_listen, &readfds);

        if (sd_part_listen > maxfd)
            maxfd = sd_part_listen;
        if (sd_obs_listen > maxfd)
            maxfd = sd_obs_listen;

		// Use select to poll all available file descriptors for read readiness
        status = select(maxfd+1, &readfds, NULL, NULL, NULL);

        if (status < 0) {
            fprintf(stderr, "Error: selecting failed for some reason\n");
            exit(EXIT_FAILURE);
        }
        
        if (FD_ISSET(sd_part_listen, &readfds)) {
            fprintf(stdout, "Contacted participant\n");
			visits_part++;
            
            if ((sd_part_send = accept(sd_part_listen, (struct sockaddr *)&cad_participant, &alen_part)) < 0) {
                fprintf(stderr, "Error: Accept from participant failed\n");
                exit(EXIT_FAILURE);
            }
        }

        if (FD_ISSET(sd_obs_listen, &readfds)) {
            fprintf(stdout, "Contacted observer\n");
        	visits_obs++;
        
            if ((sd_obs_send = accept(sd_obs_listen, (struct sockaddr *)&cad_observer, &alen_obs)) < 0) {
                fprintf(stderr, "Error: Accept from observer failed\n");
                exit(EXIT_FAILURE);
            }
        }
        

		sprintf(buf_part,"This server has been contacted from participants %d time%s\n",visits_part,visits_part==1?".":"s.");
		sprintf(buf_obs,"This server has been contacted from observers %d time%s\n",visits_obs,visits_obs==1?".":"s.");
		
        send(sd_part_send, buf_part, strlen(buf_part),0);
		// close(sd_part_send);
        send(sd_obs_send, buf_obs, strlen(buf_obs),0);
		// close(sd_obs_send);
	}
}

// Handle connection from remote client
void handleConnect(int socket, int* socket2, char* buffer, struct sockaddr_in clientAddress, int addressLength) {
    //addressLength = sizeof(clientAddress);
    if((*socket2 = accept(socket, (struct sockaddr *)&clientAddress, &addressLength)) < 0) {
        fprintf(stderr, "Error: Accept failed\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Connected\n");
}

// Helper function to receive strings
int recvString(int sd2, void* string, int size, int flag) {
	int n;
	int totalBits = size;
	int bytesRead = 0;

	printf("Got to the while received");

	while(totalBits > 0) {
		if((n = recv(sd2, string, totalBits, flag)) <= 0) {
			exit(1);
        }
		bytesRead += n;
		totalBits -= n;
        string += n;
	}
	printf("%d", bytesRead);
	
	return bytesRead;
}

//Helper function to send strings and make sure the proper amount of bits are sent
int sendString(int sd2, void* string, int size, int flag) {
	//printf("Sending %s\n", string);
	int n;
	int totalBits = size;
	int bytesRead = 0;
	

	while(totalBits > 0) {
		if((n = send(sd2, string, totalBits, flag)) <= 0) {
			exit(1);
		}
		bytesRead += n;
		totalBits -= n;
        string += n;
	}
	return 0;
}