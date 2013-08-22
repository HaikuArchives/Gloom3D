//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   draw.h - low level rendering interface                             //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _DRAW_H
#define _DRAW_H

#include "ttexture.h"
#include "ttrap.h"


// sets the data for the perspective correct texture mapping
// a is from point from a polygon with (u,v)=(0,0)
// p is a vector in the polygon plane along u
// q is a vector in the polygon plane along v
void setdrawdata(Tvector *a,Tvector *p,Tvector *q);
// sets the current texture
void setdrawtexture(int txt);
// draws a Tmonotone
void drawmp(Tmonotone *mp);

// initializes the software renderer
void draw_init(byte *buffer,int w,int h);
// frees the allocated memory
void draw_done( void );

#endif
