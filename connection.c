/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "connection.h"
#include "types.h"


//message header
struct MESSAGE_HEADER {
    unsigned short type;
    unsigned short length;
};

//any message
struct MESSAGE {
    char buffer[512];
};
union swap {
float f;
unsigned char buf[4];
};
float SWAP_FP(float n)
{
    union swap t0, t1;

    t0.f  = n;

    t1.buf[3] = t0.buf[0];
    t1.buf[2] = t0.buf[1];
    t1.buf[1] = t0.buf[2];
    t1.buf[0] = t0.buf[3];

    return t1.f;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int initconnection(char *hostname, char *portnum)
{
	int sockfd;  
	int numbytes;  
	char *buf;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];



	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	fprintf(stderr,"hostname=%s, portnum=%s\n", hostname, portnum);
	if ((rv = getaddrinfo(hostname, portnum, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);

	// disable Nagle algorithm with TCP_NODELAY
	int flag=1;
	int result;
	if ((result=setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int))) < 0)
		perror("setsockopt");

	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	buf = (char *) malloc(sizeof(char) * 14);
	if ((numbytes = recv(sockfd, buf, 13, 0)) == -1) {
    	perror("recv 0");
    	exit(1);
	}
	else if (send(sockfd, "Hello, server!", 14, 0) == -1)
               perror("send");

	buf[numbytes] = '\0';
	fprintf(stderr,"client: received '%s'\n",buf);
	free(buf);

	return sockfd;
}

void disconnect(int socket)
{
	close(socket);
	return;
}

//receive specific size of bytes from the socket
int recvdata(int  socket, void *buffer, size_t size, int flags) {
    int r;
	int cnt = 0;
	void *b = buffer;
    do {
        r = recv(socket, b, size, flags);
        if (r == 0 || r == -1) break;
        b += r;
        size -= r;
		cnt += r;
    } while (size > 0);
    return cnt;
}

//send specific size of bytes to a socket
int senddata(int  socket, const void *buffer, size_t size, int flags) {
    int r;
	int cnt = 0;
	const char *b = buffer;
    do {
        r = send(socket, (const char *)b, size, flags);
        if (r == 0 || r == -1) break;
        b += r;
        size -= r;
		cnt += r;
    } while (size);
    return cnt;
}
int sendstate(int socket, STATE mystate)
{
    int num = 0;

	// TODO: hton here
	mystate.finish = htonl(mystate.finish);
	mystate.winwidth = htonl(mystate.winwidth);
	mystate.winheight = htonl(mystate.winheight);

	num = senddata(socket, (unsigned char *)&mystate, sizeof(STATE), 0);
    return num;
}
int recvstate(int socket, STATE *mystate)
{
	int cnt;
	unsigned char *buf = ( unsigned char *) malloc(sizeof(STATE));

    cnt = recvdata(socket, (unsigned char *) buf, sizeof(STATE), 0);

	// ntoh here 
	memcpy(mystate, buf, sizeof(STATE));
	mystate->finish = ntohl(mystate->finish);
	mystate->winwidth = ntohl(mystate->winwidth);
	mystate->winheight = ntohl(mystate->winheight);
#ifdef BIGENDIAN
	mystate->alpha  = SWAP_FP(mystate->alpha);
	for (int i=0; i<6;  i++) mystate->newconst[i]  = SWAP_FP(mystate->newconst[i]);
	for (int i=0; i<4;  i++) mystate->vv[i]  = SWAP_FP(mystate->vv[i]);
	for (int i=0; i<16; i++) mystate->mvm[i] = SWAP_FP(mystate->mvm[i]);
	for (int i=0; i<16; i++) mystate->pm[i] = SWAP_FP(mystate->pm[i]);
#endif

    return cnt;
}


int sendimage(int socket, unsigned char *image, int size)
{
	int cnt;

	cnt = senddata(socket, image, size, 0);
	return cnt;
}
int recvimage(int socket, unsigned char *image, int size)
{
	int cnt;

	cnt = recvdata(socket, image, size, 0);
	return cnt;
}

/*
//get message from socket
static bool receive(int  socket, MESSAGE &msg) {
    int r = receive(socket, &msg, sizeof(MESSAGE_HEADER));
    if (r == -1 || r == 0) return false;
    if (ntohs(msg.length) == 0) return true;
    r = receive(socket, msg.buffer, ntohs(msg.length));
    if (r == -1 || r == 0) return false;
    return true;
}

//send message
static bool send(int  socket, const MESSAGE &msg) {
    int r = send(socket, &msg, ntohs(msg.length) + sizeof(MESSAGE_HEADER));
    if (r == -1 || r == 0) return false;
    return true;
}
*/
