//
//  mathutil.h
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 11/29/12.
//  Copyright (c) 2012 Bruce D'Amora. All rights reserved.
//


#include <math.h>
#include "types.h"


inline static int imin(int x, int y) { return((x < y) ? x : y);}
inline static int imax(int x, int y) { return ((x > y) ? x : y);}
inline static int imax3(int w, int h, int d)
{
    int mmax = -1;
    mmax = imax(w,h);
    mmax = imax(mmax,d);
    return mmax;
}


