//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tportal.cpp - class Tportal implementation                         //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _TPORTAL_CPP
#include "boom.h"
#include "read.h"
#include "write.h"


Tportal::Tportal():
  Thole()
{
}

// saves the portal to the current file
void Tportal::save(Tmap *m,Tcluster *c,Tsector *s,Tline *l)
{
  wrchar(wtPORTAL);
  savewall();
#ifdef EDITOR
  int si;
  Tsector *s0;
  for (si=0,s0=target->sectors;s0!=sector;si++,NEXTSECTOR(s0));
  // writes the index of the target sector
  wrlong(si);
  int ci;
  Tcluster *c0;
  for (ci=0,c0=m->clusters;c0!=target;ci++,NEXTCLUSTER(c0));
  // writes the index of the target cluster
  wrlong(ci);
#else
  // writes the index of the target sector
  wrlong(sector-target->sectors);
  // writes the index of the target cluster
  wrlong(target-m->clusters);
#endif
}

// loads the portal from the current file
bool Tportal::load()
{
  Thole::load();
  // reads the index of the target cluster
  *(long *)&target = rdlong();
  return true;
}

// initializes the portal after the loading
void Tportal::postload(Tmap *m,Tcluster *c,Tsector *s,Tline *l)
{
  // converts cluster index to cluster pointer
  target=m->getcluster(*(long *)&target);
  // converts sector index to sector pointer
#ifdef EDITOR
  int si;
  Tsector *s0;
  for (si=0,s0=target->sectors;si<*(long *)&sector;si++,NEXTSECTOR(s0));
  sector=s0;
#else
  sector=target->sectors+*(long *)&sector;
#endif
}

// draws the portal
void Tportal::draw(Tmonotone *mp)
{
  // stores the current clip
  Tclip *old=cur_clip;

  // sets the portal to be the current clip
  xmin=mp->xmin;
  xmax=mp->xmax;
  ymin=mp->ymin;
  ymax=mp->ymax;
  traps=mp->traps;
  set_cur_clip(this);

  // draws the target cluster
  target->draw();

  // restores the current clip
  mp->traps=traps;
  cur_clip=old;
}
