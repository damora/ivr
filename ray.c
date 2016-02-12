//
//  main.c
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 10/20/12.
//  Copyright (c) 2012 Bruce D'Amora. All rights reserved.
//
//  Notes:
//  - use a right-handed coord system - consistent with OpenGL and heck I'm right handed :-)
//  - could use unit cube, but more convenient to have origin centered cube from -1,-1,-1 to 1,1,1
//  - quaternion based camera for viewing
//  - tri-linear sampling, but also have point sampling code in place
//  - added gradient over scalar field for lighint later on, but for now it isn't used
//  - working on color lut still, but really need a transfer function editor

#include <stdio.h>
#ifdef __linux
#include <values.h>
#endif
#include <math.h>

#include <assert.h>
#include "ray.h"

extern int 			cnt;
extern int			comm;
extern float 		deltasample;
extern color4f		*lut;
extern color4f		*lut;
extern STATE		mystate;
extern AABB			aabb;
extern VOLUME		volume;



//extern float				pzero[];

float				dconst[6];


//------------------------------------------------------------------------------------------------------------------------//
// ray casting functions                                                                                                  //
//------------------------------------------------------------------------------------------------------------------------//
void setplaneconst(float planeconst[], float newconst[])
{

	for (int i=0; i<6; i++) 
		dconst[i] = (newconst[i] < planeconst[i]) ? newconst[i] : planeconst[i];
	
	fprintf(stderr, "setplaneconst(): dconst=%f\n", dconst[0]);
}

int map(float x)
{
    float numbuckets = 4096.0f/256.0f;
    return (int) (x/numbuckets);
}

void raycast(ray3f ray, color4f * color)
{
    int maxsamples = MAXVOLUMESAMPLES * 4;

    int num = 0;
    float samples[MAXVOLUMESAMPLES * 4];

    // get samples along ray at deltat points along the ray
    num = getsamplesonray(ray, maxsamples, samples);
 
	PRINTDEBUG2("NUM - %d\n", num);

    if (num <= 0) {
        color->a = color->b = color->g = color->r = 0.0f;
        return;
    } 
    
    // this is where you will apply a transfer function to blend voxels and
    // assign a final color which will be returned to showimage() calling function
    color->a =  color->b = color->g = color->r = 0.0f;

    // apply transfer function
    *color = (*transferfunc)(lut, *color, samples, num, mystate.alpha);


    return;
}

inline float getlinearsample(float x, float y, float z)
{
    float x0, y0, z0, xd, yd, zd, xp, yp, zp;

    // map x, y, z to volumetric data indices
    vector3f xyz = {{x}, {y}, {z}};
    
    //  volume dimensions
    int vw = volume.localdims.w;
    int vh = volume.localdims.h;
    int vd = volume.localdims.d;

#ifdef BGQ
    float fvw = volume.fvoldims.x;
    float fvh = volume.fvoldims.y;
    float fvd = volume.fvoldims.z;

    float rfvw = volume.rfvoldims.x;
    float rfvh = volume.rfvoldims.y;
    float rfvd = volume.rfvoldims.z;

    float fj = __frizs (x *  fvw/aabb.w -  (aabb.min.x*fvw)/aabb.w);
    float fi = __frizs (y *  fvh/aabb.h -  (aabb.min.y*fvh)/aabb.h);
    float fk = __frizs (z *  fvd/aabb.d -  (aabb.min.z*fvd)/aabb.d);

    fj = __fsels((fj - (fvw-2.0f)), fvw-2.0f, fj);
    fi = __fsels((fi - (fvh-2.0f)), fvh-2.0f, fi);
    fk = __fsels((fk - (fvd-2.0f)), fvd-2.0f, fk);

    int i = (int) fi;
    int j = (int) fj;
    int k = (int) fk;

	PRINTDEBUG2("IJK: %d,%d,%d\n", i,j,k);

#else
    // number of voxels in each direction and should be set when volume is input
    // mapping should be (volr-voll)/(box.r-box.l) + (aabb.x*vw/bboxw

	float ratio = (xyz.x-aabb.min.x)/aabb.w;
	int j = vw * ratio;
	ratio = (xyz.y-aabb.min.y)/aabb.h;
	int i = vh * ratio;
	ratio = (xyz.z-aabb.min.z)/aabb.d;
	int k = vd * ratio;
	
    i = (i > vh-2) ? vh-2 : i;
    j = (j > vw-2) ? vw-2 : j;
    k = (k > vd-2) ? vd-2 : k;
	
#endif

    // get 8 neighboring voxels
	float v000, v100, v010, v001, v101, v011, v110, v111;

   v000 = *(volume.data.f + maptoffset3d(i, j, k, vw, vh, vd));
   v100 = *(volume.data.f + maptoffset3d(i+1, j, k, vw, vh, vd));
   v010 = *(volume.data.f + maptoffset3d(i, j+1, k, vw, vh, vd));
   v001 = *(volume.data.f + maptoffset3d(i, j, k+1, vw, vh, vd));
   v101 = *(volume.data.f + maptoffset3d(i+1, j, k+1, vw, vh, vd));
   v011 = *(volume.data.f + maptoffset3d(i, j+1, k+1, vw, vh, vd));
   v110 = *(volume.data.f + maptoffset3d(i+1, j+1, k, vw, vh, vd));
   v111 = *(volume.data.f + maptoffset3d(i+1, j+1, k+1, vw, vh, vd));
    //  tri-linear interpolation; I should pre-compute x0, y0, z0 for each cell probably
	// map to 0..aabb->max
    xp = x + fabs(aabb.min.x);
    yp = y + fabs(aabb.min.y);
    zp = z + fabs(aabb.min.z);
#ifdef BGQ
    x0 = (fj * aabb.w) * rfvw;
    y0 = (fi * aabb.h) * rfvh;
    z0 = (fk * aabb.d) * rfvd;
	
    xd = (__fabss(xp - x0) * aabb.w) * rfvw;
    yd = (__fabss(yp - y0) * aabb.h) * rfvh;
    zd = (__fabss(zp - z0) * aabb.d) * rfvd;
#else 
    x0 = (j * aabb.w)/vw;
    y0 = (i * aabb.h)/vh;
    z0 = (k * aabb.d)/vd;
    
    xd = (fabsf(xp - x0) * aabb.w)/vw;
    yd = (fabsf(yp - y0) * aabb.h)/vh; 
    zd = (fabsf(zp - z0) * aabb.d)/vd;
#endif
    float c00 = v000 * (1.0f - xd) + v100 * xd;
    float c10 = v010 * (1.0f - xd) + v110 * xd;
    float c01 = v001 * (1.0f - xd) + v101 * xd;
    float c11 = v011 * (1.0f - xd) + v111 * xd;
    float c0 = c00 * (1.0f - yd) + c10 * yd;
    float c1 = c01 * (1.0f - yd) + c11 * yd;
    float c = c0 * (1.0f - zd) + c1 * zd;
    
    return c;
}
inline float getnearestsample(float x, float y, float z)
{
    // map x, y, z to volumetric data indices
    vector3f xyz = {{x}, {y},  {z}};
    
    //  volume dimensions
    int vw = volume.localdims.w;
    int vh = volume.localdims.h;
    int vd = volume.localdims.d;

#ifdef BGQ
    float fvw = volume.fvoldims.x;
    float fvh = volume.fvoldims.y;
    float fvd = volume.fvoldims.z;

    float rfvw = volume.rfvoldims.x;
    float rfvh = volume.rfvoldims.y;
    float rfvd = volume.rfvoldims.z;

    float fj = __frizs (x *  fvw/aabb.w -  (aabb.min.x*fvw)/aabb.w);
    float fi = __frizs (y *  fvh/aabb.h -  (aabb.min.y*fvh)/aabb.h);
    float fk = __frizs (z *  fvd/aabb.d -  (aabb.min.z*fvd)/aabb.d);

    fj = __fsels((fj - (fvw-2.0f)), fvw-2.0f, fj);
    fi = __fsels((fi - (fvh-2.0f)), fvh-2.0f, fi);
    fk = __fsels((fk - (fvd-1.0f)), fvd-1.0f, fk);

    int i = (int) fi;
    int j = (int) fj;
    int k = (int) fk;

	PRINTDEBUG2("IJK: %d,%d,%d\n", i,j,k);

#else
    // number of voxels in each direction and should be set when volume is input
    // mapping should be (volr-voll)/(box.r-box.l) + (aabb.x*vw/bboxw

	float ratio = (xyz.x-aabb.min.x)/aabb.w;
	int j = vw * ratio;
	ratio = (xyz.y-aabb.min.y)/aabb.h;
	int i = vh * ratio;
	ratio = (xyz.z-aabb.min.z)/aabb.d;
	int k = vd * ratio;
	
    i = (i >= vh-2) ? vh-2 : i;
    j = (j >= vw-2) ? vw-2 : j;
    k = (k >= vd-2) ? vd-2 : k;
	
#endif

    // get nearest  neighboring voxels

	float c;
    c = *(volume.data.i + maptoffset3d(i, j, k, vw, vh, vd));
    
    return c;
}
int getsamplesonray(ray3f ray, int maxsamples, float *samples)
{

    float deltat = DELTASAMPLE;
    float x, y, z;
    float tin, tout;
    boolean_t hit = FALSE;
    if ((hit = getrayintersect(ray, &tin, &tout)) != TRUE) {
        return -1;
    }

	cnt++;
	int i=0;
	float t;
    int iend =  1.0f + (tout - tin)/deltat + 0.5f;
	iend = (iend < maxsamples) ? iend : maxsamples;
#pragma omp parallel for private(x, y, z, t) lastprivate(i)
    for (i=0; i<iend; i++) {
        t = tin + (i * deltat);
        
        x = ray.dir.x * t + ray.orig.x;
        y = ray.dir.y * t + ray.orig.y;
        z = ray.dir.z * t + ray.orig.z;

 
        //samples[i] = getsamplevalue(x, y, z);
		samples[i] = getsamplefunc(x, y, z);
#ifdef REVISIT // probably need to save these intersections for later isosurface painting...yuk
        samples[i].intersect.x = x;
        samples[i].intersect.y = y;
        samples[i].intersect.z = z;
#endif
    }
    return i;
}
        

boolean_t getrayintersect(ray3f ray, float *tin, float* tout)
{
    int i;
    float numer, denom;
    float thit;
	vector3f * n;
    boolean_t hit = FALSE;

	n = aabb.facenormals;
     
    *tin = -MAXFLOAT;
    *tout = MAXFLOAT;
	i = 0;
    while ((i < MAXPLANE) && (*tin < *tout)) {
        // compute numer, denom for ith plane
        numer = dconst[i] - dot3f(n[i], ray.orig);
        denom = dot3f(n[i], ray.dir);
        if (denom == 0.0f) {
            if (numer < 0.0f) *tin = *tout + 1.0f;
        }
        else {
            thit = numer/denom;
            if (denom > 0.0f ) {
               *tout = fmin(*tout, thit); 
            }
            else {
               *tin = fmax(*tin, thit);
            }
        }
        i++;
    }
    if (*tin < *tout) {
        if (*tin > 0.0f) {
            hit = TRUE;
//            thit = *tin;
        }
        else if (*tout > 0.0f) {
            hit = TRUE;
//            thit = *tout;
        }
    }
    return hit;
}

#define X_FACE 0
#define Y_FACE 1
#define Z_FACE 2
#define MAX_FACE 4

// true if we hit a box face, false otherwise
int hitface(float uhit,float vhit,
              float umin,float umax,float vmin,float vmax)
{
    return (umin <= uhit && uhit <= umax && vmin <= vhit && vhit <= vmax);
}

// 0.0 if we missed, the time of impact otherwise
int getfirstvoxel(ray3f ray, vector3i *samples)
{
    float times[6];
    int hits[6];
    int faces[6];
    double t;
    if (ray.dir.x==0) { times[0] = times[1] = 0.0; }
    else {
        t = aabb.min.x/ray.dir.x;
        times[0] = t; faces[0] = X_FACE;
        hits[0] = hitface(t*ray.dir.y , t*ray.dir.z , aabb.min.y , aabb.max.y , aabb.min.z , aabb.max.z);
        t = aabb.max.x/ray.dir.x;
        times[1] = t; faces[1] = X_FACE + MAX_FACE;
        hits[1] = hitface(t*ray.dir.y , t*ray.dir.z , aabb.min.y , aabb.max.y , aabb.min.z , aabb.max.z);
    }
    if (ray.dir.y==0) { times[2] = times[3] = 0.0; }
    else {
        t = aabb.min.y/ray.dir.y;
        times[2] = t; faces[2] = Y_FACE;
        hits[2] = hitface(t*ray.dir.x , t*ray.dir.z , aabb.min.x , aabb.max.x , aabb.min.z , aabb.max.z);
        t = aabb.max.y/ray.dir.y;
        times[3] = t; faces[3] = Y_FACE + MAX_FACE;
        hits[3] = hitface(t*ray.dir.x , t*ray.dir.z , aabb.min.x , aabb.max.x , aabb.min.z , aabb.max.z);
    }
    if (ray.dir.z==0) { times[4] = times[5] = 0.0; }
    else {
        t = aabb.min.z/ray.dir.z;
        times[4] = t; faces[4] = Z_FACE;
        hits[4] = hitface(t*ray.dir.x , t*ray.dir.y , aabb.min.x , aabb.max.x , aabb.min.y , aabb.max.y);
        t = aabb.max.z/ray.dir.z;
        times[5] = t; faces[5] = Z_FACE + MAX_FACE;
        hits[5] = hitface(t*ray.dir.x , t*ray.dir.y , aabb.min.x , aabb.max.x , aabb.min.y , aabb.max.y);
    }
    int first = 6;
    t = 0.0;
    for (int i=0 ; i<6 ; i++) {
        if (times[i] > 0.0 && (times[i]<t || t==0.0)) {
            first = i;
            t = times[i];
        }
    }
    vector3f v;
    v.x = ray.dir.x*times[first] + ray.orig.x;
    v.y = ray.dir.y*times[first] + ray.orig.y;
    v.z = ray.dir.z*times[first] + ray.orig.z;
    
    samples->x = (int) (v.x + 0.5f);
    samples->y = (int) (v.y + 0.5f);
    samples->z = (int) (v.z + 0.5f);
    if (first>5) return -1;  // Found nothing
    else return 0;  // Probably want hits[first] and faces[first] also....
}


void getvoxelsonray(ray3f ray, int maxdepth, float *samples)
{
    // Implementation is based on:
    // "A Fast Voxel Traversal Algorithm for Ray Tracing"
    // John Amanatides, Andrew Woo
    // http://www.cse.yorku.ca/~amana/research/grid.pdf
    // http://www.devmaster.net/articles/raytracing_series/A%20faster%20voxel%20traversal%20algorithm%20for%20ray%20tracing.pdf
    
    // NOTES:
    // * This code assumes that the ray's origition and direction are in 'cell coordinates', which means
    //   that one unit equals one cell in all directions.
    // * When the ray doesn't start within the voxel grid, calculate the first origition at which the
    //   ray could enter the grid. If it never enters the grid, there is nothing more to do here.
    // * Also, it is important to test when the ray exits the voxel grid when the grid isn't infinite.
    // * The vector3f structure is a simple structure having three integer fields (X, Y and Z).
    
    // The cell in which the ray starts.
    
    int rc=0;
    vector3i first;
    // Rounds the origition's x, y and z down to the nearest integer values.
    if ((rc=getfirstvoxel(ray, &first)) == -1) {
        //fprintf(stderr, "ray misses volume\n");
        return;
    };
    int x = first.x;
    int y = first.y;
    int z = first.z;
    
    
    // Determine which way we go.
    vector3i step;
    step.x = signof(ray.dir.x);
    step.y = signof(ray.dir.y);
    step.z = signof(ray.dir.z);
    
    // Calculate cell boundaries. When the step (i.e. direction sign) is origitive,
    // the next boundary is AFTER our current origition, meaning that we have to add 1.
    // Otherwise, it is BEFORE our current origition, in which case we add nothing.
    vector3f cellboundary;
    cellboundary.x = x + (step.x > 0 ? 1 : 0);
    cellboundary.y = y + (step.y > 0 ? 1 : 0);
    cellboundary.z = z + (step.z > 0 ? 1 : 0);
    
    
    // NOTE: For the following calculations, the result will be HUGE_VAL (+infinity)
    // when ray.dir.x, y or z equals zero, which is OK. However, when the left-hand
    // value of the division also equals zero, the result is Single.NaN, which is not OK.
    
    // Determine how far we can travel along the ray before we hit a voxel boundary.
    vector3f tmax;
    tmax.x = (cellboundary.x - ray.orig.x) / ray.dir.x;    // Boundary is a plane on the YZ axis.
    tmax.y = (cellboundary.y - ray.orig.y) / ray.dir.y;    // Boundary is a plane on the XZ axis.
    tmax.z = (cellboundary.z - ray.orig.z) / ray.dir.z;    // Boundary is a plane on the XY axis.
    
    if (isnan(tmax.x)) tmax.x = HUGE_VAL;
    if (isnan(tmax.y)) tmax.y = HUGE_VAL;
    if (isnan(tmax.z)) tmax.z = HUGE_VAL;
    
    // Determine how far we must travel along the ray before we have crossed a gridcell.
    vector3f deltat ;
    deltat.x = step.x / ray.dir.x;                    // Crossing the width of a cell.
    deltat.y = step.y / ray.dir.y;                    // Crossing the height of a cell.
    deltat.z = step.z / ray.dir.z;                    // Crossing the depth of a cell.
    
    if (isnan(deltat.x)) deltat.x = HUGE_VAL;
    if (isnan(deltat.y)) deltat.y = HUGE_VAL;
    if (isnan(deltat.z)) deltat.z = HUGE_VAL;
    
    // For each step, determine which distance to the next voxel boundary is lowest (i.e.
    // which voxel boundary is nearest) and walk that way.
    for (int i = 0; i < maxdepth; i++)
    {
        // compute index and save it.
        samples[i] = maptoffset3d(x,y,z, volume.localdims.w, volume.localdims.h, volume.localdims.d);
        
        // Do the next step.
        if (tmax.x < tmax.y && tmax.x < tmax.z)
        {
            // tmax.x is the lowest, an YZ cell boundary plane is nearest.
            x += step.x;
            tmax.x += deltat.x;
        }
        else if (tmax.y < tmax.z)
        {
            // tmax.y is the lowest, an XZ cell boundary plane is nearest.
            y += step.y;
            tmax.y += deltat.y;
        }
        else
        {
            // tmax.z is the lowest, an XY cell boundary plane is nearest.
            z += step.z;
            tmax.z += deltat.z;
        }
    }
}

void initsamplefunc(int type)
{

	switch(type) {
		case NEAREST:
			getsamplefunc = &getnearestsample;
			break;

		case LINEAR:
			getsamplefunc = &getlinearsample;
			break;

		default:
			getsamplefunc = &getlinearsample;
			break;
	}
}
		
