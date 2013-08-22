//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Ttrap.cpp - Ttrap and Tmonotone implementation                     //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _TTRAP_CPP
#include "ttrap.h"

// different kind of constructors to initialize the object
Ttrap::Ttrap(coord2d _x2,coord2d _y11,coord2d _y12,coord2d _y21,coord2d _y22)
{
  x2=_x2;
  un=csegment++;
  dn=csegment++;
  y11=_y11;
  y12=_y12;
  y21=_y21;
  y22=_y22;
#ifdef COUNT_TRAPS
  total_traps++;
#endif
}

Ttrap::Ttrap(coord2d _x2,coord2d _y11,coord2d _y12,coord2d _y21,coord2d _y22,int _un,int _dn)
{
  x2=_x2;
  un=_un;
  dn=_dn;
  y11=_y11;
  y12=_y12;
  y21=_y21;
  y22=_y22;
#ifdef COUNT_TRAPS
  total_traps++;
#endif
}

Ttrap::Ttrap(coord2d _x2,coord2d _y11,coord2d _y12,coord2d _y21,coord2d _y22,Ttrap *_next)
{
  x2=_x2;
  y11=_y11;
  y12=_y12;
  y21=_y21;
  y22=_y22;
  next=_next;
#ifdef COUNT_TRAPS
  total_traps++;
#endif
}

Ttrap::Ttrap(coord2d _x2,coord2d _y11,coord2d _y12,coord2d _y21,coord2d _y22,int _un,int _dn,Ttrap *_next)
{
  x2=_x2;
  un=_un;
  dn=_dn;
  y11=_y11;
  y12=_y12;
  y21=_y21;
  y22=_y22;
  next=_next;
#ifdef COUNT_TRAPS
  total_traps++;
#endif
}

Ttrap::~Ttrap( void )
{
#ifdef COUNT_TRAPS
  total_traps--;
#endif
}
