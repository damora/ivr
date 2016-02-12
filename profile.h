//
//  profile.h
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 1/28/13.
//  Copyright (c) 2013 Bruce D'Amora. All rights reserved.
//

#ifdef LOCAL
#ifdef __linux
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif
#endif

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif


#ifndef _PROFILE_H
#define _PROFILE_H

#ifdef MAIN
unsigned long long          start=0L;
unsigned long long          stop=0L;
int             			framenum=0;
float           			elapsed=0.0f;
float           			avg=0.0f;
float           			convfac=0.0f;
float           			nsec_per_sec=0.0f;
#ifdef __APPLE__
mach_timebase_info_data_t info;
#endif
#else
extern unsigned long long	start;
extern long		stop;
extern int		framenum;
extern float	elapsed;
extern float	avg;
extern float    convfac;
extern float	nsec_per_sec;
#ifdef __APPLE__
extern mach_timebase_info_data_t info;
#endif
#endif

inline static void renderstring(char* string)
{

#ifdef LOCAL
    glutSetWindowTitle(string);
#else
    fprintf(stderr, "%s\n", string);
#endif
}


#ifdef __APPLE__
#define gettickcount() mach_absolute_time()
#elif _WIN32
#define gettickcount() GetTickCount()
#elif BGQ
#include <hwi/include/bqc/A2_inlines.h>
#define gettickcount() GetTimeBase();
#elif __linux
#include <sys/time.h>
struct timeval tv;
#define gettickcount() (gettimeofday(&tv,NULL) != 0) ? 0 : ((tv.tv_sec) + (tv.tv_usec/1000000));
#endif

#ifdef _RENDER
#define RENDER_TIME(t) t = gettickcount()
#else
#define RENDER_TIME(t)
#endif

#ifdef _INTERSECT
#define INTERSECT_TIME(t) t = gettickcount()
#else
#define INTERSECT_TIME(t)
#endif

#ifdef _SAMPLE
#define SAMPLE_TIME(t) t = gettickcount()
#else
#define SAMPLE_TIME(t)
#endif

#ifdef _COMPOSITE
#define COMPOSITE_TIME(t) t = gettickcount()
#else
#define COMPOSITE_TIME(t)
#endif

#define TIME(s, t) s##_TIME(t)

#define F_RATE(fn, totalsec) ((double)fn)/((totalsec <= 0) ? 1.0:totalsec)

#ifdef __APPLE__
#define	RESET_TIME()                                                            \
{																				\
start = 0;																		\
stop = 0;																		\
framenum = 0;																	\
elapsed = 0.0f;																	\
avg= 0.0f;                                                                      \
mach_timebase_info(&info);                                                      \
convfac =  info.numer/(float)info.denom;                                        \
nsec_per_sec = 1.0/NSEC_PER_SEC;												\
}
#else 
#define RESET_TIME()                                                            \
{                                                                               \
start = 0;                                                                      \
stop = 0;                                                                       \
framenum = 0;                                                                   \
elapsed = 0.0f;                                                                 \
avg= 0.0f;                                                                      \
}

#endif

// a *= NSEC_PER_SEC;
#ifdef __APPLE__
#define PERFORMANCE(str, stp, fnum, elap, a)                                    \
{                                                                               \
float e;																		\
elap += (stp-str);                                                              \
e = elap * nsec_per_sec;														\
a = F_RATE(++fnum, e);															\
char title[25];                                                                 \
a *= convfac;                                                                   \
sprintf(title, "iVR %e fps", a);												\
renderstring(title);					              							\
}
#elif BGQ
#define PERFORMANCE(str, stp, fnum, elap, a)                                    \
{                                                                               \
elap += (stp-str);                                                              \
float qelap = elap/1600000000;													\
a = F_RATE(++fnum, qelap);                                                      \
char title[27];                                                                 \
sprintf(title, "iVR %3.5f fps", a);                                             \
renderstring(title);                                                            \
}
#else
#define PERFORMANCE(str, stp, fnum, elap, a)                                    \
{                                                                               \
elap += (stp-str);                                                              \
a = F_RATE(++fnum, elap);                                                       \
char title[27];                                                                 \
sprintf(title, "iVR %3.5f fps", a);                                             \
renderstring(title);					              							\
}
#endif


#endif
