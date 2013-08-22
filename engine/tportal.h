//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tportal.h - class Tportal interface                                //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TPORTAL_H
#define _TPORTAL_H

#include "tclip.h"

// Tportal is a wall without texture that leads to another cluster
// it combines the features of hole and clip
class Tportal: public Thole, public Tclip
{
public:
  Tcluster *target; // target cluster

  Tportal();

  // saves the portal to the current file
  virtual void save(Tmap *m,Tcluster *c,Tsector *s,Tline *l);
  // loads the portal from the current file
  virtual bool load();
  // initializes the portal after the loading
  virtual void postload(Tmap *m,Tcluster *c,Tsector *s,Tline *l);
  // draws the portal
  virtual void draw(Tmonotone *mp);
};

#endif
