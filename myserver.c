/* server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "connection.h"
#include "types.h"

#define PORT "1400"  // the port users will be connecting to

void sigchld_handler()
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
int main(int argc, char *argv[])
{
	int sockfd, new_fd[CNT];  // listen on sock_fd, new connection on new_fd
	int cnt=0;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	char *buf = NULL;

	if (argc != 2) {
		fprintf(stderr, "usage: server <portnum> \n");
		exit(1);
	}

	
	// set  the socket  parameters
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// create and bind to first socket we can
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	// if we couldn't bind to any sockets then quit with error
	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure


	// we just wait here listenting for an incoming cclient
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while (cnt < 2) {
		sin_size = sizeof their_addr;
		new_fd[cnt] = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd[cnt] == -1) {
			perror("accept");
			exit(1);;
		}

		inet_ntop(their_addr.ss_family,
		get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		char greeting[15];
		if (send(new_fd[cnt], "Hello, world!", 13, 0) == -1) 
				perror("send");
		if (recv(new_fd[cnt], greeting, 14, 0) == -1) {
			perror("recv failed\n");
		}
		greeting[14] = '\0';
		fprintf(stderr,"%s\n", greeting);

		cnt++;
	}

	int finished = 0;
	int w=WINDOWWIDTH, h=WINDOWHEIGHT;
	char * mystate = (char *) malloc(sizeof(STATE));
	
	while(!finished) {
		int result = 0;
		// First, get the state from client
		// Next, send the state to the server
		if ((result=recvdata(new_fd[0], (char*) mystate, sizeof(STATE), 0)) != -1) {
			fprintf(stderr,"client1 sent %d bytes of data to relay server\n", result);
			h = ntohl(((STATE *) mystate)->winheight);
			w = ntohl(((STATE *) mystate)->winwidth);
			fprintf(stderr,"w=%d, h=%d\n", w, h);
	
			if ((result=senddata(new_fd[1], (char *) mystate, sizeof(STATE), 0)) == -1) 
				perror("send");
			fprintf(stderr,"relayserver sent %d bytes of data to client2\n", result);
		}
		else {
			perror("recv");
		}
		// after server has processed the state it generates an image
		// receive the image from the server and send to the client
		int size = ELEMENTSPERPIXEL * w * h * sizeof(unsigned char);
		unsigned char * image = (unsigned char *) malloc(size);
		if ((result=recvdata(new_fd[1], (unsigned char*) image, size, 0)) != -1) {
			fprintf(stderr,"client2 sent %d bytes of data to relay server\n", result);
	
			if ((result=senddata(new_fd[0], (unsigned char *) image, size, 0)) == -1) 
				perror("send");
				fprintf(stderr,"relay server  sent %d bytes of data to client1\n", result);
		}
		else {
			perror("recv");
		}
		free(image);
	}

	for (int i=0; i<CNT; i++) {
		close(new_fd[i]);  // parent doesn't need this
	}
	if (buf != NULL) free(buf);

	return 0;
}
/*
//receive specific size of bytes from the socket
int recvdata(int  socket, void *buffer, size_t size, int flags) {
    int r;
	int cnt=0;
	void *b = buffer;
    do {
        r = recv(socket, b, size, flags);
        if (r == 0 || r == -1) break;
        b += r;
        size -= r;
		cnt += r;
    } while (size);
    return cnt;
}

//send specific size of bytes to a socket
int senddata(int  socket, const void *buffer, size_t size, int flags) {
    int r;
	int cnt=0;
	const char *b=buffer;
    do {
        r = send(socket, (const char *)b, size, flags);
        if (r == 0 || r == -1) break;
        b +=  r;
        size -= r;
		cnt += r;
    } while (size);
    return cnt;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
*/
