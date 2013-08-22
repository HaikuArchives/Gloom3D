//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tmap.h - class Tmap interface                                      //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TMAP_H
#define _TMAP_H

#define MAP_VERSION 10
#define MAP_ID 'BOOM'

// maximum number of vertices per map (only when EDITOR is defined)
#define MAX_VERTICES 10000

// Tmap contains all the information for one level
class Tmap
{
public:
  Tcluster *clusters; // clusters in the map
  int clustersnum;    // number of the clusters in the map
  coord3d start_x, start_y, start_z; // start position

  Tmap();
  // saves the map to the current file
  void save();
  // loads the map from the current file
  bool load();
  // unloads the map (releases all resources allocated by load())
  void unload();
  // initializes all items in the map after the loading
  void postload( void );
  // returns the address of the cluster with number "target"
  Tcluster *getcluster(int target);

  // draws the map from the current point of view
  void draw();

  // implementation of these functions is in collide.cpp
  // moves the current point of view with the vector (dx,dy,dz) and sets the
  // view angles to ax,ay and az
// returns true if successful
  bool move(coord3d dx,coord3d dy,coord3d dz,angle ax,angle ay,angle az);
  // checks for collision with the map. returns the new z value in *z
  bool collision(coord3d x,coord3d y,coord3d *z);
};


#ifdef _TMAP_CPP
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

// the sector containing the view point
GLOBAL Tsector *cur_sector;
// the cluster containing the current sector. the drawing always begins from it
GLOBAL Tcluster *cur_cluster;
// current map
GLOBAL Tmap *map;
// array with the vertices in the current map
GLOBAL Tvertex *verts;
// number of the vertices in the current map
GLOBAL int vertsnum;

#undef GLOBAL

// loads a map from file fn
bool map_init(char *fn);
// frees all allocated resources
void map_done( void );
// draws the map on the screen
bool map_draw( void );

#endif
