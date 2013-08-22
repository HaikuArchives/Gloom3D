//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tcluster.cpp - class Tcluster implementation                       //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _TCLUSTER_CPP
#include "boom.h"
#include "read.h"
#include "write.h"
#include "geom.h"
#include "draw.h"
#include "tclip.h"

#include <assert.h>
#include <malloc.h>
#include <stdio.h>

Tcluster::Tcluster():
  Tmap_item()
{
  sectors = NULL;
  cont = NULL;
}

Tcluster::~Tcluster()
{
  if (cont) free( cont );
}

// saves the cluster to the current file
void Tcluster::save(Tmap *m)
{
  Tmap_item::save();
  wrlong( sectorsnum );
  int i;
  Tsector *s;
  for (i=0,s=sectors;i<sectorsnum;NEXTSECTOR(s),i++)
    s->save(m,this);
}

// loads the cluster from the current file
bool Tcluster::load()
{
  Tmap_item::load();
  int n = rdlong();
  if (n) {
    CONSTRUCT_AND_LOAD_ARRAY( sectors, Tsector, n );
  }
  sectorsnum=n;
  return true;
}

// initializes the cluster after the loading
void Tcluster::postload(Tmap *m)
{
  Tsector *s=sectors;
  for (int i=0;i<sectorsnum;i++,NEXTSECTOR(s))
    s->postload(m,this);
}

// unloads the cluster (releases all resources allocated by load())
void Tcluster::unload()
{
  if (sectors) {
    Tsector *s=sectors;
    for (int i=0;i<sectorsnum;i++,NEXTSECTOR(s))
      s->unload();
    DESTRUCT_ARRAY(sectors);
  }
}

// checks if the point (x,y) is inside the cluster
Tsector *Tcluster::inside(coord3d x,coord3d y)
{
  Tsector *s=sectors;
  for (int i=0;i<sectorsnum;i++,NEXTSECTOR(s)) {
    if (s->inside(x,y)) return s;
  }
  return NULL;
}

static inline int front(coord3d x1,coord3d y1,coord3d x2,coord3d y2,coord3d x,coord3d y)
{
  double d=(x2-x)*(y2-y1)-(y2-y)*(x2-x1);
  return d;
}

// compares two contours
static bool before(Tcont *c1,Tcont *c2)
{
  if (c1->x1>=c2->x2 || c2->x1>=c1->x2) return false;
  if (c1->minz>c2->maxz) return false;
  Tline *l1 = c1->l1;
  Tline *l2 = c2->l1;
  if (c1->x1<c2->x1) {
    for (int i=0;i<c1->n;i++,l1=l1->next_in_contour())
      if (c1->x[i]>c2->x1) break;
  }
  else {
    for (int j=0;j<c2->n;j++,l2=l2->next_in_contour())
      if (c2->x[j]>c1->x1) break;
  }
  int f1=front(verts[l1->v1].x,verts[l1->v1].y,verts[l1->v2].x,verts[l1->v2].y,verts[l2->v1].x,verts[l2->v1].y);
  int f2=front(verts[l1->v1].x,verts[l1->v1].y,verts[l1->v2].x,verts[l1->v2].y,verts[l2->v2].x,verts[l2->v2].y);
  if (f1>=0 && f2>=0) return true;
  if (f1<=0 && f2<=0) return false;
  f1=front(verts[l2->v1].x,verts[l2->v1].y,verts[l2->v2].x,verts[l2->v2].y,verts[l1->v1].x,verts[l1->v1].y);
  if (f1==0) f1=front(verts[l2->v1].x,verts[l2->v1].y,verts[l2->v2].x,verts[l2->v2].y,verts[l1->v2].x,verts[l1->v2].y);
  return (f1<0);
}

// sorts the contours in the cluster
// performs topological sorting algorithm
void Tcluster::sort_contours(int n)
{
  int i, j, k;

  for (k=0;k<n;k++) {
    for (i=k;i<n;i++) {
      for (j=k;j<n;j++)
        if (i!=j && before(cont+j,cont+i)) break;
      if (j==n) {
        if (i!=k) {
          Tcont q=cont[i];
          cont[i]=cont[k];
          cont[k]=q;
        }
        break;
      }
    }
    if (i==n) break;
  }
}

// removes the contours after the drawing of the frame is complete
// marks the cluster as not visited
void Tcluster::reset_contours()
{
  if (cont) free( cont );
  cont = NULL;
  options&=~clVISITED;
}


#define MAX_CONT 40

// draws the cluster
void Tcluster::draw()
{
  if (!(options&clVISITED)) {
    // if the cluster is visited for the first time this frame
    options|=clVISITED;
    // allocate memory for the contours
    cont=(Tcont *)malloc(MAX_CONT*sizeof(Tcont));
    // calculate the contours
    Tsector *s=sectors;
    int i;
    int cn=0;
    for (i=0;i<sectorsnum;i++,NEXTSECTOR(s))
      cn += s->build_contours(cont+cn);

    // sort the contours
    sort_contours(cn);
    contn=cn;
    conti=-1;
  }

  int oldconti = conti;
  // draw the lines from every contour
  for (conti++;conti<contn;conti++) {
    Tsector *s=cont[conti].s;
    int n = cont[conti].n+1;
    for (Tline *l=cont[conti].l1;--n;l=l->next_in_contour())
      l->draw( s );
  }
  conti=oldconti;
}
