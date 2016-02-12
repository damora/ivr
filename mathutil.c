//
//  mathutil.c
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 11/29/12.
//  Copyright (c) 2012 Bruce D'Amora. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "utils.h"
#include "mathutil.h"


//---------------------------------------------------------------------------------------------------------------------//
// math functions                                                                                                      //
//---------------------------------------------------------------------------------------------------------------------//

float degtorad(float a)
{
    return (a * M_PI/180.f);
}

inline float dot3f(vector3f v0, vector3f v1)
{
    float prod;
    prod = v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
    return prod;
}
inline float dot4f(vector4f v0, vector4f v1)
{
    float prod;
    prod = v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w;
    return prod;
}
vector3f cross3f(vector3f operand0, vector3f operand1)
{
    vector3f cross;
    cross.x = operand0.y*operand1.z - operand0.z*operand1.y;
    cross.y = operand0.z*operand1.x - operand0.x*operand1.z;
    cross.z = operand0.x*operand1.y - operand0.y*operand1.x;
    return cross;
}
vector3f vec3fadd(vector3f in0, vector3f in1)
{
    vector3f sum;
    sum.x = in0.x + in1.x;
    sum.y = in0.y + in1.y;
    sum.z = in0.z + in1.z;
    
    return sum;
};
vector3f vec3fmult(vector3f in0, float m)
{
    vector3f prod;
    prod.x = in0.x * m;
    prod.y = in0.y * m;
    prod.z = in0.z * m;
    
    return prod;
};
vector4f vec4fadd(vector4f in0, vector4f in1)
{
    vector4f sum;
    sum.x = in0.x + in1.x;
    sum.y = in0.y + in1.y;
    sum.z = in0.z + in1.z;
    sum.w = in0.w + in1.w;
    return sum;
}

vector3f vec3fsub(vector3f in0, vector3f in1)
{
    vector3f sum;
    sum.x = in0.x - in1.x;
    sum.y = in0.y - in1.y;
    sum.z = in0.z - in1.z;
    return sum;
};
vector4f vec4fsub(vector4f in0, vector4f in1)
{
    vector4f out;
    out.x = in0.x - in1.x;
    out.y = in0.y - in1.y;
    out.z = in0.z - in1.z;
    out.w = in0.w - in1.w;
    return out;
};

vector2f vec2fxscalar(vector2f v, float s)
{
    vector2f out;
    
    out.x = v.x * s;
    out.y = v.y * s;
    return out;
}

vector3f normalize3f(vector3f in)
{
    float magnitude;
    vector3f norm;
    magnitude = mag3f(in.x, in.y, in.z);
    norm.x = in.x/magnitude;
    norm.y = in.y/magnitude;
    norm.z = in.z/magnitude;
    
    return norm;
}

vector4f normalize4f(vector4f in)
{
    float magnitude;
    vector4f norm;
    
    magnitude = sqrtf(dot4f(in, in));
    norm.x = in.x/magnitude;
    norm.y = in.y/magnitude;
    norm.z = in.z/magnitude;
    norm.w = in.w/magnitude;
    
    return norm;
}

inline vector3f mat3fxvec3f (matrix3f m, vector3f v)
{
    float tmp[3];
    vector3f out;
    
    for (int j=0; j<3; j++) {
        vector3f row;
        
        row.x = m[j][0];
        row.y = m[j][1];
        row.z = m[j][2];
        
        tmp[j] = dot3f(row, v);
    }
    out.x = tmp[0];
    out.y = tmp[1];
    out.z = tmp[2];
 
    
    return out;
}
void mat3fxmat3f(matrix3f inout, matrix3f in)
{
    matrix3f tmp;
    for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            vector3f col, row;
            row.x = inout[i][0];
            row.y = inout[i][1];
            row.z = inout[i][2];
            col.x = in[0][j];
            col.y = in[1][j];
            col.z = in[2][j];
            
            tmp[i][j] = dot3f(row, col);
        }
    }
    for (int i=0; i<3; i++) {
        inout[i][0] = tmp[i][0];
        inout[i][1] = tmp[i][1];
        inout[i][2] = tmp[i][2];
    }
}
void mat4fxmat4f(matrix4f inout, matrix4f in)
{
    matrix4f tmp;
    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++) {
            vector4f col, row;
            row.x = inout[i][0];
            row.y = inout[i][1];
            row.z = inout[i][2];
            row.w = inout[i][3];
            col.x = in[0][j];
            col.y = in[1][j];
            col.z = in[2][j];
            col.w = in[3][j];
            
            tmp[i][j] = dot4f(row, col);
        }
    }
    for (int i=0; i<4; i++) {
        inout[i][0] = tmp[i][0];
        inout[i][1] = tmp[i][1];
        inout[i][2] = tmp[i][2];
        inout[i][3] = tmp[i][3];
    }
}
vector4f mat4fxvec4f (matrix4f m, vector4f v)
{
    float tmp[4];
    vector4f out;

    for (int j=0; j<4; j++) {
        vector4f row;
        row.x = m[j][0];
        row.y = m[j][1];
        row.z = m[j][2];
        row.w = m[j][3];
        
        tmp[j] = dot4f(row, v);
    }

    out.x = tmp[0];
    out.y = tmp[1];
    out.z = tmp[2];
    out.w = tmp[3];
    
    return out;
}

void vec3ftomatrix3f(matrix3f m, CAMERA cam) {
    m[0][0] = cam.u.x; m[0][1] = cam.v.x; m[0][2] = cam.n.x;
    m[1][0] = cam.u.y; m[1][1] = cam.v.y; m[1][2] = cam.n.y;
    m[2][0] = cam.u.z; m[2][1] = cam.v.z; m[2][2] = cam.n.z;
}

void vec4ftomatrix4f(matrix4f m, CAMERA cam) {
    m[0][0] = cam.u.x; m[0][1] = cam.v.x; m[0][2] = cam.n.x; m[0][3] = -cam.r.u;
    m[1][0] = cam.u.y; m[1][1] = cam.v.y; m[1][2] = cam.n.y; m[1][3] = -cam.r.v;
    m[2][0] = cam.u.z; m[2][1] = cam.v.z; m[2][2] = cam.n.z; m[1][3] = -cam.r.n;
    m[3][0] = 0.0f;    m[3][1] = 0.0f;   m[3][2] = 0.0f;   m[3][3] = 1.0f;
}

float mag3f(float x, float y, float z)
{
    return sqrtf(x*x + y*y + z*z);
}

float mag4f(float x, float y, float z, float w)
{
    return sqrtf(x*x + y*y + z*z + w*w);
}
void gradient3d(vector4f *v, int m, int n, int p)
{
    int i = 0, j = 0, k = 0;
    long long offset=0;
    float dx = 1.0f, dy = 1.0f, dz = 1.0f;
    float x, y, z;
    
    // Internal voxel cases
#pragma omp parallel for private(j, k, x, y, z, offset) firstprivate(v)
    for (i=1; i<m-1; i++) {
        for (j=1; j<n-1; j++) {
            for (k=1; k<p-1; k++) {
                offset = i*j*k;
                (v+offset)->x = x = (grad(i,  j+1,k) - grad(i,   j-1, k))/2.0f*dx;
                (v+offset)->y = y = (grad(i+1,j,  k) - grad(i-1, j,   k))/2.0f*dy;
                (v+offset)->z = z = (grad(i,  j,  k+1) - grad(i, j,   k-1))/2.0f*dz;
                (v+offset)->w = mag3f(x, y, z);
            }
        }
    }
    
    // Border cases: two cases i=0; i=m-1
#pragma omp parallel for private(i, k, x, y, z, offset) firstprivate(v)
    for (j=1; j<n-1; j++) {
        for (k=1; k<p-1; k++) {
            i=0;
            offset = i*j*k;
            (v+offset)->x = x = (grad(i,  j+1,k) - grad(i,   j-1, k))/2.0f*dx;
            (v+offset)->y = y = (grad(1,j,  k) - grad(0, j, k))/2.0f*dy;
            (v+offset)->z = z = (grad(i,  j,  k+1) - grad(i, j,   k-1))/2.0f*dz;
            (v+offset)->w = mag3f(x, y, z);
            
            i=m-1;
            offset = i*j*k;
            (v+offset)->x = x = (grad(i,  j+1,k) - grad(i,   j-1, k))/2.0f*dx;
            (v+offset)->y = y = (grad(m-1,j,  k) - grad(m-2, j,   k))/2.0f*dy;
            (v+offset)->z = z = (grad(i,  j,  k+1) - grad(i, j,   k-1))/2.0f*dz;
            (v+offset)->w = mag3f(x, y, z);
            
        }
    }
    
    // Border cases: two cases j=0; j=n-1
#pragma omp parallel for private(j, k, x, y, z, offset)  firstprivate(v)  
    for (i=1; i<m-1; i++) {
        for (k=1; k<p-1; k++) {
            j=0;
            offset = i*j*k;
            (v+offset)->x = x = (grad(i,  1,k) - grad(i,   0, k))/2.0f*dx;
            (v+offset)->y = y = (grad(i+1,j,  k) - grad(i-1, j,   k))/2.0f*dy;
            (v+offset)->z = z = (grad(i,  j,  k+1) - grad(i, j,   k-1))/2.0f*dz;
            (v+offset)->w = mag3f(x, y, z);
            
            j=n-1;
            offset = i*j*k;
            (v+offset)->x = x = (grad(i,  n-1,k) - grad(i,   n-2, k))/2.0f*dx;
            (v+offset)->y = y = (grad(i+1,j,  k) - grad(i-1, j,   k))/2.0f*dy;
            (v+offset)->z = z = (grad(i,  j,  k+1) - grad(i, j,   k-1))/2.0f*dz;
            (v+offset)->w = mag3f(x, y, z);
            
        }
    }
    
    // Border cases: two cases k=0; k=n-1
#pragma omp parallel for private(j, k, x, y, z, offset) firstprivate(v)   
    for (i=1; i<m-1; i++) {
        for (j=1; j<n-1; j++) {
            k=0;
            offset = i*j*k;
            (v+offset)->x = x = (grad(i,  j+1,k) - grad(i,   j-1, k))/2.0f*dx;
            (v+offset)->y = y = (grad(i+1,j,  k) - grad(i-1, j,   k))/2.0f*dy;
            (v+offset)->z = z = (grad(i,  j,  1) - grad(i, j,   0))/2.0f*dz;
            (v+offset)->w = mag3f(x, y, z);
            
            k=p-1;
            offset = i*j*k;
            (v+offset)->x = x = (grad(i,  j+1,k) - grad(i,   j-1, k))/2.0f*dx;
            (v+offset)->y = y = (grad(i+1,j,  k) - grad(i-1, j,   k))/2.0f*dy;
            (v+offset)->z = z = (grad(i,  j,  k-1) - grad(i, j,   k-2))/2.0f*dz;
            (v+offset)->w = mag3f(x, y, z);
            
        }
    }
}
float grad(int i, int j, int k)
{
    long long result;
    int n, m, p;
    
    n = volume.localdims.w;
    m = volume.localdims.h;
    p = volume.localdims.d;
    
    result = maptoffset3d(i, j, k, n, m, p);
    
    return *(volume.data.f+result);
}
int signof(float f)
{
    return (f > 0.0f) ? 1 : (f < 0.0) ? -1 : 0;
}

void setwvm(matrix3f m, CAMERA cam)
{
    
    m[0][0] = cam.u.x; m[0][1] = cam.u.y; m[0][2] = cam.u.z;
    m[1][0] = cam.v.x; m[1][1] = cam.v.y; m[1][2] = cam.v.z;
    m[2][0] = cam.n.x; m[2][1] = cam.n.y; m[2][2] = cam.n.z;
    
}
void setvvm(matrix4f m, FRUSTUM vv)
{
	float tx = (vv.right+vv.left)/(vv.right-vv.left);
	float ty = (vv.top+vv.bottom)/(vv.top-vv.bottom);
	float tz = (vv.far+vv.near)/(vv.far-vv.near);
    
    m[0][0] = 2.0f/(vv.right-vv.left); m[0][1] = 0.0f;                    m[0][2] = 0.0f; 						m[0][3] = tx;
    m[1][0] = 0.0f;                    m[1][1] = 2.0f/(vv.top-vv.bottom); m[1][2] = 0.0f;						m[1][3] = ty;
    m[2][0] = 0.0f;                    m[2][1] = 0.0f;                    m[2][2] = -2.0f/(vv.far - vv.near); 	m[2][3] = tz;
    m[3][0] = 0.0f;                    m[3][1] = 0.0f;                    m[3][2] = 0.0f; 						m[3][3] = 1.0f;
    
}
void wvmtovp(CAMERA *cam, matrix3f m)
{
    cam->u.x = m[0][0]; cam->u.y = m[0][1]; cam->u.z = m[0][2];
    cam->v.x = m[1][0]; cam->v.y = m[1][1]; cam->v.z = m[1][2];
    cam->n.x = m[2][0]; cam->n.y = m[2][1]; cam->n.z = m[2][2];
}

vector3f worldtoview(vector3f w)
{
    vector3f v;
    
    v = vec3fsub(w, camera.r);
    
    v =  mat3fxvec3f(wvm, v);
    
    return v;
    
}
void setvwm(matrix3f m, CAMERA cam)
{
    m[0][0] = cam.u.x; m[0][1] = cam.v.x; m[0][2] = cam.n.x;
    m[1][0] = cam.u.y; m[1][1] = cam.v.y; m[1][2] = cam.n.y;
    m[2][0] = cam.u.z; m[2][1] = cam.v.z; m[2][2] = cam.n.z;

}

void setortho(float scale, FRUSTUM vf)
{
	float r, l, t, b, n, f;
	float invscale = 1.0f/scale;

	r = vf.right*invscale; l = vf.left*invscale;
	t = vf.top*invscale;   b = vf.bottom*invscale;
	n = vf.near;  f = vf.far;


    pm[0][0] = (2.0)/(r-l);   pm[0][1] = 0.0f;       pm[0][2] = 0.0f;        pm[0][3] = -(r+l)/(r-l);
    pm[1][0] = 0.0f;          pm[1][1] = (2)/(t-b);  pm[1][2] = 0.0f;        pm[1][3] = -(t+b)/(t-b);
    pm[2][0] = 0.0f;          pm[2][1] = 0.0f;       pm[2][2] = -2.0f/(f-n); pm[2][3] = -(n+f)/(f-n);
    pm[3][0] = 0.0f;          pm[3][1] = 0.0f;       pm[3][2] = 0.0f;        pm[3][3] = 1.0f;;

	viewvolume.left = l, viewvolume.right = r; viewvolume.bottom = b; viewvolume.top = t;
}

void setperspective(float fovy, float aspect, float n, float f)
{
	fovy = RADIANS * fovy;
	float fv = (1.0f/tan(fovy/2.0f));
		

	
    pm[0][0] = fv/aspect;  pm[0][1] = 0.0f;    pm[0][2] = 0.0f;               pm[0][3] = 0.0f;
    pm[1][0] = 0.0f;       pm[1][1] = fv;      pm[1][2] = 0.0f;               pm[1][3] = 0.0f;
    pm[2][0] = 0.0f;       pm[2][1] = 0.0f;    pm[2][2] = (f+n)/(n-f);        pm[2][3] = (2.0f*n*f)/(n-f);
    pm[3][0] = 0.0f;       pm[3][1] = 0.0f;    pm[3][2] = -1.0f;	  		  pm[3][3] = 0.0f;;



	// use projection matrix to compute viewvolume extents
	float l,r,t,b;
	t = tan(fovy/2.0f) * 5.0f; b = -t;
	l = aspect * b; r = aspect * t;

	viewvolume.left = l, viewvolume.right = r; viewvolume.bottom = b; viewvolume.top = t;

}


vector3f viewtoworld(vector3f v)
{
    vector3f w;

    
    v = mat3fxvec3f(vwm, v);
    
    w = vec3fadd(v, camera.r);

    return w;
}

int histogram(int **hist, float *values, float *hmin, float *hmax, int width, int height, int depth)
{
    int indx, hsize;
    long long i;
    long long maxvalues = width * height * depth;

    
    *hmin = MAXFLOAT;
    *hmax = -MAXFLOAT;
    
    for (i=0; i<maxvalues; i++) {
        *hmin = fmin(*hmin, values[i]);
        *hmax = fmax(*hmax, values[i]);
    }
    
    hsize = (int) ((*hmax - *hmin) + 1);
   *hist = (int *) malloc((hsize) * sizeof(int));
	assert(*hist != NULL);

	for (int j=0; j<hsize; j++) (*hist)[j] = 0;
    for (i=0; i<maxvalues; i++) {
        indx = (int) (values[i] - *hmin);
        (*hist)[indx]  += 1;
    }
    
    return hsize;
}

void movecamera(vector3f d)
{
	d.x = -d.x;
	d.y = -d.y;
    
    camera.r = vec3fadd(vrp, d);

    return;
}

void zoomcamera(float scale)
{
    float invscale = 1.0f/scale;

    viewvolume.right = invscale * frustum.right;
    viewvolume.left = invscale * frustum.left;
    viewvolume.top = invscale * frustum.top;
    viewvolume.bottom = invscale * frustum.bottom;
    viewvolume.near = invscale * frustum.near;
    viewvolume.far = invscale * frustum.far;

	
}

void inversemat3f(matrix3f m)
{
    vector3f rows[3];
    
    for (int i = 0; i<3; i++) {
        rows[i].x = m[i][0];
        rows[i].y = m[i][1];
        rows[i].z = m[i][2];
    }
    for (int j = 0; j<3; j++) {
        m[0][j] = rows[j].x;
        m[1][j] = rows[j].y;
        m[2][j] = rows[j].z;
    }
}
void transposerotation(matrix3f m3f)
{
	vector3f rows[3];

	for (int i=0; i<3; i++) {
		rows[i].x = m3f[i][0];
		rows[i].y = m3f[i][1];
		rows[i].z = m3f[i][2];
	}
	for (int i=0; i<3; i++) {
		m3f[0][i] = rows[i].x;
		m3f[1][i] = rows[i].y;
		m3f[2][i] = rows[i].z;
	}
}
void  setrotation(matrix4f m)
{ 
	int i,j;
    for (i=0; i<4; i++)
      for (j=0; j<4; j++)
         rotation[i][j] = m[i][j];

	setrotate(0.0f, 1.0, 0.0, 0.0);
    setrotate(0.0f, 0.0, 1.0, 0.0);
    setrotate(0.0f, 0.0, 0.0, 1.0);

};

void setrotate(float ang, float x, float y, float z)
{
    matrix4f m = {{1.0, 0.0, 0.0, 0.0},
                  {0.0, 1.0, 0.0, 0.0},
                  {0.0, 0.0, 1.0, 0.0},
                  {0.0, 0.0, 0.0, 1.0}};
    float radians;
    float cosine, sine;
    float t;

    radians =   ang * RADIANS;
    sine = sin(radians);
    cosine = cos(radians);

    t = x * x;
    m[0][0] = t + cosine * (1 - t);
    m[2][1] = -x * sine;
    m[1][2] = x * sine;

    t = y * y;
    m[1][1] = t + cosine * (1 - t);
    m[2][0] = y * sine;
    m[0][2] = -y * sine;

    t = z * z;
    m[2][2] = t + cosine * (1 - t);
    m[1][0] = -z * sine;
    m[0][1] = z * sine;

    mat4fxmat4f(m, rotation);
}

		
void rotatecamera()
{

    matrix3f m3f;
   
    camera.n.x = dir.x; camera.n.y = dir.y; camera.n.z = dir.z;
	m3f[0][0] = rotation[0][0]; m3f[0][1] = rotation[0][1]; m3f[0][2] = rotation[0][2];
	m3f[1][0] = rotation[1][0]; m3f[1][1] = rotation[1][1]; m3f[1][2] = rotation[1][2];
	m3f[2][0] = rotation[2][0]; m3f[2][1] = rotation[2][1]; m3f[2][2] = rotation[2][2];
    camera.r = mat3fxvec3f(m3f, camera.r);
    camera.n = mat3fxvec3f(m3f, dir);
    camera.n = normalize3f(camera.n);
    
    camera.v = mat3fxvec3f(m3f, up);
    camera.v = normalize3f(camera.v);
    camera.u = cross3f(camera.n, camera.v);
    //camera.u = normalize3f(camera.u); // don't really need to normalize because n and v are already normalized
    setvwm(vwm, camera);
    setwvm(wvm, camera);
    
    return;
}



vector4f axisangtoquat(vector4f v, float angle)
{
    float sinang;
    
    angle = degtorad(angle);
    angle *= 0.5f;
    
    vector4f vn = {v.x, v.y, v.z, v.w};
    
    vn = normalize4f(vn);
    
    sinang = sinf(angle);
    
    vn.x = (v.x * sinang);
    vn.y = (v.y * sinang);
    vn.z = (v.z * sinang);
    vn.w = cosf(angle);;
    
    return vn;
    
}
void quatomatrix3f(vector4f v, matrix3f mat)
{
	float xx      = v.x * v.x;
   	float xy      = v.x * v.y;
   	float xz      = v.x * v.z;
   	float xw      = v.x * v.w;

   	float yy      = v.y * v.y;
   	float yz      = v.y * v.z;
   	float yw      = v.y * v.w;

   	float zz      = v.z * v.z;
   	float zw      = v.z * v.w;

    mat[0][0]  = 1.0f - 2.0f * ( yy + zz );
    mat[0][1]  =     2.0f * ( xy - zw );
    mat[0][2]  =     2.0f * ( xz + yw );

    mat[1][0]  =     2.0f * ( xy + zw );
    mat[1][1]  = 1.0f - 2.0f * ( xx + zz );
    mat[1][2]  =     2.0f * ( yz - xw );

    mat[2][0]  =     2.0f * ( xz - yw );
    mat[2][1]  =     2.0f * ( yz + xw );
    mat[2][2]  = 1.0f - 2.0f * ( xx + yy );
}
void quatomatrix4f(vector4f v, matrix4f mat)
{
    float xx      = v.x * v.x;
    float xy      = v.x * v.y;
    float xz      = v.x * v.z;
    float xw      = v.x * v.w;
    
    float yy      = v.y * v.y;
    float yz      = v.y * v.z;
    float yw      = v.y * v.w;
    
    float zz      = v.z * v.z;
    float zw      = v.z * v.w;
    mat[0][0]  = 1.0f - 2.0f * ( yy + zz );
    mat[0][1]  =     2.0f * ( xy - zw );
    mat[0][2]  =     2.0f * ( xz + yw );

    mat[1][0]  =     2.0f * ( xy + zw );
    mat[1][1]  = 1.0f - 2.0f * ( xx + zz );
    mat[1][2]  =     2.0f * ( yz - xw );

    mat[2][0]  =     2.0f * ( xz - yw );
    mat[2][1]  =     2.0f * ( yz + xw );
    mat[2][2]  = 1.0f - 2.0f * ( xx + yy );


	mat[0][3]  = mat[1][3] = mat[2][3] = mat[3][0] = mat[3][1] = mat[3][2]=  0.0f;
    mat[3][3] = 1.0f;
}

void quattoaxisang(float  *v, float  *axis, float *angle)
{
    float x,y,z,w;
    float scale, sinangle;
    
    x = v[0];
    y = v[1];
    z = v[2];
    w = v[3];
    
    scale = sqrtf(x*x + y*y + z*z);
    
    axis[0] = x/scale;
    axis[1] = y/scale;
    axis[2] = z/scale;
    
    *angle = acosf(w) * 2.0f * RADIANS;
	sinangle  = sqrtf( 1.0f - w*w);
	if (fabs(sinangle) < 0.0005f)  sinangle = 1;

	axis[0] /= sinangle;
	axis[1] /= sinangle;
	axis[2] /= sinangle;
    
    return;
}
vector4f conjugate(vector4f quat)
{
    vector4f conj;
    
    conj.x = -quat.x;
    conj.y = -quat.y;
    conj.z = -quat.z;
    conj.w = quat.w;
    
    return conj;
}

vector4f rotvector (vector4f a, vector4f b)
{

	b = normalize4f(b);
    
	vector4f quat, conquat, out;
	conquat.x = -b.x;
	conquat.y = -b.y;
	conquat.z = -b.z;
	conquat.w =  b.w;
    quat = multquat(b, a);
    out = multquat(quat, conquat);
       

	return out;
}

vector4f multquat(vector4f a, vector4f b)
{
    vector4f q;
    
   
    q.x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
    q.y = a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x;
    q.z = a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w;
    q.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
    return q;
}
void axisangtomatrix4(vector4f q, matrix4f m)
{
    float cosang, sinang;
    vector3f a = {{q.x}, {q.y}, {q.z}};

    
    a = normalize3f(a);
    
    cosang = cos(degtorad(q.w));
    sinang = sin(degtorad(q.w));
    
    
    m[0][0] = cosang + a.x*a.x * (1.0f - cosang);
    m[1][0] = a.x*a.y * (1.0f - cosang) - a.z * sinang;
    m[2][0] = a.x*a.z * (1.0f - cosang) + a.y * sinang;
    m[3][0] = 0.0f;
    
    
    m[0][1] = a.x*a.y * (1.0f - cosang) + a.z * sinang;
    m[1][1] = cosang + a.y*a.y * (1.0f - cosang);
    m[2][1] = a.y*a.z * (1.0f - cosang) - a.x * sinang;
    m[3][1] = 0.0f;
    
    m[0][2] = a.x*a.z * (1.0f - cosang) - a.y * sinang;
    m[1][2] = a.y*a.z * (1.0f - cosang) + a.x * sinang;
    m[2][2] = cosang + a.z*a.z * (1.0f - cosang);
    m[3][2] = 0.0f;
    
    m[0][3] = 0.0f;
    m[1][3] = 0.0f;
    m[2][3] = 0.0f;
    m[3][3] = 1.0f;
  
    return;
}
/*
void
axisangtomatrix4(vector4f q, matrix4f  m)
{
    float cosang, sinang;
    vector3f a = {q.x, q.y, q.z};


    a = normalize3f(a);

    cosang = cos(degtorad(q.w));
    sinang = sin(degtorad(q.w));


    m[0][0] = cosang + a.x*a.x * (1.0f - cosang);
    m[0][1] = a.x*a.y * (1.0f - cosang) - a.z * sinang;
    m[0][2] = a.x*a.z * (1.0f - cosang) + a.y * sinang;
    m[0][3] = 0.0f;


    m[1][0] = a.x*a.y * (1.0f - cosang) + a.z * sinang;
    m[1][1] = cosang + a.y*a.y * (1.0f - cosang);
    m[1][2] = a.y*a.z * (1.0f - cosang) - a.x * sinang;
    m[1][3] = 0.0f;

    m[2][0] = a.x*a.z * (1.0f - cosang) - a.y * sinang;
    m[2][1] = a.y*a.z * (1.0f - cosang) + a.x * sinang;
    m[2][2] = cosang + a.z*a.z * (1.0f - cosang);
    m[2][3] = 0.0f;
   
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
 
    return;
}
*/

void eaxisangtomatrix4(vector4f q, matrix4f m)
{
    float cosang, sinang;
    vector3f a = {{q.x}, {q.y}, {q.z}};
    
    
    a = normalize3f(a);
    
    cosang = cos(degtorad(q.w));
    sinang = -sin(degtorad(q.w));
    
    
    m[0][0] = cosang + a.x*a.x * (1.0f - cosang);
    m[1][0] = a.x*a.y * (1.0f - cosang) - a.z * sinang;
    m[2][0] = a.x*a.z * (1.0f - cosang) + a.y * sinang;
    m[3][0] = 0.0f;
    
    
    m[0][1] = a.x*a.y * (1.0f - cosang) + a.z * sinang;
    m[1][1] = cosang + a.y*a.y * (1.0f - cosang);
    m[2][1] = a.y*a.z * (1.0f - cosang) - a.x * sinang;
    m[3][1] = 0.0f;
    
    m[0][2] = a.x*a.z * (1.0f - cosang) - a.y * sinang;
    m[1][2] = a.y*a.z * (1.0f - cosang) + a.x * sinang;
    m[2][2] = cosang + a.z*a.z * (1.0f - cosang);
    m[3][2] = 0.0f;
    
    m[0][3] = 0.0f;
    m[1][3] = 0.0f;
    m[2][3] = 0.0f;
    m[3][3] = 1.0f;
    
    
    return;
}

void axisangtomatrix3(vector4f q, matrix3f m)
{
    float cosang, sinang;
    vector3f a = {{q.x}, {q.y}, {q.z}};
    
    
    a = normalize3f(a);
    
    cosang = cos(degtorad(q.w));
    sinang = sin(degtorad(q.w));
    
    
    m[0][0] = cosang + a.x*a.x * (1.0f - cosang);
    m[1][0] = a.x*a.y * (1.0f - cosang) - a.z * sinang;
    m[2][0] = a.x*a.z * (1.0f - cosang) + a.y * sinang;
    
    
    m[0][1] = a.x*a.y * (1.0f - cosang) + a.z * sinang;
    m[1][1] = cosang + a.y*a.y * (1.0f - cosang);
    m[2][1] = a.y*a.z * (1.0f - cosang) - a.x * sinang;
    
    m[0][2] = a.x*a.z * (1.0f - cosang) - a.y * sinang;
    m[1][2] = a.y*a.z * (1.0f - cosang) + a.x * sinang;
    m[2][2] = cosang + a.z*a.z * (1.0f - cosang);
    
    return;
}

void setuvn(vector3f vdir, vector3f up)
{
    float dotprod;
    vector3f upp;
    
      
    camera.n = normalize3f(vdir);
    dotprod = dot3f(up, camera.n);
    
    upp.x = up.x - dotprod * camera.n.x;
    upp.y = up.y - dotprod * camera.n.y;
    upp.z = up.z - dotprod * camera.n.z;
    
    camera.v = normalize3f(upp);
    camera.u = cross3f(camera.n, camera.v);
 
}


float computedelta(int indx, float celldelta, float coord, float offset, float deltabbox)
{
    float delta = 0.0f;
    
    delta = (offset - coord)/(deltabbox) - (indx * celldelta);
    return delta;
}


void composemvm(matrix4f  rotation, float scale, vector3f delta)
{
    /* view transforms */
	loadidentity(mvm);

	fprintf(stderr,"DELTAS: %f, %f, %f\n", delta.x, delta.y, e.z);
	vector3f ee, a, u;
	ee.x = -delta.x; ee.y = -delta.y; ee.z = -e.z;
	a.x = -delta.x; a.y = -delta.y; a.z=0;
	u.x = 0; u.y = 1.0f; u.z =0.0f;
	lookat(ee, a, u);

	// move camera
	delta.z = 0.0f;

	// rotate the camera
	transposemat4f(rotation);
    fprintf(stderr,"My Rotation matrix\n");
    for (int i=0; i<4; i++)
        fprintf(stderr,"%f  %f  %f  %f\n", rotation[i][0], rotation[i][1], rotation[i][2], rotation[i][3]);

	mat4fxmat4f(mvm,rotation);
    fprintf(stderr, "My ModelView\n");
    for (int i=0; i<4; i++) {
        fprintf(stderr,"%f, %f, %f, %f\n", mvm[i][0], mvm[i][1], mvm[i][2], mvm[i][3]);
    }


	// zooom camera
#ifdef ORTHO
	setortho(scale, viewvolume);
#else
	float invscale = 1.0f/scale;
	float fovy = 43.6028;
	setperspective(invscale*fovy, 1.0f, 0.1f, 100.0f);
#endif

}


// update all coordinates associated with the volumes bounding box
void updatevolbbox()
{
    vector3f  wmin, wmax, mvmin, mvmax;
	matrix4f ctm;

	loadmatrix(ctm, pm);

	// pmv is the concatenated model-view and projection matrix to transform from world to NDC
	mat4fxmat4f(ctm,mvm); // concatentated projection x modelview matrix = ctm

	// now convert all vertices of aabb for volume to NDC
	for (int i=0; i<8; i++) {
		vector4f *v = (vector4f *) &aabb.verts[i];
		vector4f *mc = (vector4f *) &aabb.mverts[i];
		vector4f *pc = (vector4f *) &aabb.pverts[i];

		*mc = mat4fxvec4f(mvm, *v);
		*pc = mat4fxvec4f(ctm, *v);
	}


	// now convert to window coordinates by applying viewport xform
	wmin.x = wmin.y = wmin.z = mvmin.x = mvmin.y = mvmin.z = MAXFLOAT;
	wmax.x = wmax.y = wmax.z = mvmax.x = mvmax.y = mvmax.z = -MAXFLOAT;
	for (int i=0; i<8; i++) {
		float w = aabb.pverts[i].w;
		vector3f mvc = {{aabb.mverts[i].x}, {aabb.mverts[i].y}, {aabb.mverts[i].z}};
		vector4f ndc = {aabb.pverts[i].x/w, aabb.pverts[i].y/w, aabb.pverts[i].z/w, 1.0f};
		vector4f win;

		win = mat4fxvec4f(vpm, ndc);
		wmin.x = fmin(wmin.x, win.x);
		wmin.y = fmin(wmin.y, win.y);
		wmin.z = fmin(wmin.z, win.z);
		wmax.x = fmax(wmax.x, win.x);
		wmax.y = fmax(wmax.y, win.y);
		wmax.z = fmax(wmax.z, win.z);

		mvmin.x = fmin(mvmin.x, mvc.x);
		mvmin.y = fmin(mvmin.y, mvc.y);
		mvmin.z = fmin(mvmin.z, mvc.z);
		mvmax.x = fmax(mvmax.x, mvc.x);
		mvmax.y = fmax(mvmax.y, mvc.y);
		mvmax.z = fmax(mvmax.z, mvc.z);

	}
		
    wmin.x = (wmin.x < viewport.x) ? viewport.x : wmin.x; 
	wmax.x = (wmax.x > viewport.x+viewport.w) ? viewport.x+viewport.w : wmax.x;
    wmin.x = (wmin.x > viewport.x+viewport.w) ? viewport.x+viewport.w : wmin.x;
	wmax.x = (wmax.x < viewport.x) ? viewport.x : wmax.x;

    wmin.y = (wmin.y < viewport.y) ? viewport.y : wmin.y; 
	wmax.y = (wmax.y > viewport.y+viewport.h) ? viewport.y+viewport.h : wmax.y;
    wmin.y = (wmin.y > viewport.y+viewport.h) ? viewport.y+viewport.h : wmin.y;
	wmax.y = (wmax.y < viewport.y) ? viewport.y : wmax.y;


    aabb.wincoord.x = wmin.x; aabb.wincoord.y = wmin.y; aabb.wincoord.z = wmax.z;
    aabb.wincoord.w = wmax.x - wmin.x;
    aabb.wincoord.h = wmax.y - wmin.y;

    // just take the midpoint of distances to represent avg distance from eye
    aabb.wincoord.d = (wmax.z + wmin.z)/2.0f;

	aabb.mvmin.x = mvmin.x; aabb.mvmin.y = mvmin.y; aabb.mvmin.z = mvmin.z;
	aabb.mvmax.x = mvmax.x; aabb.mvmax.y = mvmax.y; aabb.mvmax.z = mvmax.z;
	PRINTDEBUG("MINMAX: %f, %f, %f, %f, %f, %f\n", mvmin.x, mvmin.y, mvmin.z, mvmax.x, mvmax.y, mvmax.z);

	return;
}

inline float sign(float x) {return (x >= 0.0f) ? +1.0f : -1.0f;}
inline float norm(float a, float b, float c, float d) {return sqrt(a * a + b * b + c * c + d * d);}
/*
void mat4ftoquat(matrix4f m, float * q)
{
	q[0] = ( m[0][0] + m[1][1] + m[2][2] + 1.0f) / 4.0f;
	q[1] = ( m[0][0] - m[1][1] - m[2][2] + 1.0f) / 4.0f;
	q[2] = (-m[0][0] + m[1][1] - m[2][2] + 1.0f) / 4.0f;
	q[3] = (-m[0][0] - m[1][1] + m[2][2] + 1.0f) / 4.0f;
	if(q[0] < 0.0f) q[0] = 0.0f;
	if(q[1] < 0.0f) q[1] = 0.0f;
	if(q[2] < 0.0f) q[2] = 0.0f;
	if(q[3] < 0.0f) q[3] = 0.0f;
	q[0] = sqrt(q[0]);
	q[1] = sqrt(q[1]);
	q[2] = sqrt(q[2]);
	q[3] = sqrt(q[3]);
	if(q[0] >= q[1] && q[0] >= q[2] && q[0] >= q[3]) {
		q[0] *= +1.0f;
		q[1] *= sign(m[2][1] - m[1][2]);
		q[2] *= sign(m[0][2] - m[2][0]);
		q[3] *= sign(m[1][0] - m[0][1]);
	} else if(q[1] >= q[0] && q[1] >= q[2] && q[1] >= q[3]) {
		q[0] *= sign(m[2][1] - m[1][2]);
		q[1] *= +1.0f;
		q[2] *= sign(m[1][0] + m[0][1]);
		q[3] *= sign(m[0][2] + m[2][0]);
	} else if(q[2] >= q[0] && q[2] >= q[1] && q[2] >= q[3]) {
		q[0] *= sign(m[0][2] - m[2][0]);
		q[1] *= sign(m[1][0] + m[0][1]);
		q[2] *= +1.0f;
		q[3] *= sign(m[2][1] + m[1][2]);
	} else if(q[3] >= q[0] && q[3] >= q[1] && q[3] >= q[2]) {
		q[0] *= sign(m[1][0] - m[0][1]);
		q[1] *= sign(m[2][0] + m[0][2]);
		q[2] *= sign(m[2][1] + m[1][2]);
		q[3] *= +1.0f;
	} else {
		printf("coding error\n");
	}
	float r = norm(q[0], q[1], q[2], q[3]);
	q[0] /= r;
	q[1] /= r;
	q[2] /= r;
	q[3] /= r;
}
*/
void  lookat(vector3f eye, vector3f center, vector3f up) 
{
	vector3f s,u,f;
	f.x = center.x - eye.x; f.y = center.y - eye.y; f.z = center.z - eye.z;
	f = normalize3f(f);
	u.x = up.x; u.y = up.y; u.z = up.z;
	u = normalize3f(u);
	s = cross3f(f,u);
	s = normalize3f(s);
	u = cross3f(s, f);
	matrix4f m = 	{{s.x, s.y, s.z, 0.0f},
			 		{u.x, u.y, u.z, 0.0f},
			 		{-f.x, -f.y, -f.z, 0.0f},
			 		{ 0.0f, 0.0f, 0.0f, 1.0f}};

	matrix4f t = {{1.0f, 0.0f, 0.0f, -eye.x},
				  {0.0f, 1.0f, 0.0f, -eye.y},
				  {0.0f, 0.0f, 1.0f, -eye.z},
				  {0.0f, 0.0f, 0.0f, 1.0f}};

	mat4fxmat4f(mvm,m);
	mat4fxmat4f(mvm,t);
	fprintf(stderr, "MY LOOKAT MATRIX\n");
	for (int i=0; i<4; i++)
		fprintf(stderr,"%f, %f, %f, %f\n", mvm[i][0], mvm[i][1], mvm[i][2], mvm[i][3]);
}

void transposemat4f(matrix4f r)
{
	matrix4f tmp;

	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++) 
			tmp[i][j] = r[j][i];
	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++) 
			r[i][j] = tmp[i][j];
}
		
/************* Begin of buildray Function ****************************/
/***********************************************************************/
/*                                                                     */
/* FUNCTION: buildray                                                */
/* DESCRIPTION: builds ray through ith,jth pixel in world coords       */
/* INPUT:  i,j - row, and column                                       */
/* OUTPUT: ray - RAY                                                   */
/***********************************************************************/

void buildray(float du, float dv, vector3f *i,vector3f *j, ray3f *ray)
{
	vector3f  eye, uvn, trans;
	matrix3f ctm;

  	uvn.x = winwidth - camera.e.u;                      /* build i,j th ray */
  	uvn.y = winheight - camera.e.v;
  	uvn.z  = -camera.e.n;
                                       /* compute row delta and column delta */
  	i->x = i->z = 0.0f; i->y = dv;
  	j->x = du; j->y = j->z = 0.0f;


	// get view translation factor
	trans.x = mvm[0][3]; trans.y = mvm[1][3]; trans.z = 0.0f;

	// set start position, eye represents camera eye in world cooords translated by vrp
	eye = mat3fxvec3f(vwm, camera.e);
	eye = vec3fadd(eye, camera.r);

  	ray->orig.x = eye.x - trans.x;
  	ray->orig.y = eye.y - trans.y;
  	ray->orig.z = eye.z - trans.z;

	// transpose to get inverse rotation
	for (int i=0; i<3; i++)
		for (int j=0; j<3; j++)
			ctm[i][j] = mvm[j][i];
	// rotate ray origin by inverse current modelview
	ray->orig = mat3fxvec3f(ctm, ray->orig);

	// concatenate with view to world transform matrix to transform ray with a single matrix multiply
  	mat3fxmat3f(ctm, vwm);
    // compute direction vector 
  	ray->dir = mat3fxvec3f(ctm, ray->dir);

    // compute i,j increment values 
  	*i = mat3fxvec3f(ctm,*i);
  	*j = mat3fxvec3f(ctm,*j);
}

/************* End of buildray  Function*******************************/

