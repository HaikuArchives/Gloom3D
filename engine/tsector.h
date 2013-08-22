//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tsector.h - class Tsector interface                                //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TSECTOR_H
#define _TSECTOR_H

#include "ttrap.h"
#include "ttexture.h"

struct Tcont;

enum
{
  seLAST_SECTOR = 0x01, //it's the last sector
};

// Tcluster contains all the information for one sector in the cluster
class Tsector: public Tmap_item
{
public:
  Tline *lines; // lines in the sector
  int linesnum; // number of the lines in the sector

  float zfa,zfb,zfc,zca,zcb,zcc;  // floor and ceiling equation coeficients
  int tfloor, tceiling; // floor and ceiling textures
  coord3d minx,maxx,miny,maxy;  // sector bounding box

  Tsector();

  // saves the sector to the current file
  void save(Tmap *m,Tcluster *c);
  // loads the sector from the current file
  bool load();
  // unloads the sector (releases all resources allocated by load())
  void unload();
  // initializes the sector after the loading
  void postload(Tmap *m,Tcluster *c);

  // creates the contours from the lines of the sector
  int build_contours(Tcont *c);
  // fills polygon fp with the texture of the floor
  void draw_floor(Tmonotone *mp);
  // fills polygon fp with the texture of the ceiling
  void draw_ceiling(Tmonotone *mp);
  // returns the height of the floor at point (x,y)
  coord3d getzf(coord3d x,coord3d y);
  // returns the height of the ceiling at point (x,y)
  coord3d getzc(coord3d x,coord3d y);
  // must be called when the height of the floor or ceiling is changed
  void changeheight( void );
  // returns the line with ends v1 and v2
  Tline *getline(int v1,int v2);

  // implementation of these functions is in collide.cpp
  // checks if (x,y) is inside the sector (in 2D)
  bool inside(coord3d x,coord3d y);
  // checks if (x,y,z) is inside the sector (in 3D)
  bool inside(coord3d x,coord3d y,coord3d z);
  // checks if the sphere with center (x,y,z) intersects the sector
  int collision(coord3d x,coord3d y,coord3d z);

#ifdef EDITOR
  Tsector *next;
#endif
};

#ifndef EDITOR
#define NEXTSECTOR(s) (s)++
#else
#define NEXTSECTOR(s) (s)=(s)->next
#endif


#define MAX_LINES_PER_CONTOUR 20

// contour (used in realtime sorting)
struct Tcont
{
  Tsector *s; // sector of the contour
  Tline *l1;  // first line of the contour
  int n;      // number of the lines in the contour
  coord2d x1,x2;  // minimal and maximal x coordinate of the projection on the screen
  coord3d minz,maxz;  // minimal and maximal z coordinate in camera space
  coord2d x[MAX_LINES_PER_CONTOUR]; // x coordinates of the projection on the screen
};

#endif
