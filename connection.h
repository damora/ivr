#ifndef CONNECTION_H
#define CONNECTION_H
#include <sys/socket.h>

#define CNT 2
#define BACKLOG 10   // how many pending connections queue will hold


typedef struct _STATE {
	int finish;
	int winwidth;
	int winheight;
	float alpha;
	float newconst[6];
	float vv[4];
	float mvm[16];
	float pm[16];
} STATE;

void *get_in_addr(struct sockaddr *);
void disconnect(int);
int initconnection(char *, char *);
int senddata(int, const void *, size_t, int);
int recvdata(int, void *, size_t, int);
int sendstate(int, STATE );
int recvstate(int, STATE *);
int sendimage(int, unsigned char *, int size);
int recvimage(int, unsigned char *, int size);
static inline unsigned int htonf(float);
static inline unsigned int htonf(float f)
{
    unsigned int p;
    unsigned int sign;

    if (f < 0) { sign = 1; f = -f; }
    else { sign = 0; }
        
    p = ((((unsigned int)f)&0x7fff)<<16) | (sign<<31); // whole part and sign
    p |= (unsigned int)(((f - (int)f) * 65536.0f))&0xffff; // fraction

    return p;
}

static float ntohf(unsigned int);
static inline float ntohf(unsigned int p)
{
    float f = ((p>>16)&0x7fff); // whole part
    f += (p&0xffff) / 65536.0f; // fraction

    if (((p>>31)&0x1) == 0x1) { f = -f; } // sign bit set

    return f;
}
#endif
