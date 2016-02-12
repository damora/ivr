//
//  glui.h
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 11/30/12.
//  Copyright (c) 2012 Bruce D'Amora. All rights reserved.
//

#ifndef IBM_Ray_Caster_glui_h
#define IBM_Ray_Caster_glui_h

//----------------------------------------------------------------------------------------------------------------------------//
// OpenGL display, interactivity                                                                                              //
//----------------------------------------------------------------------------------------------------------------------------//
#ifdef LOCAL
void display(void);
void reshape(int, int);
void mousemotion(int, int);
void stopmotion(int, int);
void startmotion(int, int);
void mousebutton(int, int, int, int);
void keyboard(unsigned char, int, int);
void resettransforms();
#endif
#define  setlastxy(x, y) {lastx = x; lasty = y;}
#endif
