//
//  types.h
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 11/29/12.
//  Copyright (c) 2012 Bruce D'Amora. All rights reserved.
//

#ifndef IBM_Ray_Caster_types_h
#define IBM_Ray_Caster_types_h
#define TRILINEARSAMPLE 	1
#define OVERLAP			   	1	  // overlap between subvolumes is 1 voxel
//#define DELTASAMPLE       	0.0000125
#ifdef ORTHO
#define DELTASAMPLE       	0.000002f
#else
#define DELTASAMPLE       	0.001f
#endif
#define DELTAALPHA			0.05f
#define BYTESPERPIXEL		4
#define ELEMENTSPERPIXEL    4
#define WINDOWWIDTH       	1280
#define WINDOWHEIGHT     	720
#define FRUSTUMWIDTH		4.0f
#define FRUSTUMHEIGHT		4.0f
#define NUMTILESX			2 
#define	NUMTILESY			2
#define MAXVOLUMESAMPLES    64
#define ROTATESENSITIVITY 2.0f
#define ZOOMSENSITIVITY   2.0f
#define MOVESENSITIVITY   200.0f
#define MEDIAN            25000
#define LOWBOUND          500
#define ZOOMMIN           0.01f
#define ZOOMMAX           100.0f
#define MAXPLANE 			6
#define MINX			 -1.0f
#define MINY			 -1.0f
#define MINZ			 -1.0f
#define MAXX			  1.0f
#define MAXY			  1.0f
#define MAXZ			  1.0f
#define RANGE 				4096.0f
#define RADIANS  			M_PI/180.0f
#define DEGREES  			180.0f/M_PI
#define ROTATE				0
#define PAN					1
#define ZOOM				2
#define ALPHA				3
#define CUTX				4
#define CUTY				5
#define CUTZ				6
#define HOME				7


#ifdef  __linux
#include <stdbool.h>
typedef bool boolean_t;
#define TRUE true
#define FALSE false
#endif

typedef struct _size {
    int width;
    int height;
} size;


typedef struct _vector2f {
    float x,y;
} vector2f;

typedef struct _vector3i {
	union {
    	int x, w, i;
	};
	union {
    	int y, h, j;
	};
	union {
    	int z, d, k;
	};
} vector3i;

typedef struct _vector3f {
    union {
        float x;
        float u;
		float w;
    };
    union {
        float y;
        float v;
		float h;
    };
    union {
        float z;
        float n;
		float d;
    };
} vector3f;

typedef struct _vector4f {
    float x,y,z,w;
} vector4f;

typedef float matrix3f[3][3];
typedef float matrix4f[4][4];

typedef struct _ray3f {
    vector3f orig;
    vector3f dir;
} ray3f;

typedef struct _color4f {
    float r,g,b,a;
} color4f;

typedef struct _index3d {
    int i, j, k;
} index3d;

typedef struct _CMDARGS {
	char filename[80];
	int	width;
	int height;	
	int depth;
	int	xfer;
	int	colormap;
	int dataendian;
	int procdims[3];
	int tilewid;
	int tilehgt;
	float angle;
	vector3f axis;
} CMDARGS;
	
typedef union _DATA {
	float *f;
	unsigned int *i;
} DATA;

typedef struct _VOLUME {
//   	float    *values;         // value at index for point; interpolated value for trilinear
	DATA data;
    vector4f *gradient;       // gradient array - use dx, dy, dz, mag(dx,dy,dz) for 4th element
    vector3i globaldims;      // dimensions of whole volume
    vector3i localdims;       // dimensions of subvolume
	vector3i procdims;		  // dcomposition rows, cols, slabs
	vector3i startindex;	  // starting offset into volume
    vector3f celldims;        // dimensions of volume cells
    vector3f fvoldims;        // dimensions of volume as fp32s
    vector3f rfvoldims;       // reciprocal of dimensions as fp32s
	int		 endianness;	  // endianness of data
	int		 byteswap;		  // flag for byteswap
} VOLUME;


typedef struct _RECT {
	float x,y,z;
	float w,h,d;
	float cx,cy;

} RECT;

typedef struct _BRICK {
	float w,h,d;
	float x,y,z;
} BRICK;

typedef struct _AABB {
	int 	rank;
	float 	w,h,d;
	RECT 	wincoord; // these are projected window or viewport coordinate so go from 0...WINDOWWITH, WINDOWHEIGHT
    vector3f mvmin;
    vector3f mvmax;
    vector3f min;
    vector3f max;
	vector3f facenormals[6];
	vector4f verts[8];
	vector4f everts[8];
	vector4f pverts[8];
	vector4f mverts[8];

} AABB;

typedef struct _CAMERA {
    vector3f r;             // view reference point
    vector3f n;             // n axis (analog to y axis in world coords)
    vector3f v;             // v axis (analog to z axis in world coords)
    vector3f u;             // u axis (analog to x axis in world coords)
	vector3f e;				// eye position
} CAMERA;


typedef struct _FRUSTUM {
    float left;
    float right;
    float top;
    float bottom;
    float near;
    float far;
} FRUSTUM;

typedef struct _WINDOW {
	int	  tilex, tiley;
	int	  tilewid, tilehgt;
    float x;
    float y;
    float w;
    float h;
} WINDOW;
#endif
