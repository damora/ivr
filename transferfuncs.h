// //  transferfuncs.h //  IBM Ray Caster // //  Created by Bruce D'Amora on 1/18/13.
//  Copyright (c) 2013 Bruce D'Amora. All rights reserved.
//

#ifndef IBM_Ray_Caster_transferfuncs_h
#define IBM_Ray_Caster_transferfuncs_h

#define SEISMIC 0
#define MEDXFER 1
#define  SEISMIC2 2
#define  CHEM 3
#define  BLEND 4
#define  RGBBLEND 5

void inittransferfunc(int);
color4f medtransferfunc(color4f *, color4f, float *, int, float);
color4f blendtransferfunc(color4f *, color4f, float *, int, float);
color4f seismictransferfunc(color4f *, color4f, float *, int, float);
color4f seismictransferfunc2(color4f *, color4f, float *, int, float);
color4f chemistrytransferfunc(color4f *, color4f, float *, int, float);
color4f rgbtransferfunc(color4f *, color4f,  float *, int, float);
color4f (*transferfunc)(color4f *, color4f, float *, int, float);

#endif
