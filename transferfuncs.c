//
//  transferfuncs.c
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 1/8/13.
//  Copyright (c) 2013 Bruce D'Amora. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "types.h"
#include "connection.h"
#include "transferfuncs.h"

extern int lutoffset;
extern int maxlutindx;



#ifdef BGQ
#define __mymax(a,b)  (__fsel(a-b, a, b))
#endif
color4f medtransferfunc(color4f *lut, color4f color, float *samples, int nsamples, float myalpha)
{
  int i = 0, index;  

#ifdef BGQ_QPX
  vector4double colorq;
  colorq = vec_ld(0, (float *)(&color));
  vector4double med_const = vec_splats(0.8);
#endif

  for (i = 0; i < nsamples; ++i) {
    index = (int) samples[i];
	index -= lutoffset;

#ifndef BGQ
    color.b = (lut[index].b < 0.8) ? fmax(lut[index].b, color.b) : color.b;
    color.g = (lut[index].g < 0.8) ? fmax(lut[index].g, color.g) : color.g;
    color.r = (lut[index].r < 0.8) ? fmax(lut[index].r, color.r) : color.r;
    color.a = (lut[index].a) * myalpha;
#elif BGQ_QPX
    vector4double lutq = vec_ld(sizeof(color4f) * index, (float *)lut);
    vector4double tmp0 = vec_sub(med_const, lutq);
    //compute max
    vector4double tmp1 = vec_sub(lutq, colorq);
    vector4double tmp2 = vec_sel(colorq, lutq, tmp1);    
    colorq = vec_sel(colorq, tmp2, tmp0);
#else
    color.b = __fsels((0.8 - lut[index].b), 
		      __mymax(lut[index].b, color.b), color.b);
    color.g = __fsels((0.8 - lut[index].g), 
		      __mymax(lut[index].g, color.g), color.g);
    color.r = __fsels((0.8 - lut[index].r), 
		      __mymax(lut[index].r, color.r), color.r);
    color.a = lut[index].a * myalpha;    
#endif    
  }

#ifdef BGQ_QPX
  vec_insert(1.0, colorq, 3);
  vec_st(colorq, 0, (float *)&color);
  color.a *= myalpha;
#endif

  return color;
}

color4f blendtransferfunc(color4f *lut, color4f color, float *samples, int nsamples, float myalpha)
{
    int i, index = 0;
    for (i = 0; i < nsamples; ++i) {
      index = (int) samples[i];
      float alpha = 1.0f;
      alpha = lut[index].a;
      color.b = alpha * lut[index].b + (1.0f - alpha)  * color.b;
      color.g = alpha * lut[index].g + (1.0f - alpha)  * color.g;
      color.r = alpha * lut[index].r + (1.0f - alpha)  * color.r;
      color.a = alpha * myalpha;
    }
    return color;
}


color4f seismictransferfunc(color4f *lut, color4f color, float *samples, int nsamples, float myalpha)
{
    int i = 0, index = 0;
    for (i = 0; i < nsamples; ++i) {
      index = (int) samples[i];
      index -= lutoffset;
      color.b = ((lut[index].b > 0.65) && (lut[index].b < 1.0f)) ? 0.6f : color.b;
      color.g = ((lut[index].g > 0.65) && (lut[index].g < 1.0f)) ? 0.0f : color.g;
      color.r = ((lut[index].r > 0.65) && (lut[index].r < 1.0f)) ? 0.0f : color.r;
      
      color.b = ((lut[index].b > 0.5) && (lut[index].b < 0.65f)) ? 0.9f : color.b;
      color.g = ((lut[index].g > 0.5) && (lut[index].g < 0.65f)) ? 0.9f : color.g;
      color.r = ((lut[index].r > 0.5) && (lut[index].r < 0.65f)) ? 0.9f : color.r;
      
      color.b = ((lut[index].b > 0.3) && (lut[index].b < 0.5f)) ? 0.0f : color.b;
      color.g = ((lut[index].g > 0.3) && (lut[index].g < 0.5f)) ? 0.0f : color.g;
      color.r = ((lut[index].r > 0.3) && (lut[index].r < 0.5f)) ? 0.6f : color.r;
      color.a = 1.0f * myalpha;
    }

    return color;    
}

color4f seismictransferfunc2(color4f *lut, color4f color, float *samples, int nsamples, float myalpha)
{
    int i = 0, index = 0;
    float oneminusalpha;
    float red, green, blue, alpha;
    color4f *srccolor;
    
    for (i = nsamples-1; i >= 0; i--) {
        index = (int) samples[i];
    	index -= lutoffset;
		if (index < 0) index = 0;
		if (index > maxlutindx) index = maxlutindx;
        srccolor = &lut[index];
        
        red = srccolor->r;
        green = srccolor->g;
        blue = srccolor->b;
        alpha = srccolor->a * myalpha;
        oneminusalpha = 1.0f - alpha;
        color.b = oneminusalpha  * color.b +  alpha * blue;
		color.g = oneminusalpha  * color.g +  alpha * green;
		color.r = oneminusalpha  * color.r +  alpha * red;
		color.a = alpha + oneminusalpha * color.a;        
		if (color.a > 1.0f) color.a = 1.0f;
	}

    return color;    
}
color4f chemistrytransferfunc(color4f *lut, color4f color, float *samples, int nsamples, float myalpha)
{
    int i = 0, index = 0;
    float oneminusalpha;
    float red, green, blue, alpha;
    color4f *srccolor;
    
    for (i = nsamples-1; i >= 0; i--) {
        index = (int) samples[i];
        index -= lutoffset;
        if (index < 0) index = 0;
        srccolor = &lut[index];
/*
		color.r = srccolor->r;
		color.g = srccolor->g;
		color.b = srccolor->b;
		color.a = srccolor->a;
*/
        red = srccolor->r;
        green = srccolor->g;
        blue = srccolor->b;
        alpha = srccolor->a * myalpha;
        oneminusalpha = 1.0f - alpha;
        color.b = oneminusalpha  * color.b +  alpha * blue;
        color.g = oneminusalpha  * color.g +  alpha * green;
        color.r = oneminusalpha  * color.r +  alpha * red;
        color.a = alpha + oneminusalpha * color.a;
		if (color.a > 1.0f) color.a = 1.0f;
	}

    return color;    
}

color4f rgbtransferfunc(color4f *lut, color4f color, float *samples, int nsamples, float myalpha)
{
    int i = 0, index = 0;
    float oneminusalpha;
    float red, green, blue, alpha;
    color4f srccolor;

    for (i = nsamples-1; i >= 0; i--) {
		unsigned int *ui = (unsigned int *) (&samples[i]);
        srccolor.r  =  1.0f/255.0f * (float) (( *ui  >> 24) & 0xff) ;
        srccolor.g  =  1.0f/255.0f * (float) (( *ui  >> 16) & 0xff) ;
        srccolor.b  =  1.0f/255.0f * (float) (( *ui  >> 8) & 0xff) ;
        //srccolor.a  =  (float) ((srccolor.r > srccolor.g) ? srccolor.r : srccolor.g);
        srccolor.a  =   myalpha;

        red = srccolor.r;
        green = srccolor.g;
        blue = srccolor.b;
        //alpha = srccolor.a * myalpha;
        alpha = srccolor.a;
		
        oneminusalpha = 1.0f  - alpha;
        color.b = oneminusalpha  * color.b +  alpha * blue;
		color.g = oneminusalpha  * color.g +  alpha * green;
		color.r = oneminusalpha  * color.r +  alpha * red;
		color.a = alpha  + oneminusalpha * color.a ;       
		if (color.a > 1.0f) color.a = 1.0f;
	}

    return color;    
}
void inittransferfunc(int xfer)
{
    switch(xfer) {
        case SEISMIC:
            transferfunc = &seismictransferfunc;
            break;
        case MEDXFER:
            transferfunc = &medtransferfunc;
            break;
        case SEISMIC2:
            transferfunc = &seismictransferfunc2;
            break;
        case CHEM:
            transferfunc = &chemistrytransferfunc;
            break;
        case BLEND:
            transferfunc = &blendtransferfunc;
            break;
        case RGBBLEND:
            transferfunc = &rgbtransferfunc;
            break;
        default:
            transferfunc = &seismictransferfunc;
            break;
    }   
}
