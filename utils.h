#include <stdio.h>
#include <stdarg.h>
#include "types.h"

void sortboxlist(int, int, AABB *);
void createtiles(RECT *, WINDOW);
void stridememcpy(float *, float *, int, int, int, int);
void getlocalbrick(vector3i *, int, VOLUME *, vector3f, int, BRICK *);
int intersectrect(RECT, RECT);
int slicingcalc(int , int , int , int , int , int , int *, int *);
void PRINTDEBUG(char *, ...);
void PRINTDEBUG2(char *, ...);
void DEBUGSUBVOLUMES(color4f *, int);
void PRINTDEBUGIMAGE(char *, char *, int);
void PRINTDEBUGTILES(int , int , int, float *);
