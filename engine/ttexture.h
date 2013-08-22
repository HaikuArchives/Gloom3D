//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Ttexture.h - texture handling interface                            //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TTEXTURE_H
#define _TTEXTURE_H

#include "types.h"
#ifdef DIRECT_3D
#include "texd3d.h"
#endif

#define MAX_TEXTURENAME 32

struct Ttexture
{
  int wd, he; // image size
  char name[MAX_TEXTURENAME]; // name of the texture
#ifdef DIRECT_3D
  hTexture h;   // texture handle
#else
  byte *datap;  // data buffer
#endif
};

#ifdef _TTEXTURE_CPP
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

#define MAX_TEXTURES 256

GLOBAL Ttexture textures[MAX_TEXTURES];
GLOBAL int ntextures;        // number of textures
GLOBAL byte palette[256][3]; // 256 color palette 

#undef GLOBAL

// loads all textures
void loadtextures( void );

#ifndef DIRECT_3D
// saves all textures
void savetextures( void );
// frees the allocated memory
void freetextures( void );
#endif

#endif
