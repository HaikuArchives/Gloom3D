//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Thole.h - class Thole interface                                    //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _THOLE_H
#define _THOLE_H

// Thole is a wall without texture that leads to another sector
class Thole: public Twall
{
public:
  Tsector *sector;  // target sector

  Thole();

  // saves the hole to the current file
  virtual void save(Tmap *m,Tcluster *c,Tsector *s,Tline *l);
  // loads the hole from the current file
  virtual bool load();
  // initializes the hole after the loading
  virtual void postload(Tmap *m,Tcluster *c,Tsector *s,Tline *l);
};

#endif
