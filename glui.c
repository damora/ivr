//
//  glui.c
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 11/30/12.
//  Copyright (c) 2012 Bruce D'Amora. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <types.h>
#include <mathutil.h>
#include <profile.h>
#include <glui.h>
#include <utils.h>
#include <trackball.h>

#ifdef LOCAL
#ifdef __linux
#include <GL/gl.h>
#include <GL/glut.h>
#else
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#endif

extern void freeall(void);
extern void createlocalimage(void);
//extern CAMERA camera;
extern WINDOW viewport;
//extern AABB aabb;
extern matrix3f vwm;
extern matrix4f mvm;
extern matrix4f pm;
extern matrix4f rotation;
extern FRUSTUM frustum, viewvolume;

/* Draw the cube.*/
extern GLfloat vertices[][3];

// temporary for test purposes
/* These functions implement a simple trackball-like motion control. */
#define bool int /* remove for C++ */
#define false 0
#define true 1


int         	startx, starty, lastx, lasty;
int         	camerazoom = 0;
int         	cameramove = 0;
int        		camerarotate = 1;
int        		boundingbox = 0;
int        		boundingvol = 0;
static bool    	mousemove = false;
static bool    	redraw = false;
static float    lastquat[4] = {0.0, 0.0, 0.0, 0.0};
static float    curquat[4];
extern int     	winwidth, winheight;
extern vector3f	delta;
extern float    angle, axis[4];
extern float    scale;

void resettransforms()
{
	matrix4f m;

    angle = 0.0f;
    scale = 1.0f;
    setlastxy(0, 0);
    delta.x = delta.y = 0.0f;
#ifdef ORTHO
	delta.z = 0.0f;
#else
	delta.z = -5.0f;
#endif
    trackball(curquat, 0.0, 0.0, 0.0, 0.0);
 	build_rotmatrix(m, curquat);
    setrotation(m);

}


void polygon(int a, int b, int c , int d)
{
    /* draw a polygon via list of vertices */
    glBegin(GL_LINE_LOOP);
    
    glVertex3fv(vertices[a]);
    glVertex3fv(vertices[b]);
    glVertex3fv(vertices[c]);
    glVertex3fv(vertices[d]);

    glEnd();
}
void showbbox()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(0.0f, 1.0f, 0.0f);
	float x0, y0, x1, y1;
	x0 = aabb.wincoord.x; x1 = x0 + aabb.wincoord.w;
	y0 = aabb.wincoord.y; y1 = y0 + aabb.wincoord.h;
	glRectf(x0, y0, x1, y1);
}
void showbvol()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(1.0f, 1.0f, 1.0f);
    /* map vertices to faces */
    polygon(1,0,3,2);
    polygon(3,7,6,2);
    polygon(7,3,0,4);
    polygon(2,6,5,1);
    polygon(4,5,6,7);
    polygon(5,4,0,1);

}

  
    
void keyboard(unsigned char key, int x, int y)
{
	y = winheight - y;
	setlastxy(x,y);
    switch(key) {
        case 27:
            freeall();
            exit(1);
            break;
        case 'm':
            cameramove = 1;
			camerazoom = 0;
			camerarotate = 0;
            break;
            
        case 's':
            camerazoom = 1;
			cameramove = 0;
			camerarotate = 0;
            break;
        case 'r':
            camerarotate = 1;
			camerazoom = 0;
			cameramove = 0;
            break;
        case 'b':
            boundingbox ^= 1;
            break;
        case 'v':
            boundingvol ^= 1;
            break;
            
        default:
            break;
    }
    
//	resettransforms();
    glutPostRedisplay();
}

void reshape(int w, int h)
{
	float fwidth = FRUSTUMWIDTH;
	float fheight = FRUSTUMHEIGHT;

	if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    winwidth = w;
    winheight = h;
	float aspect = (float) winwidth / (float) winheight;

	fprintf(stderr,"RESHAPE\n");
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef ORTHO
  	if (aspect > 1.0f) {
		fwidth *= aspect;
  	}
  	else {
    	aspect = 1.0f/aspect;
		fheight *= aspect;
  	}
	frust.bottom =  -fheight/2.0f;
	frust.top 	=  	-frust.bottom;
	frust.left = 	-fwidth/2.0f;
	frust.right =   -frust.left;
	frust.near =    -2.0f;
	frust.far =      2.0f;
   	glOrtho(frust.left, frust.right, frust.bottom, frust.top, frust.near, frust.far);
#else
    gluPerspective(43.6028, aspect, 0.1f, 100.0f);
#endif

	// initialize the server (if we are running this we have integrated client/server)
	initcamera();
	setviewport(&viewport, 0, 0, w, h);
	initcolbufs();

	matrix4f m;
    glGetFloatv(GL_PROJECTION_MATRIX, (float *)m);
    fprintf(stderr, "GL Projection\n");
    for (int i=0; i<4; i++)
        fprintf(stderr,"%f, %f, %f, %f\n", m[0][i],m[1][i],m[2][i],m[3][i]);

    fprintf(stderr, "My Projection\n");
    for (int i=0; i<4; i++)
        fprintf(stderr,"%f, %f, %f, %f\n", pm[i][0],pm[i][1],pm[i][2],pm[i][3]);

}


void display()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// we're modifying view of bounding box, bounding volume and the camera parameters used to 
	// transfrom rays from view to world coordinates
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// move
	gluLookAt(-delta.x, -delta.y, -e.z, -delta.x, -delta.y, 0, 0, 1, 0);
	fprintf(stderr,"GL LOOKAT  MATRIX\n");
	float l[4][4];
	glGetFloatv(GL_MODELVIEW_MATRIX,(float *)l);
	for (int i=0; i<4; i++) 
		fprintf(stderr,"%f  %f  %f  %f\n", l[0][i], l[1][i], l[2][i], l[3][i]);


	// the synthetic camera eye pos is already specified a distance away from VRP
	// in fact we could potentially ignore the delta.z within movecamera
	delta.z=0;

    // rotate
    glMultMatrixf((float *) rotation);

	// scale
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#ifdef ORTHO
	glOrtho(viewvolume.left, viewvolume.right, viewvolume.bottom, viewvolume.top, viewvolume.near, viewvolume.far);
#else
	float invscale = 1.0f/scale;
	gluPerspective(invscale*43.6028, 1.0f, 0.1f, 100.0f);

	float pp[4][4];
	glGetFloatv(GL_PROJECTION_MATRIX,(float *)pp);
	fprintf(stderr,"GL PROJECTION  MATRIX\n");
	for (int i=0; i<4; i++) 
		fprintf(stderr,"%f  %f  %f  %f\n", pp[0][i], pp[1][i], pp[2][i], pp[3][i]);
#endif

	glMatrixMode(GL_MODELVIEW);
	
	// get the current transform matrix since OpenGL has it
	float cm[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, cm);
	fprintf(stderr,"GL MODELVIEW MATRIX\n");
	int cnt;
	for (int i=0; i<4; i++) {
		fprintf(stderr,"%f  %f  %f  %f\n", cm[i], cm[i+4], cm[i+8], cm[i+12]);
	}

	setmvm(mvm, cm);

	// with every view change the volume (or subvolumes) bbox projects to differnt extent in window coordinates
	// we use those window coordinate extents to constraint the start and end indices for casting rays so we only 
	// visit screen pixels that have a chance of being "colored"
    updatevolbbox(0);

    TIME(RENDER, start);
	// set identity modelview xform
	glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
	// set non-perspective projection so glRasterPos for DrawPixels won't get clobbered
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
    glLoadIdentity();
	glOrtho(0.0, winwidth, 0.0, winheight, 0.0, 512.0f);
    createlocalimage();
	if (boundingbox) showbbox();
	// return projection to whatever it was
    glPopMatrix();
	// set modelview xform mode and return modelview xfrom to whatever it was
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	// draw a bounding volume if appropriate
    if (boundingvol) showbvol();
    glutSwapBuffers();
    TIME(RENDER, stop);
    PERFORMANCE(start, stop, framenum, elapsed, avg);
	PRINTDEBUG("start,stop=%ld, %ld\n", start, stop);
     
}

void mousebutton(int button, int state, int x, int y)
{
	y = winheight - y;
    if(button==GLUT_RIGHT_BUTTON) exit(0);
    if(button==GLUT_LEFT_BUTTON)
    {
        switch(state)
        {
            case GLUT_DOWN:
                startmotion( x,y);
                break;
            case GLUT_UP:
                stopmotion( x,y);
                break;
        }
    }
}
void mousemotion(int x, int y)
{
	matrix4f m;
    
	y = winheight - y;
    if (!mousemove) return;
    if (camerarotate) {
        if (lastx != x || lasty != y) {
            float p1x = 2.0f * ((float) (winwidth-lastx)/winwidth) - 1.0f;
            float p1y = 2.0f * ((float) (winheight-lasty)/winheight) - 1.0f;
            float p2x = 2.0f * ((float) (winwidth-x)/winwidth) - 1.0f;
            float p2y = 2.0f * ((float) (winheight-y)/winheight) - 1.0f;
            trackball(lastquat,p1x, p1y, p2x, p2y);
            add_quats(lastquat, curquat, curquat);
			build_rotmatrix(m, curquat);
			setrotation(m);
        }
    }


    else if (cameramove) {
		float dx = (float) (x - lastx)/winwidth;
		float dy = (float) (y - lasty)/winheight;
        delta.x += dx;
        delta.y += dy;
     //   delta.z = 0.0f; // delta.z is initialized once as there is currently no interface to move  along the Z axis
    }

    else if (camerazoom) {
        float dy = (float) (y - lasty);

        scale  +=  dy/(float)winheight;
        scale = (scale < ZOOMMIN) ? ZOOMMIN : scale;
        scale = (scale > ZOOMMAX) ? ZOOMMAX : scale;

    }

    setlastxy(x, y);
    glutPostRedisplay();
}


void startmotion(int x, int y)
{
    mousemove = true;
    redraw = false;
    setlastxy(x,  y);
 
}

void stopmotion(int x, int y)
{
    mousemove = false;
  	redraw = true;
    setlastxy(x,   y);
}
#endif

