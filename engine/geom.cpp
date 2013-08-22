//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   geom.cpp - basic geometry functions implementation                 //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _GEOM_CPP
#include "geom.h"

#include <math.h>

// sets width and height of the screen
void setscreensize(int w,int h)
{
  scr_dx=w;
  scr_dy=h;
  scr_ox=w/2;
  scr_oy=h/2;
  if(scr_ox>scr_oy) scr_foc=scr_ox;
  else scr_foc=scr_oy;
}

// sets current view point and view angles
void setviewpoint( coord3d x,coord3d y,coord3d z,angle ax,angle ay,angle az )
{
  view.x=x;
  view.y=y;
  view.z=z;
  viewa[0]=ax;
  viewa[1]=ay;
  viewa[2]=az;
  sinx=sin(ax);cosx=cos(ax);
  siny=sin(ay);cosy=cos(ay);
  sinz=sin(az);cosz=cos(az);
}

// applies rotation along z axis
void rotatez(coord3d x,coord3d y,coord3d *xr,coord3d *yr)
{
  *xr=(x-view.x)*cosz+(y-view.y)*sinz;
  *yr=-(x-view.x)*sinz+(y-view.y)*cosz;
}

// applies rotation along x and y axes
void rotatexyp(coord3d x,coord3d y,coord3d z,coord2d *xr,coord2d *yr)
{
  coord3d x1=x*cosy+z*siny;
  coord3d z1=-x*siny+z*cosy;
  coord3d y2=y*cosx+z1*sinx;
  coord3d z2=-y*sinx+z1*cosx;
  *xr=x1*scr_foc/y2;
  *yr=z2*scr_foc/y2;
}

// applies rotation along y and x axes
void rotateyxp(coord3d x,coord3d y,coord3d z,coord2d *xr,coord2d *yr)
{
  coord3d y1=y*cosx-z*sinx;
  coord3d z1=y*sinx+z*cosx;
  coord3d x2=x*cosy-z1*siny;
  coord3d z2=x*siny+z1*cosy;
  *xr=x2*scr_foc/y1;
  *yr=z2*scr_foc/y1;
}

// applies rotation along y and x axes
void rotateyxv(Tvector *v)
{
  coord3d y1=v->y*cosx+v->z*sinx;
  coord3d z1=-v->y*sinx+v->z*cosx;
  coord3d x2=v->x*cosy+z1*siny;
  coord3d z2=-v->x*siny+z1*cosy;
  v->x=x2;v->y=y1;v->z=z2;
}

// projects (x,y,z) to screen coordinates
void project(coord3d x,coord3d y,coord3d z,coord2d *x2,coord2d *y2)
{
  *x2=x*scr_foc/z;
  *y2=(y-view.z)*scr_foc/z;
}

// projects only x coordinate
coord2d projectx(coord3d x,coord3d y)
{
  return x*scr_foc/y;
}

// projects only z coordinate
coord2d projectz(coord3d z,coord3d y)
{
  return (z-view.z)*scr_foc/y;
}
