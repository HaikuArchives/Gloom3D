//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tmapitem.cpp - class Tmap_item implementation                      //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _TMAPITEM_CPP
#include "tmapitem.h"
#include "read.h"
#include "write.h"


Tmap_item::Tmap_item()
{
  options = 0;
}

// saves the map item to the current file
void Tmap_item::save()
{
  wrchar( options );
}

// loads the map item from the current file
bool Tmap_item::load()
{
  options = rdchar();
  return true;
}

//marks the map item as 'dirty'
void Tmap_item::dirty()
{
  options |= miDIRTY;
}
