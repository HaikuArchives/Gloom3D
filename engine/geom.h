//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   geom.h - basic geometry functions                                  //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _GEOM_H
#define _GEOM_H

#include "types.h"

// sets width and height of the screen
void setscreensize(int width,int height);
// sets current view point and view angles
void setviewpoint(coord3d x,coord3d y,coord3d z,angle ax,angle ay,angle az);
// projects (x,y,z) to screen coordinates
void project(coord3d x,coord3d y,coord3d z,coord2d *x2,coord2d *y2);
// applies rotation along z axis
void rotatez(coord3d x,coord3d y,coord3d *xr,coord3d *yr);
// projects only x coordinate
coord2d projectx(coord3d x,coord3d y);
// projects only z coordinate
coord2d projectz(coord3d z,coord3d y);
// applies rotation along x and y axes
void rotatexyp(coord3d x,coord3d y,coord3d z,coord2d *xr,coord2d *yr);
// applies rotation along y and x axes
void rotateyxp(coord3d x,coord3d y,coord3d z,coord2d *xr,coord2d *yr);
void rotateyxv(Tvector *v);

#ifdef _GEOM_CPP
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

GLOBAL Tvector view;   // current view point
GLOBAL angle viewa[3]; // current view angles
GLOBAL double sinx,cosx,siny,cosy,sinz,cosz;  // sines and cosines of the view angles
GLOBAL int scr_dx,scr_dy; // screen size
GLOBAL int scr_ox,scr_oy; // screen center
GLOBAL int scr_foc;       // focal length (distance between the view point and the screen)


#undef GLOBAL

#endif
