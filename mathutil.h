//
//  mathutil.h
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 11/29/12.
//  Copyright (c) 2012 Bruce D'Amora. All rights reserved.
//

#ifndef IBM_Ray_Caster_mathutil_h
#define IBM_Ray_Caster_mathutil_h
#ifdef __linux
#include <values.h>
#endif
#include <math.h>
#include "types.h"

extern VOLUME volume;
extern CAMERA camera;
extern AABB aabb;
extern FRUSTUM frustum, viewvolume;
extern WINDOW viewport;
extern vector3f dir, up, e, vrp;
extern matrix4f rotation;
extern int winwidth, winheight;
extern matrix3f vwm;
extern matrix3f wvm;
extern matrix4f vpm;
extern matrix4f vvm;
extern matrix4f mvm;
extern matrix4f pm;

inline static int imin(int x, int y) { return((x < y) ? x : y);}
inline static int imax(int x, int y) { return ((x > y) ? x : y);}
inline static int imax3(int w, int h, int d)
{
    int mmax = -1;
    mmax = imax(w,h);
    mmax = imax(mmax,d);
    return mmax;
}

inline static void lerp(float *colout, float *vec0, float *vec1,  int maplen)
{
	float incr1, incr2;
	
	incr1 = (vec1[1] - vec1[0])/(vec0[1]-vec0[0]);
    incr2 = (vec1[2] - vec1[1])/(vec0[2]-vec0[1]);

	
	for (int i=1; i<=maplen; i++)
	{
		if ((i >= vec0[0]) && (i <= vec0[1])) 
			colout[i-1] = vec1[0] + i * incr1;
		else if ((i >= vec0[1]) && (i <= vec0[2]))
			colout[i-1] = vec1[1] + (i-vec0[1]) * incr2;
	}
		
    return;	
}
int histogram(int **, float *, float *, float *, int, int, int);
int signof(float);
float degtorad(float);
float grad(int , int , int );
float mag3f(float, float, float);
float mag4f(float, float, float, float);
float dot3f(vector3f , vector3f);
float dot4f(vector4f, vector4f);
float computedelta(int, float, float, float, float);
void  lookat(vector3f, vector3f, vector3f); 
////////////////////////void composemvm(float [], float, float, vector3f);
void composemvm(matrix4f, float, vector3f);
void updatevolbbox();
void vec3ftomatrix3f(matrix3f, CAMERA);
void vec4ftomatrix4f(matrix4f, CAMERA);
void gradient3d(vector4f *,  int, int, int );
void setrotate(float, float, float, float);
void setvwm(matrix3f, CAMERA);
void setwvm(matrix3f, CAMERA);
void setvvm(matrix4f, FRUSTUM);
void setortho(float, FRUSTUM);
void setperspective(float , float , float , float );
void movecamera(vector3f);
void rotatecamera();
void zoomcamera(float);
void mat3fxmat3f(matrix3f, matrix3f);
void mat4fxmat4f(matrix4f, matrix4f);
void wvmtovp(CAMERA *, matrix3f);
void quattoaxisang(float *, float *, float *);
void setuvn(vector3f , vector3f);
void eaxisangtomatrix4(vector4f, matrix4f );
void axisangtomatrix4(vector4f, matrix4f );
void axisangtomatrix3(vector4f, matrix3f );
void transposerotation(matrix3f);
void transposemat4f(matrix4f);
void buildray(float, float, vector3f *, vector3f *, ray3f *);
vector3f mat3fxvec3f(matrix3f, vector3f);
vector4f mat4fxvec4f(matrix4f, vector4f);
vector4f conjugate(vector4f);
vector4f axisangtoquat(vector4f , float);
vector4f multquat(vector4f, vector4f);
vector4f rotvector(vector4f, vector4f);
vector3f cross3f(vector3f, vector3f);
vector3f vec3fadd(vector3f, vector3f);
vector4f vec4fadd(vector4f, vector4f);
vector3f vec3fsub(vector3f, vector3f);
vector4f vec4fsub(vector4f, vector4f);
vector4f vec4fadd(vector4f, vector4f);
vector3f vec3fmult(vector3f, float);
vector3f worldtoview(vector3f);
vector3f viewtoworld(vector3f);
vector3f normalize3f(vector3f);
vector4f normalize4f(vector4f);

static inline float clampf(float, float, float);
static inline float clampf(float in, float low, float high)
{
	return (((in > low) && (in <= high)) ? in : (in > high) ? high : low);

}
static inline void loadidentity(matrix4f );
static inline void loadidentity(matrix4f out)
{
    out[0][0] = out[1][1] = out[2][2] = out[3][3] = 1.0;
    out[0][1] = out[0][2] = out[0][3] = 0.0;
    out[1][0] = out[1][2] = out[1][3] = 0.0;
    out[2][0] = out[2][1] = out[2][3] = 0.0;
    out[3][0] = out[3][1] = out[3][2] = 0.0;

}
static inline void loadmatrix(matrix4f out, matrix4f in);
static inline void loadmatrix(matrix4f out, matrix4f in)
{
	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++)
			out[i][j] = in[i][j];

}
static int maptoffset3d(int, int, int, int, int, int);
static inline int maptoffset3d(int i , int j, int k, int w, int h, int d)
{
    int result = (((k * h) + i) * w) + j;
    return result;
}


static void setviewport(WINDOW *, int x, int y, int w, int h);
static inline void setviewport(WINDOW * viewport, int x, int y, int w, int h)
{

	viewport->tilex = w/viewport->tilewid;
	viewport->tiley = h/viewport->tilehgt;
	viewport->w = viewport->tilex * viewport->tilewid;
	viewport->h = viewport->tiley * viewport->tilehgt;
	w = viewport->w;
	h = viewport->h;
	viewport->x = x;
	viewport->y = y;

	vpm[0][0] = w/2.0f;  	 vpm[0][1] = 0.0f;         vpm[0][2] = 0.0f;      vpm[0][3] =  (x+w)/2.0f;
	vpm[1][0] = 0.0f;        vpm[1][1] = h/2.0f;       vpm[1][2] = 0.0f;      vpm[1][3] =  (y+h)/2.0f;
	vpm[2][0] = 0.0f;        vpm[2][1] =  0.0f;        vpm[2][2] = 0.5f;      vpm[2][3] =  0.5f;
	vpm[3][0] = 0.0f;        vpm[3][1] =  0.0f;        vpm[3][2] = 0.0f;      vpm[3][3] =  1.0f;
}

static void getfrustum(float, float, float *);
static inline void getfrustum(float fovy, float aspect, float *vv)
{
    // use projection matrix to compute viewvolume extents
	fovy = RADIANS * fovy;
    float l,r,t,b;
    t = tan(fovy/2.0f) * 5.0f; b = -t;
    l = aspect * b; r = aspect * t;

    vv[0] = l, vv[1] = r; vv[2] = b; vv[3] = t;
}

static void setfrustum(FRUSTUM *, float *);
static inline void setfrustum(FRUSTUM *f, float *vv)
{
    f->left = vv[0]; f->right = vv[1]; f->bottom = vv[2]; f->top = vv[3];
}
// could just stick the viewport transform in a 3x3 during setviewport call and do away with ndctowindow
static vector3f ndctowindow(vector3f, WINDOW);
static inline vector3f ndctowindow(vector3f v, WINDOW viewport)
{
    float x, y, w, h;
    float far, near;
    vector3f win;

    // window xform
    x = viewport.x;
    y = viewport.y;
    w = viewport.w;
    h = viewport.h;
    far = w;		// could add a setdepthrange function and set near, far there, e.g. OGL
    near = 0.0f;

    win.x = w/2.0f * v.x + (x + w/2.0f);
    win.y = h/2.0f * v.y + (y + h/2.0f);
    win.z = (far-near)/2.0f * v.z + (far+near)/2.0f;

	win.x = (win.x < x) ? x : win.x;
	win.x = (win.x > x+w) ? x+w : win.x;
	win.y = (win.y < y) ? y : win.y;
	win.y = (win.y > y+h) ? y+h : win.y;
//	win.z = (win.z < 0) ? 0 : win.z;
//	win.z = (win.z > w) ? w : win.z;

    return win;
}
static void setmvm(matrix4f , float [16]);
static inline void setmvm(matrix4f m, float cm[16])
{

    //this is because and where it puts the translate in elemets 12,13,14. 
	//I put it in 3,7,11 assuming row major access of 2x2 matrix
    m[0][0] = cm[0]; m[0][1] = cm[4]; m[0][2] = cm[8];  m[0][3] = cm[12];
    m[1][0] = cm[1]; m[1][1] = cm[5]; m[1][2] = cm[9];  m[1][3] = cm[13];
    m[2][0] = cm[2]; m[2][1] = cm[6]; m[2][2] = cm[10]; m[2][3] = cm[14];
    m[3][0] = cm[3]; m[3][1] = cm[7]; m[3][2] = cm[11]; m[3][3] = cm[15];

}

static void setpm(matrix4f , float [16]);
static inline void setpm(matrix4f m, float cm[16])
{

    m[0][0] = cm[0]; m[0][1] = cm[4]; m[0][2] = cm[8];  m[0][3] = cm[12];
    m[1][0] = cm[1]; m[1][1] = cm[5]; m[1][2] = cm[9];  m[1][3] = cm[13];
    m[2][0] = cm[2]; m[2][1] = cm[6]; m[2][2] = cm[10]; m[2][3] = cm[14];
    m[3][0] = cm[3]; m[3][1] = cm[7]; m[3][2] = cm[11]; m[3][3] = cm[15];

}



#endif
