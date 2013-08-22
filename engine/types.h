//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   types.h - basic type and macro definitions                         //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TYPES_H
#define _TYPES_H

//#pragma warning( disable: 4244 )
//#pragma warning( disable: 4305 )

////////////////////////////////////////////////////////////////////////////

//general typedefs
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef float angle;
typedef double coord3d;
typedef float coord2d;

////////////////////////////////////////////////////////////////////////////

//general struct definitions
struct Tvertex {
  coord3d x,y;
};

struct Tvector {
  coord3d x,y,z;
};

#endif
