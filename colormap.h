#ifndef COLORMAP_H

#include "types.h"
#include "mathutil.h"
#include "utils.h"
#include "profile.h"

#define COLORMAP_H
#define BASE 0
#define RAINBOW 1
#define MEDICAL 2
#define SEISMIC 3
#define JET  4
#define NOMAP -1
void basecolormap(color4f *, int *, int, float, float);
void seismiccolormap(color4f *, int *, int, float, float);
void medcolormap (color4f *, int *, int, float, float);
void rainbowcolormap (color4f *, int *, int, float, float);
void jetcolormap(color4f *, int *, int, float, float);
void nomap (color4f *, int *, int, float, float);
void initcolormap(int);
void (*colormap)(color4f *, int *, int , float, float);
#endif

