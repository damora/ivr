//
//  ivrclient.c
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 11/30/12.
//  Copyright (c) 2012 Bruce D'Amora. All rights reserved.
//

#define MAIN 1
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "types.h"
#include "mathutil.h"
#include "profile.h"
#include "ivrclient.h"
#include "utils.h"
#include "sysutil.h"
#include "trackball.h"
#include <connection.h>
#include "tiffio.h"

#ifdef __linux
#include <GL/gl.h>
#include <GL/glut.h>
#else
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#endif


#define menuitem(x) 	\
(((menurightedge - x) <= menuwidth) && ((menurightedge - x) >= 0) ? (((x-menuleftedge))/menuitemwidth) : -1);

#define bool int 
#define false 0
#define true 1

char *hostname = "localhost";
char *portnum = "1400";
static bool    	mousemove = false;
static bool    	redraw = false;
int  highlight=0;
int  winwidth = WINDOWWIDTH;
int  winheight = WINDOWHEIGHT;
int  width=501,height=201,depth=276;
int  win, subwin;
int  subwinwidth = WINDOWWIDTH;
int	 subwinheight =  70;
int	 cutx=0, cuty=0, cutz=0;
int	 sockfd;
int startx, starty, lastx, lasty;
int camerazoom = 0;
int cameramove = 0;
int camerarotate = 1;
int boundingbox = 0;
int boundingvol = 0;
static GLint texName;
static float alpha = 1.0f;
float aspect;
float *tiffimage;
float scale;
float maxdim;
float deltaw, deltah, deltad;
float fwidth, fheight;
static float    lastquat[4] = {0.0, 0.0, 0.0, 0.0};
static float    curquat[4];


STATE 		mystate;
WINDOW 		viewport;
matrix3f    vwm;        // view to world matrix
matrix3f    wvm;        // world to view matrix
matrix4f    vpm;        // ndc to window coordinates matrix
matrix4f    vvm;        // eye to view volume (ndc) matrix
matrix4f    mvm;        // model-view matrix
matrix4f    pm;         // projection  matrix
matrix4f    rotation = {{1.0f, 0.0f, 0.0f, 0.0f},
                        {0.0f, 1.0f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 1.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f, 1.0f}};  // rotation  matrix

vector3f    eyepos;
vector3f    scalefactor;
#ifdef ORTHO
vector3f    delta = {0.0f, 0.0f, 0.0f};
#else
vector3f    delta = {0.0f, 0.0f, -5.0f};
#endif
vector3f    dir, up, e, vrp;
VOLUME      volume;
AABB        aabb;
CAMERA      camera;
FRUSTUM     viewvolume, frustum;
WINDOW      viewport;


/* Draw the menu */
GLfloat menubox[][3] = {{-7.1f, -1.0f, 0.0f}, {-7.1f, 1.0f, 0.0f}, {7.1f, 1.0f, 0.0f}, {7.1f, -1.0f, 0.0f}};
float  menuleftedge, menurightedge;
int menuitems = 8;
int menuitemselected = -1;
float menubuttonwidth;
float  menuitemwidth;
float menuwidth;
			
			
//cutx: 0,3,7,4......1,2,6,5
//cuty: 7,3,2,6......4,0,1,5
//cutz: 7,6,5,4......0,1,2,3
/* Draw the cube.*/
GLfloat vertices[][3] = {
    {-1.0,-1.0,-1.0},{1.0,-1.0,-1.0}, {1.0,1.0,-1.0}, {-1.0,1.0,-1.0},
    {-1.0,-1.0,1.0}, {1.0,-1.0,1.0}, {1.0,1.0,1.0}, {-1.0,1.0,1.0}};


void subinit()
{
 	menubuttonwidth = (fabs(menubox[0][0]) * 2.0f)/menuitems;

    fwidth = fheight = 2.0f;

    aspect = (float)subwinwidth/(float)subwinheight;

    glLoadIdentity();
    if (aspect > 1.0f) {
        fwidth *= aspect;
    }
    else {
        aspect = 1.0f/aspect;
        fheight *= aspect;
    }

	FILE *texin = fopen("../iVRMenu.rgb","r+b");
	int cnt = 2048*256*3;
	unsigned char *pixels = (unsigned char *)malloc(sizeof(unsigned char) * cnt);	
	int num=fread(pixels, sizeof(unsigned char), cnt * sizeof(unsigned char), texin);
	fprintf(stderr,"num read=%d\n", num);

	glGenTextures(1, &texName);
   	glBindTexture(GL_TEXTURE_2D, texName);

   	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	fprintf(stderr,"error=%s\n", gluErrorString(glGetError()));
   	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                   GL_NEAREST);
   	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                   GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	fprintf(stderr,"error = %s\n", gluErrorString(glGetError()));

}
void resettransforms()
{
	matrix4f m;
    scale = 1.0f;
    setlastxy(0, 0);
    delta.x = delta.y = 0.0f;
#ifdef ORTHO
    e.u = 0.0f; e.v = 0.0f; e.n = -10000.0f;    // ORTHO
    delta.z = 0.0f;
#else
    e.u = 0.0f; e.v = 0.0f; e.n = -5.0f;        // PERSPECTIVE
    delta.z = -5.0f;
#endif

    trackball(curquat, 0.0, 0.0, 0.0, 0.0);
    build_rotmatrix(m, curquat);
    setrotation(m);
	scale=1.0f;
	mystate.newconst[0] = mystate.newconst[1] = scalefactor.x;
	mystate.newconst[2] = mystate.newconst[3] = scalefactor.y;
	mystate.newconst[4] = mystate.newconst[5] = scalefactor.z;

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
	if (boundingbox) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glColor3f(0.0f, 1.0f, 0.0f);
		float x0, y0, x1, y1;
		x0 = aabb.wincoord.x; x1 = x0 + aabb.wincoord.w;
		y0 = aabb.wincoord.y; y1 = y0 + aabb.wincoord.h;
		glRectf(x0, y0, x1, y1);
	}
}
void showbvol()
{
	if (boundingvol) {
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

}
void showcutplane(int x, int y, int z) 
{
	//cutx: 0,3,7,4......1,2,6,5
	//cuty: 7,3,2,6......4,0,1,5
	//cutz: 7,6,5,4......0,1,2,3
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(4);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (x) {
		float dx1 = (scalefactor.x - mystate.newconst[0]);
		float dx2 = -(scalefactor.x - mystate.newconst[1]);
		
		fprintf(stderr,"dx1,dx2=%f,%f\n", dx1, dx2);
		if (highlight == 1)  {
			glTranslatef(dx1, 0.0f, 0.0f);
			glColor3f(1.0f, 0.0f, 0.0f);
    		polygon(0,3,7,4);
		}
		else {
			glTranslatef(dx1, 0.0f, 0.0f);
			glColor3f(0.5f, 0.0f, 0.0f);
    		polygon(0,3,7,4);
		}
			
		if (highlight == 2) {
			glTranslatef(-dx1, 0.0f, 0.0f);
			glTranslatef(dx2, 0.0f, 0.0f);
			glColor3f(0.0f, 1.0f, 0.0f);
    		polygon(1,2,6,5);
		}
		else {
			glTranslatef(-dx1, 0.0f, 0.0f);
			glTranslatef(dx2, 0.0f, 0.0f);
			glColor3f(0.0f, 0.5f, 0.0f);
    		polygon(1,2,6,5);
		}
	}
	else if (y) {
		float dy1 = (scalefactor.y - mystate.newconst[2]);
		float dy2 = -(scalefactor.y - mystate.newconst[3]);
		if (highlight == 1)  {
			glTranslatef(0.0f, dy1, 0.0f);
			glColor3f(1.0f, 0.0f, 0.0f);
    		polygon(4,0,1,5);
		}
		else {
			glTranslatef(0.0f, dy1, 0.0f);
			glColor3f(0.5f, 0.0f, 0.0f);
    		polygon(4,0,1,5);
		}

		if (highlight == 2)  {
			glTranslatef(0.0f, -dy1, 0.0f);
			glTranslatef(0.0f, dy2, 0.0f);
			glColor3f(0.0f, 1.0f, 0.0f);
    		polygon(7,3,2,6);
		}
		else {
			glTranslatef(0.0f, -dy1, 0.0f);
			glTranslatef(0.0f, dy2, 0.0f);
			glColor3f(0.0f, 0.5f, 0.0f);
    		polygon(7,3,2,6);
		}
	}
	else if (z) {
		float dz1 = (scalefactor.z - mystate.newconst[4]);
		float dz2 = -(scalefactor.z - mystate.newconst[5]);
		if (highlight == 1)  {
			glTranslatef(0.0f, 0.0f, dz1);
			glColor3f(1.0f, 0.0f, 0.0f);
    		polygon(0,1,2,3);
		}
		else {
			glTranslatef(0.0f, 0.0f, dz1);
			glColor3f(0.5f, 0.0f, 0.0f);
    		polygon(0,1,2,3);
		}

		if (highlight == 2)  {
			glTranslatef(0.0f, 0.0f, -dz1);
			glTranslatef(0.0f, 0.0f, dz2);
			glColor3f(0.0f, 1.0f, 0.0f);
    		polygon(7,6,5,4);
		}
		else {
			glTranslatef(0.0f, 0.0f, -dz1);
			glTranslatef(0.0f, 0.0f, dz2);
			glColor3f(0.0f, 0.5f, 0.0f);
    		polygon(7,6,5,4);
		}
	}
	glLineWidth(1);
}
  
    
void keyboard(unsigned char key, int x, int y)
{
	y = winheight - y;
	setlastxy(x,y);
    switch(key) {
        case 27:
            freeall();
			mystate.finish = 1;
			sendstate(sockfd, mystate);
			disconnect(sockfd);
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
		case 'a':
			mystate.alpha -= DELTAALPHA;
			mystate.alpha = (mystate.alpha < 0.0f) ? 0.0f : mystate.alpha;
			break;
		case 'A':
			mystate.alpha += DELTAALPHA;
			mystate.alpha = (mystate.alpha > 1.0f) ? 1.0f : mystate.alpha;
			break;
		case '1':
			mystate.newconst[0] += deltaw;;
			mystate.newconst[0] = (mystate.newconst[0] > 1.0f) ? 1.0f : mystate.newconst[0];
			break;
		case '!':
			mystate.newconst[0] -= deltaw;
			mystate.newconst[0] = (mystate.newconst[0] < deltaw) ? 0.1f : mystate.newconst[0];
			break;
		case '2':
			mystate.newconst[1] += deltaw;
			mystate.newconst[1] = (mystate.newconst[1] > 1.0f) ? 1.0f : mystate.newconst[1];
			break;
		case '@':
			mystate.newconst[1] -= deltaw;
			mystate.newconst[1] = (mystate.newconst[1] < deltaw) ? 0.1f : mystate.newconst[1];
			break;
		case '3':
			mystate.newconst[2] += deltah;
			mystate.newconst[2] = (mystate.newconst[2] > 1.0f) ? 1.0f : mystate.newconst[2];
			break;
		case '#':
			mystate.newconst[2] -= deltah;
			mystate.newconst[2] = (mystate.newconst[2] < deltah) ? 0.1f : mystate.newconst[2];
			break;
		case '4':
			mystate.newconst[3] += deltah;
			mystate.newconst[3] = (mystate.newconst[3] > 1.0f) ? 1.0f : mystate.newconst[3];
			break;
		case '$':
			mystate.newconst[3] -= deltah;
			mystate.newconst[3] = (mystate.newconst[3] < deltah) ? 0.1f : mystate.newconst[3];
			break;
		case '5':
			mystate.newconst[4] += deltad;
			mystate.newconst[4] = (mystate.newconst[4] > 1.0f) ? 1.0f : mystate.newconst[4];
			break;
		case '%':
			mystate.newconst[4] -= deltad;
			mystate.newconst[4] = (mystate.newconst[4] < deltad) ? 0.1f : mystate.newconst[4];
			break;
		case '6':
			mystate.newconst[5] += deltad;
			mystate.newconst[5] = (mystate.newconst[5] > 1.0f) ? 1.0f : mystate.newconst[5];
			break;
		case '^':
			mystate.newconst[5] -= deltad;
			mystate.newconst[5] = (mystate.newconst[5] < deltad) ? 0.1f : mystate.newconst[5];
			break;
        default:
            break;
    }
    
    glutPostRedisplay();
}

void reshapesubwin(int w, int h)
{
	fwidth = 2.0f;
	fheight = 2.0f;
	glViewport(0, 0, w, h);
	fprintf(stderr,"w,h=%d,%d\n", w, h);
    float aspect = (float) w/(float) h;

	fprintf(stderr,"reshapesubwin\n");
    if (aspect > 1.0f) {
        fwidth *= aspect;
    }
    else {
        aspect = 1.0f/aspect;
        fheight *= aspect;
    }
	fprintf(stderr,"fw,fh=%f,%f\n", fwidth, fheight);
	float menuedge = fabs(menubox[0][0]);
	menuleftedge = (float) w *  ((fwidth/2.0f-menuedge)/fwidth);
	menurightedge = (float) w *  ((fwidth/2.0f+menuedge)/fwidth);
	menuwidth = menurightedge - menuleftedge;
	menuitemwidth = menuwidth/menuitems;
	fprintf(stderr,"menuitemwidth=%f\n", menuitemwidth);
	fprintf(stderr,"menuleftedge, menurightedge=%f,%f\n", menuleftedge, menurightedge);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-fwidth/2.0f, fwidth/2.0f, -fheight/2.0f, fheight/2.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
void reshape(int w, int h)
{
	float fwidth = FRUSTUMWIDTH;
	float fheight = FRUSTUMHEIGHT;

	if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    mystate.winwidth = winwidth = w;
    mystate.winheight = winheight = h;

	aspect = (float) winwidth / (float) winheight;

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
	frustum.bottom =  -fheight/2.0f;
	frustum.top 	=  	-frustum.bottom;
	frustum.left = 	-fwidth/2.0f;
	frustum.right =   -frustum.left;
	frustum.near =    -2.0f;
	frustum.far =      2.0f;
   	glOrtho(frustum.left, frustum.right, frustum.bottom, frustum.top, frustum.near, frustum.far);
#else
	float invscale = 1.0f/scale;
    gluPerspective(invscale*43.6028, aspect, 0.1f, 100.0f);
	getfrustum(invscale*43.6028, aspect, mystate.vv);
#endif

	initcolbufs();

	glutSetWindow(subwin);
	glutReshapeWindow(w, subwinheight);
	glutPositionWindow(0, winheight-subwinheight);
	glutSetWindow(win);
}


void displaysubwin()
{
	fprintf(stderr,"displaysubwin\n");
	glClearColor(0.31f, 0.51, 0.74, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
   	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, texName);

	glColor3f(0.3f, 0.3f, 0.8f);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 1.0);
	glVertex3fv(menubox[0]);
	glTexCoord2f(0.0, 0.0);
	glVertex3fv(menubox[1]);
	glTexCoord2f(1.0, 0.0);
	glVertex3fv(menubox[2]);
	glTexCoord2f(1.0, 1.0);
	glVertex3fv(menubox[3]);
	glEnd();

	if (menuitemselected != -1) {
		fprintf(stderr,"here\n");
   		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, texName);
		float tr,tl;
		float vr, vl;
		tl = 1.0f/menuitems * menuitemselected;
		tr = tl + 1.0f/menuitems;
		vl = menubox[0][0] + menubuttonwidth * menuitemselected;
		vr = vl + menubuttonwidth;
		glBegin(GL_POLYGON);
		glTexCoord2f(tl, 1.0);
		glVertex3f(vl, -1.0f, 0.0f);
		glTexCoord2f(tl, 0.0);
		glVertex3f(vl, 1.0f, 0.0f);
		glTexCoord2f(tr, 0.0);
		glVertex3f(vr, 1.0f, 0.0f);
		glTexCoord2f(tr, 1.0);
		glVertex3f(vr, -1.0f, 0.0f);
		glEnd();


	}
    glutSwapBuffers();

}
void display()
{
	int rc;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// modelview transformation
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// move 
	gluLookAt(-delta.x, -delta.y, -e.z, -delta.x, -delta.y, 0, 0, 1, 0);
    // rotate
    glMultMatrixf((float *) rotation);

	// store the current modelview matrix and store in state
	glGetFloatv(GL_MODELVIEW_MATRIX,(float *)mystate.mvm);

	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#ifdef ORTHO
	glOrtho(viewvolume.left, viewvolume.right, viewvolume.bottom, viewvolume.top, viewvolume.near, viewvolume.far);
#else
	float invscale = 1.0f/scale;
	gluPerspective(invscale*43.6028, aspect, 0.1f, 100.0f);
	getfrustum(invscale*43.6028, aspect, mystate.vv);
#endif
	// store current projection matrix in state
	glGetFloatv(GL_PROJECTION_MATRIX,(float *)mystate.pm);

	// send the matrix state to server application
	rc = sendstate(sockfd,  mystate);
	fprintf(stderr,"bytes sent = %d\n", rc);

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
	// get the image from server app
	//int size = winwidth*winheight*ELEMENTSPERPIXEL*sizeof(float);
	int size = winwidth*winheight*ELEMENTSPERPIXEL;
	rc = recvimage(sockfd, tiffimage, size);
    fprintf(stderr,"bytes recvd = %d\n", rc);
	glRasterPos3f(0.0f, 0.0f, 0.0f);
    glDrawPixels(winwidth, winheight, GL_RGBA, GL_UNSIGNED_BYTE, tiffimage);
	showbbox();
	// return projection to whatever it was
    glPopMatrix();
	// set modelview xform mode and return modelview xfrom to whatever it was
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	// draw a bounding volume if appropriate
    showbvol();
	showcutplane(cutx, cuty, cutz);
    glutSwapBuffers();
    TIME(RENDER, stop);
    PERFORMANCE(start, stop, framenum, elapsed, avg);
	PRINTDEBUG("start,stop=%ld, %ld\n", start, stop);
     
}

void mousebuttonsubwin(int button, int state, int x, int y) {

    if(button==GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN){
			int savemenuitemselected = menuitemselected;
			menuitemselected = menuitem(x);
			fprintf(stderr,"mousebuttonsubwin, button=%d, state=%d, x=%d, y=%d, item=%d\n", button, state, x, y, menuitemselected);

			switch(menuitemselected) {
				case HOME:
					resettransforms();
					cutx = cuty = cutz = 0;
					break;

				case CUTX:
					cutx = 1;
					cuty = 0;
					cutz = 0;
					highlight = (savemenuitemselected != menuitemselected) ? 0 : highlight;
					break;
					
				case CUTY:
					cutx = 0;
					cuty = 1;
					cutz = 0;
					highlight = (savemenuitemselected != menuitemselected) ? 0 : highlight;
					break;
					
				case CUTZ:
					cutx = 0;
					cuty = 0;
					cutz = 1;
					highlight = (savemenuitemselected != menuitemselected) ? 0 : highlight;
					break;
					
				default:
					break;
			}
			glutPostRedisplay();
			glutSetWindow(win);
			glutPostRedisplay();
		}
			
    }

		
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
				if (cutx | cuty | cutz) {
					unsigned int color[4];
					glReadBuffer(GL_FRONT);
					// grab a 4 pixel square to make it a bit easier to select
					glReadPixels(x,y,2,2,GL_RGBA,GL_UNSIGNED_BYTE, color);
					for (int i=0; i<4; i++) {
						highlight = (0x000000ff & color[i]) ? 1 : (0x0000ff00 & color[i]) ? 2 : highlight;
						if ((highlight == 1) || (highlight == 2)) break;
					}

					glutPostRedisplay();
				} 
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
	static float d=0.0f;
    
	y = winheight - y;
    if (!mousemove) return;
	switch(menuitemselected) 
	{
    	case ROTATE: 
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
		break;

		case PAN:
		{
		float dx = (float) (x - lastx)/winwidth;
		float dy = (float) (y - lasty)/winheight;
        delta.x += dx;
        delta.y += dy;
     	// delta.z = 0.0f; // delta.z is initialized once as there is currently no interface to move  along the Z axis
		}
		break;

		case ZOOM:
		{
        float dy = (float) (y - lasty);

        scale  +=  dy/(float)winheight;
        scale = (scale < ZOOMMIN) ? ZOOMMIN : scale;
        scale = (scale > ZOOMMAX) ? ZOOMMAX : scale;
		}
		break;

		case ALPHA:
		{
		int dx = x - lastx;
		float sign = copysign(1.0, dx);
        //mystate.alpha += DELTAALPHA * sign;
        alpha += DELTAALPHA * sign;
		float a = exp2f(alpha) - 1.0f;
		fprintf(stderr,"alpha=%f, exp2f(alpha)=%f\n", alpha, a);
		mystate.alpha = clampf(a, 0.0f, 1.0f);

		}
        break;
		
		case CUTX:
		{
			if (!highlight) break;
			float dx = (float) (x - lastx);
			float sign = copysign(1.0f, dx);
			if (highlight == 1) {
				mystate.newconst[0] += (deltaw * -sign);
            	mystate.newconst[0] = clampf(mystate.newconst[0], deltaw, scalefactor.x);
			}
			else {
				mystate.newconst[1] += (deltaw * sign);
            	mystate.newconst[1] =  clampf(mystate.newconst[1], deltaw, scalefactor.x);
			}
		}
		break;
		case CUTY:
		{
			float dx = (float) (x - lastx);
			float sign = copysign(1.0f, dx);
			if (!highlight) break;
			if (highlight == 1) {
				mystate.newconst[2] += (deltah  * -sign);
            	mystate.newconst[2] = clampf(mystate.newconst[2], deltah, scalefactor.y);
			}
			else {
				mystate.newconst[3] += (deltah * sign);
            	mystate.newconst[3] = clampf(mystate.newconst[3], deltah, scalefactor.y);
			}
		}
		break;
		case CUTZ:
		{
			float dx = (float) (x - lastx);
			float sign = copysign(1.0f, dx);
			if (!highlight) break;
			if (highlight == 1) {
				mystate.newconst[4] += (deltad * -sign);
            	mystate.newconst[4] = clampf(mystate.newconst[4], deltad, scalefactor.z);
			}
			else {
				mystate.newconst[5] += (deltad * sign);
            	mystate.newconst[5] = clampf(mystate.newconst[5], deltad, scalefactor.z);
			}
		}
		break;
			
		default:
		break;
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

//-----------------------------------------------------------------------------------------------------------------------//
// initcolbufs
//-----------------------------------------------------------------------------------------------------------------------//

void initcolbufs()
{
    // winsize * BYTESPERPIXEL  because the TIFF file from server is RGBA 
    int winsize = winwidth*winheight;
    if (tiffimage != NULL) free(tiffimage);
    tiffimage = (float *) malloc(winsize * ELEMENTSPERPIXEL);
    assert(tiffimage != NULL);
    memset((unsigned char *) tiffimage, 0, winsize*ELEMENTSPERPIXEL);
}

void freeall()
{
    if (tiffimage != NULL) free (tiffimage);
}

void main(int argc, char *argv[])
{
	if (argc >= 1) {
		hostname = NULL;
		hostname = (char *) malloc(sizeof(argv[1]));
		strcpy(hostname,argv[1]);

		if (argc == 5) {
			width = atoi(argv[2]);
			height = atoi(argv[3]);
			depth = atoi(argv[4]);
			fprintf(stderr,"%d, %d, %d\n", width, height, depth);
		}

	}
    // connect to relay server
    if (( sockfd = initconnection(hostname,portnum)) <= 0) {
		fprintf(stderr,"connection failed, socket=%d\n", sockfd);
		exit(0);
	};

	// INITIALIZATION
	// initialize modelview, projection transformations
    resettransforms();
	// initialize  performance timers
	RESET_TIME();
	// initialize alpha scale
	mystate.alpha = 0.25;

	// adjust the aspect ratio of bounding cube based on volume dimensions
    maxdim =  (float) (imax3(width, height, depth));
    // compute scale factor for global bounding box. Will use to draw a global bounding box
    float invdim  = 1.0f/maxdim;
    scalefactor.x = (float)width * invdim;
    scalefactor.y = (float)height * invdim;
    scalefactor.z = (float)depth * invdim;

	// initialize clipping plane constants 
	mystate.newconst[0] = mystate.newconst[1] = scalefactor.x;
	mystate.newconst[2] = mystate.newconst[3] = scalefactor.y;
	mystate.newconst[4] = mystate.newconst[5] = scalefactor.z;
	deltaw =  (scalefactor.x * 0.05);
	deltah =  (scalefactor.y * 0.05);
	deltad =  (scalefactor.z * 0.05);

	// initialize vertices for bounding volume box
    for (int i=0; i<8; i++) {
        vertices[i][0] *= scalefactor.x;
        vertices[i][1] *= scalefactor.y;
        vertices[i][2] *= scalefactor.z;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WINDOWWIDTH, WINDOWHEIGHT);
    win = glutCreateWindow("iVR");
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mousebutton);
    glutMotionFunc(mousemotion);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_DEPTH_TEST);
	int suborgx =  (winwidth<<1)  -  (subwinwidth<<1);
	int suborgy = winheight - subwinheight;
	subwin = glutCreateSubWindow(win, suborgx, suborgy, subwinwidth, subwinheight);
	subinit();
	glutMouseFunc(mousebuttonsubwin);
    glutReshapeFunc(reshapesubwin);
	glutDisplayFunc(displaysubwin);
    glutMainLoop();
}
