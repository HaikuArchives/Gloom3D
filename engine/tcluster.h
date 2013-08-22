//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tcluster.h - class Tcluster interface                              //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TCLUSTER_H
#define _TCLUSTER_H

struct Tcont;

enum
{
  clVISITED = 0x02  // the cluster is visited during the drawing the current frame
};

// Tcluster contains all the information for one cluster in the map
class Tcluster: public Tmap_item
{
protected:
  Tcont *cont;  // contours with the visible lines
  int contn;  // number of the contours
  int conti;  // index of the current contour

public:
  Tsector *sectors; // sectors in the cluster
  int sectorsnum;   // number of the sectors in the cluster

  Tcluster();
  ~Tcluster();

  // saves the cluster to the current file
  void save(Tmap *m);
  // loads the cluster from the current file
  bool load();
  // unloads the cluster (releases all resources allocated by load())
  void unload();
  // initializes the cluster after the loading
  void postload(Tmap *m);
  
  // sorts the contours in the cluster
  void sort_contours(int n);
  // removes the contours after the drawing is complete
  void reset_contours();
  // draws the current cluster
  void draw();
  // checks if the point (x,y) is inside the cluster
  Tsector *inside(coord3d x,coord3d y);

#ifdef EDITOR
  Tcluster *next;
  bool visible;
#endif
};

#ifndef EDITOR
#define NEXTCLUSTER(c) (c)++
#else
#define NEXTCLUSTER(c) (c)=(c)->next
#endif

#ifdef _TCLUSTER_CPP
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

GLOBAL int MIN_LX,MAX_LX;

#undef GLOBAL

#endif
