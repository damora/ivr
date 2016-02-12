//
//  colormap.c
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 10/20/12.
//  Copyright (c) 2012 Bruce D'Amora. All rights reserved->
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#include "colormap.h"


//-----------------------------------------------------------------------------------------------------------------------//
// functions    
//-----------------------------------------------------------------------------------------------------------------------//
void nomap(color4f *collut, int *hist, int hsize, float histmin, float histmax)
{
}
void basecolormap(color4f *collut, int *hist, int hsize, float histmin, float histmax)
{
    float amax = -MAXFLOAT;
    float c;


    for (int i=0; i<hsize; i++) {
        amax = fmax(amax, (float)hist[i]);
    }
    
    for (int i=0; i<hsize; i++) {
        c = (float) (i+histmin)/histmax;
     
		collut[i].b = collut[i].g = collut[i].r = c;
		if (c > 0.8) {
			collut[i].r = 0.7f;
			collut[i].g = 0.75f;
			collut[i].b = 0.46;
			collut[i].a = 0.07;
		}
		if (c < 0.3) {
			collut[i].b = 0.6;
			collut[i].a = 0.01;
		}
		if (c > 0.3 && c <0.6) {
			collut[i].r = 0.6;
			collut[i].a= 0.01;
		}
		if (c > 0.6 && c <0.8) {
			collut[i].r = 0.6;
			collut[i].g = 0.6;
			collut[i].a = 0.005;
		}
    }
    
    return;
}

void rainbowcolormap(color4f *collut, int *hist, int hsize, float histmin, float histmax)
{
	// need to figure out if we ever need histmax, histmin here and if not remove
    collut[0].b = 1.0f;
	collut[0].g =  0.0f;
	collut[0].r = 0.0f;
	float dr = 1.0f/hsize;

	float db = -dr;
    for (int i=0; i<hsize; i++) {
		collut[i].r += dr;
		collut[i].b += db;
	//	collut[i].a = sqrtf(mag3f(collut[i].r, collut[i].g, collut[i].b));
		collut[i].a = 1.0f;
	}
}

void medcolormap(color4f *collut, int *hist, int hsize, float histmin, float histmax)
{
    float    a, x, y1, y2;
    //  float    parm[4] = {0.03, 0.1, 0.3, 0.8};
    float    parm[4] = {0.04, 0.9, 0.9, 0.8};

/*
    for (int i=0; i<256; i++) {
        x = i / 255.0 * RANGE;
*/
    for (int i=0; i<hsize; i++) {
        x = i / (float)(hsize-1) * RANGE;

        if ( (0<=x) && (x<=484) ) {                 /* Air */
            collut[i].r = 0.1 /*0.6*/;
            collut[i].g = 0.1 /*0.6*/;
            collut[i].b = 0.1 /*0.8*/;
            collut[i].a = 0.1 * parm[0];
        } else if ( (484<x) && (x<=545) ) {         /* Air and Fat */
            a  = (x - 484.0)/(545.0 - 484.0);
            y1 = (1.0 - a) * parm[0];
            y2 =        a  * parm[1];
            collut[i].r = (0.6*y1  + 0.8    *y2);
            collut[i].g = (0.6*y1  + 0.7    *y2);
            collut[i].b = (0.8*y1  + 0.0    *y2);
            collut[i].a = (0.1*y1  + 0.03125*y2);
        } else if ( (545<x) && (x<=726) ) {         /* Fat */
            collut[i].r = 0.8;
            collut[i].g = 0.7;
            collut[i].b = 0.0;
            collut[i].a = 0.03125 * parm[1];
        } else if ( (726<x) && (x<=819) )   {       /* Fat and Tissue */
            a  = (x - 726.0)/(819.0 - 726.0);
            y1 = (1.0 - a) * parm[1];
            y2 =        a  * parm[2];
            collut[i].r = (0.8    *y1 +  0.2   *y2);
            collut[i].g = (0.7    *y1 +  0.0   *y2);
            collut[i].b = (0.0    *y1 +  0.0   *y2);
            collut[i].a = (0.03125*y1 +  0.1875*y2);
        } else if ( (819<x) && (x<=883) ) {         /* Tissue */
            collut[i].r = 0.2;
            collut[i].g = 0.0;
            collut[i].b = 0.0;
            collut[i].a = 0.1875 * parm[2];
        } else if ( (883<x) && (x<=1535) ) {        /* Tissue and Bone */
            a  = (x - 883.0)/(1535.0 - 883.0);
            y1 = (1.0 - a) * parm[2];
            y2 =        a  * parm[3];
            collut[i].r = (0.2   *y1 + 1.0 *y2);
            collut[i].g = (0.0   *y1 + 1.0 *y2);
            collut[i].b = (0.0   *y1 + 0.9 *y2);
            collut[i].a = (0.1875*y1 + 0.75*y2);
        } else if ( 1535 < x ) {                    /* Bone */
            collut[i].r = 1.0;
            collut[i].g = 1.0;
            collut[i].b = 0.9;
            collut[i].a = 0.75 * parm[3];
        }
    }
}
void seismiccolormap(color4f *collut, int *hist, int hsize, float histmin, float histmax)
{
	float er[3] = {1.0f, 1.0f, 0.0f};
	float eg[3] = {0.0f, 1.0f, 0.0f};
	float eb[3] = {0.0f, 1.0f, 1.0f};
	float sr[3] = {1.0f, 0.5f*hsize, (float)hsize};
	float sg[3] = {1.0f, 0.5f*hsize, (float)hsize};
	float sb[3] = {1.0f, 0.5f*hsize, (float)hsize};

	float *red = (float *) malloc(sizeof(float) * hsize);
	float *green = (float *) malloc(sizeof(float) * hsize);
	float *blue = (float *) malloc(sizeof(float) * hsize);

	fprintf(stderr, "seisimc2, hsize=%d\n", hsize);
	lerp(red, sr, er, hsize);
	lerp(green, sg, eg, hsize);
	lerp(blue, sb, eb, hsize);

	for (int i=0; i<hsize; i++) {
		collut[i].r = red[i];
		collut[i].g = green[i];
		collut[i].b = blue[i];
		collut[i].a = 1.0f;
		fprintf(stderr,"collut=%f, %f, %f\n", red[i], green[i], blue[i]);
	}

	free(red);
	free(green);
	free(blue);

}

void jetcolormap(color4f *collut, int *hist, int hsize, float histmin, float histmax)
{
	float i4;
	for (int i=0; i<hsize; i++) { 
		i4=(4.0f*i)/hsize; 
		collut[i].r = fmin(fmax(fmin(i4-1.5,-i4+4.5),0.0f),1.0f); 
		collut[i].g = fmin(fmax(fmin(i4-0.5,-i4+3.5),0.0f),1.0f); 
		collut[i].b = fmin(fmax(fmin(i4+0.5,-i4+2.5),0.0f),1.0f); 
		collut[i].a = 1.0f;
	}
} 
 
void initcolormap(int type)
{
   switch(type) {
        case BASE:
            colormap = &basecolormap;
            break;
        case RAINBOW:
            colormap = &rainbowcolormap;
            break;
        case MEDICAL:
            colormap = &medcolormap;
            break;
        case SEISMIC:
            colormap = &seismiccolormap;
            break;
        case JET:
            colormap = &jetcolormap;
            break;
        default:
            colormap = &nomap;
            break;
    }   
}

