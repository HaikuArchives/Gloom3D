//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Ttexture.cpp - texture handling implementation                     //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _TTEXTURE_CPP
#include "ttexture.h"

#ifdef DIRECT_3D
#include"texd3d.h"
#endif

#include "read.h"
#include "write.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h>

// mipmap number in Direct 3D
#define MIPMAPS 4

// reads a single texture
static void readtexture(Ttexture *txp)
{
  // reads the size
  txp->wd=rdlong();
  txp->he=rdlong();
  // reads the name
  rdbuf(txp->name,sizeof(txp->name));
  // allocates a buffer for the image
  byte *datap=(byte *)malloc(txp->wd*txp->he);
  if (!datap) return;
  // reads the image
  rdbuf(datap,txp->wd*txp->he);
#ifdef DIRECT_3D
  // in case of Direct 3D the texture is loaded into a surface
  txp->h=texture_load(tf8PAL,datap,palette,txp->wd,txp->he,MIPMAPS);
  free(datap);
#else
  txp->datap=datap;
#endif
}

// loads all textures
void loadtextures( void )
{
  // reads the palette
  rdbuf(palette,sizeof(palette));
  // reads the number of the textures
  ntextures=rdlong();
  // reads the textures
  for (int i=0;i<ntextures;i++)
    readtexture(textures+i);
}

#ifndef DIRECT_3D
// writes a single texture
static void writetexture(Ttexture *txp)
{
  // writes the size
  wrlong(txp->wd);
  wrlong(txp->he);
  // writes the name
  wrbuf(txp->name,sizeof(txp->name));
  // writes the image
  wrbuf(txp->datap,txp->wd*txp->he);
}

// saves all textures
void savetextures( void )
{
  // writes the palette
  wrbuf(palette,sizeof(palette));
  // writes the number of the textures
  wrlong(ntextures);
  // writes the textures
  for (int i=0;i<ntextures;i++)
    writetexture(textures+i);
}

// frees the allocated memory
void freetextures( void )
{
  for (int i=0;i<ntextures;i++)
    if (textures[i].datap) free(textures[i].datap);
}
#endif
