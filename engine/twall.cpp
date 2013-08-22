//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Twall.cpp - class Twall implementation                             //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _TWALL_CPP
#include "boom.h"
#include "draw.h"
#include "read.h"
#include "write.h"

#include <stdio.h>

Twall::Twall():
  Tmap_item()
{
#ifdef EDITOR
  next=NULL;
#endif
}

// saves the wall to the current file
void Twall::save(Tmap *m,Tcluster *c,Tsector *s,Tline *l)
{
  wrchar(wtWALL);
  savewall();
}

// saves the wall information
void Twall::savewall( void )
{
  Tmap_item::save();
  // writes the texture
  wrlong( texture );
  // writes the heights
  wrfloat( z1c );
  wrfloat( z2c );
}

// loads the wall from the current file
bool Twall::load()
{
  Tmap_item::load();
  // reads the texture
  texture=rdlong();

  // reads the heights
  z1c=rdfloat();
  z2c=rdfloat();
  return true;
}

// initializes the wall after the loading
void Twall::postload(Tmap *m,Tcluster *c,Tsector *s,Tline *l)
{
}

// draws the wall
void Twall::draw(Tmonotone* mp)
{
  setdrawtexture(texture);
  drawmp(mp);
}
