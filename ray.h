#ifndef RAY_H
#define RAY_H
#include "types.h"
#include "mathutil.h"
#include "transferfuncs.h"
#include "profile.h"
#include "utils.h"
#include "connection.h"

#define NEAREST 0
#define LINEAR 1

//-----------------------------------------------------------------------------------------------------------------------//
// ray casting function prototypes
//-----------------------------------------------------------------------------------------------------------------------//
void raycast(ray3f ray, color4f *);
void getvoxelsonray(ray3f, int, float *);
int getsamplesonray(ray3f, int, float *);
int getfirstvoxel(ray3f, vector3i *);
float hitvolume(ray3f, AABB);
float getlinearsample(float, float, float);
float getnearestsample(float, float, float);
boolean_t getrayintersect(ray3f, float *, float *);
void setplaneconst(float *, float *);
float (*getsamplefunc)(float, float, float);
void initsamplefunc(int);
#endif
