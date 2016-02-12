//
//  sysutil.h
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 11/29/12.
//  Copyright (c) 2012 Bruce D'Amora. All rights reserved.
//

#ifndef IBM_Ray_Caster_sysutil_h
#define IBM_Ray_Caster_sysutil_h
#include <sys/types.h>
#ifdef __linux
#include <sys/sysinfo.h>
#include <unistd.h>
typedef u_int64_t uint64_t;
#else
#include <sys/sysctl.h>
#endif

int querymemory(long long );
long int querycachesize(void);
//----------------------------------------------------------------------------------------------------------------------------//
// system functions                                                                                                           //
//----------------------------------------------------------------------------------------------------------------------------//
uint64_t get_cpu_num(void)
{
   uint64_t num = 0;
#ifdef __APPLE__
    size_t size = sizeof(num);

    if (sysctlbyname("hw.ncpu", &num, &size, NULL, 0) < 0)
    {
        perror("sysctl");
    }
#else
    num = get_nprocs();
#endif
    return num;

}


uint64_t get_free_mem(void)
{
    uint64_t bytes = 0;

#ifdef __APPLE__
    size_t size = sizeof(bytes);

    if (sysctlbyname("hw.usermem", &bytes, &size, NULL, 0) < 0)
    {
        perror("sysctl");
    }
#else
    struct sysinfo myinfo;
    if (sysinfo(&myinfo) < 0)
    {
        perror("sysinfo");
    }
    bytes = myinfo.freeram;
#endif
    return bytes;
}

uint64_t get_l2_size(void)
{
    uint64_t bytes = 0;
#ifdef __APPLE__
    size_t size = sizeof(bytes);

    if (sysctlbyname("hw.l2cachesize", &bytes, &size, NULL, 0) < 0)
    {
        perror("sysctl");
    }
#endif
    return bytes;

}

int querymemory(long long numvoxels)
{
#ifdef __linux
    long int pages = sysconf (_SC_AVPHYS_PAGES);
    long int pagesize = sysconf(_SC_PAGESIZE);
    long int memsize = pages * pagesize;
#else
    long int memsize = get_free_mem();
#endif
    if (numvoxels*sizeof(float) >= (unsigned long int) memsize>>1) {
        fprintf(stderr, "memsize=%ld; volume too large for memory: %lld\n", memsize, numvoxels);
        return -1;
    }
    return 0;

}

long int querycachesize()
{
#ifdef __linux
    long int l2size = sysconf(_SC_LEVEL2_CACHE_SIZE);
#else
    long int l2size = get_l2_size();
#endif
    return l2size;
}

//  Returns 1 if LITTLE-ENDIAN or 0 if BIG-ENDIAN
#include <inttypes.h>
int endianness(void);
int endianness()
{
    union { uint8_t c[4]; uint32_t i; } data;
    data.i = 0x12345678;
    return (data.c[0] == 0x78);
}
typedef union _SWAP {
float f;
unsigned int i;
unsigned char buf[4];
} SWAP;
float BYTESWAP_FP(float n)
{
    SWAP t0, t1;

    t0.f  = n;

    t1.buf[3] = t0.buf[0];
    t1.buf[2] = t0.buf[1];
    t1.buf[1] = t0.buf[2];
    t1.buf[0] = t0.buf[3];

    return t1.f;
}
unsigned int  BYTESWAP_UI(unsigned int n)
{
    SWAP t0, t1;

    t0.f  = n;

    t1.buf[3] = t0.buf[0];
    t1.buf[2] = t0.buf[1];
    t1.buf[1] = t0.buf[2];
    t1.buf[0] = t0.buf[3];

    return t1.i;
}
#endif

