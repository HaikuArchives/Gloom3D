//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tline.h - class Tline interface                                    //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TLINE_H
#define _TLINE_H

enum
{
  liCONTOUR_END = 0x01, // the line ends a contour
};

enum
{
  wtWALL   = 1, // plain wall
  wtHOLE   = 2, // a hole
  wtPORTAL = 3, // a portal (a hole to another cluster)
};

class Tsector;

#define MAX_WPL 10  // maximum walls per line

// Tline contains all the information for one line in the sector
class Tline: public Tmap_item
{
public:
  Twall **walls;  // walls in the sector
  int wallsnum;  // number of the walls in the sector

  int v1,v2;   // first and second vertex of the line
  coord2d len;    // length of the line

  Tline();

  // saves the line to the current file
  void save(Tmap *m,Tcluster *c,Tsector *s);
  // loads the line from the current file
  bool load();
  // unloads the line (releases all resources allocated by load())
  void unload();
  // initializes the line after the loading
  void postload(Tmap *m,Tcluster *c,Tsector *s);
  // this is called by the sector when the height is changed
  void changeheight(Tsector *s,bool wave);
  // draws the line
  void draw(Tsector *owner);

  // checks if the line is last of the contour
  inline bool contour_end()
  {
    return (options&liCONTOUR_END);
  }

#ifdef EDITOR
  Tline *next,*nextc;

  // returns the next line in the contour
  inline Tline *next_in_contour()
  {
    return nextc;
  }
#else
  byte nextc;

  // returns the next line in the contour
  inline Tline *next_in_contour()
  {
    if (contour_end()) return this-nextc;
    else return this+1;
  }
#endif

  // implementation of these functions is in collide.cpp
  // checks if l1 has vertex before the line
  bool before(Tline *l1);
  // checks the line for collisions
  bool collide(coord3d z,coord3d zf,coord3d zc,double t);
};

#ifndef EDITOR
#define NEXTLINE(l) (l)++
#else
#define NEXTLINE(l) (l)=(l)->next
#endif

#ifdef _TLINE_CPP
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

GLOBAL coord3d portx1,porty1,portdx,portdy; // coordinates of the current portal

#undef GLOBAL

#endif
