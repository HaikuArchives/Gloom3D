//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tclip.h - class Tclip interface                                    //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _CLIP_H
#define _CLIP_H

#include "types.h"
#include "ttrap.h"

// Tclip is a Tmonotone that can clip trapezoids with itself  
class Tclip: public Tmonotone
{
public:
  Ttrap *beg,*end,**lbeg,*last;

  void cut(coord2d x1,coord2d x2);
  void restore(Ttrap *t);
  void clip(coord2d x1,coord2d x2,coord2d y11,coord2d y12,coord2d y21,coord2d y22,Tmonotone *poly);
  void addbeg(Tmonotone *poly);
  void addend(Tmonotone *poly);
};


#ifdef _CLIP_CPP
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

GLOBAL Tclip *cur_clip; //everything is clipped by the current clip
GLOBAL coord2d gymin,gymax;

#undef GLOBAL

//Set the current clip. All polygons that are drawn on the screen are first
//clipped with the current clip.
void set_cur_clip( Tclip *clip );

// splits [a1,a2] with ratio b1:b2
#define splitab(a1,a2,b1,b2) (((a1)*(b2)+(a2)*(b1))/((b1)+(b2)))

// deletes a linked list of Ttraps
void deltraps(Ttrap *t);

#endif
