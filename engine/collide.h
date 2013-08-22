//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   collide.h - collision detection interface                          //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _COLLIDE_H
#define _COLLIDE_H

#include "types.h"

#ifdef _COLLIDE_CPP
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

GLOBAL coord3d zfloor,zceiling; // current floor and ceiling height
  
#undef GLOBAL


#endif
