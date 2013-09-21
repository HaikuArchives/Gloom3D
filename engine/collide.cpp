//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   collide.cpp - collision detection implementaton                    //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _COLLIDE_CPP
#include "collide.h"
#include "boom.h"
#include "geom.h"
#include "player.h"
#include "tclip.h"

#include <stdio.h>
#include <math.h>

#define MIN_HEIGHT 80
#define CLIMB_HEIGHT 80
#define MIN_CLIMB_HEIGHT 60
#define DROP_HEIGHT 10
#define HEAD_HEIGHT 50
#define RADIUS 100    // collision radius

#define COLIDE_TRYS 5 // number of iterations for collision detection
#define MAX_COLIDE_LINES 10 // maximum lines with possible collision per iteration

static Tline *clines[MAX_COLIDE_LINES];
static double clinedist[MAX_COLIDE_LINES];
static int clinenum;
static int climb_height;
static coord3d gdx,gdy;

bool Tline::before(Tline *l)
{
  coord3d dx=verts[v2].x-verts[v1].x;
  coord3d dy=verts[v2].y-verts[v1].y;
  if ((verts[l->v1].x-verts[v1].x)*dy>(verts[l->v1].y-verts[v1].y)*dx) return true;
  if ((verts[l->v2].x-verts[v1].x)*dy>(verts[l->v2].y-verts[v1].y)*dx) return true;
  return false;
}

// moves the current point of view with the vector (dx,dy,dz) and sets the
// view angles to ax,ay and az
// returns true if successful
bool Tmap::move(coord3d dx,coord3d dy,coord3d dz,angle ax,angle ay,angle az)
{
  bool r=true;
  coord3d x,y,z;
  gdx=dx;
  gdy=dy;
  x=view.x+dx;
  y=view.y+dy;
  z=view.z+dz;
  climb_height=CLIMB_HEIGHT;
  if (height-climb_height<MIN_CLIMB_HEIGHT) climb_height=height-MIN_CLIMB_HEIGHT;
  for (int i=0;i<COLIDE_TRYS;i++) {
    // if there is no collision, exit
    if (!collision(x,y,&z)) break;
    // if this is the last iteration, don't move
    if (clinenum==0 || i==COLIDE_TRYS-1) {
      x=view.x;
      y=view.y;
      collision(x,y,&z);
      r=false;
      break;
    }
    // sorts the colliding lines
    {
      for (int i=0;i<clinenum-1;i++) {
        int j;
        for (j=i;j<clinenum;j++) {
          int k;
          for (k=i;k<clinenum;k++)
            if (k!=j && clines[j]->before(clines[k])) break;
          if (k==clinenum) break;
        }
        if (j==clinenum) {
          j=0;
          for (int k=1;k<clinenum;k++)
            if (clinedist[k]<clinedist[j]) j=k;
        }
        Tline *ql=clines[i];
        clines[i]=clines[j];
        clines[j]=ql;
        double qd=clinedist[i];
        clinedist[i]=clinedist[j];
        clinedist[j]=qd;
      }
    }
    // finds the point of the exact collision
    for (int li=0;li<clinenum;li++) {
      Tline *l=clines[li];
      coord3d x1=verts[l->v1].x;
      coord3d y1=verts[l->v1].y;
      coord3d x2=verts[l->v2].x;
      coord3d y2=verts[l->v2].y;
      coord3d lx1=(y2-y1);
      coord3d ly1=-(x2-x1);
      if (li>0) {
        if (((x-x1)*ly1-(y-y1)*lx1>0 && (x-x1)*(x-x1)+(y-y1)*(y-y1)>RADIUS*RADIUS) ||
            ((x-x2)*ly1-(y-y2)*lx1<0 && (x-x2)*(x-x2)+(y-y2)*(y-y2)>RADIUS*RADIUS))
          continue;
      }
      double ll=l->len;
      lx1/=ll;
      ly1/=ll;
      double ld=(x1-x)*lx1+(y1-y)*ly1+RADIUS;
      if (ld>0) {
        coord3d x1=x+ld*lx1;
        coord3d y1=y+ld*ly1;
        if ((x1-view.x)*dx+(y1-view.y)*dy>=0) x=x1,y=y1;
      }
    }
  }
  // sets the view point to the new position
  setviewpoint(x,y,z,ax,ay,az);
  return r;
}

// checks for collision with the map. returns the new z value in *z
bool Tmap::collision(coord3d x,coord3d y,coord3d *z)
{
  coord3d zc=10000;
  coord3d zf=-10000;
  Tcluster *c0=cur_cluster;
  cur_cluster=NULL;
  clinenum=0;
  int b1,b2=0;
  Tcluster *c=clusters;
  // clecks all the clusters for collision
  for (int i=0;i<clustersnum;i++,NEXTCLUSTER(c)) {
    Tsector *s=c->sectors;
    int n=c->sectorsnum;
    // clecks all the sectors for collision
    for (int j=0;j<n;j++,NEXTSECTOR(s)) {
      b1=s->inside(x,y,view.z);
      // if the current view position is inside s, update cur_cluster and cur_sector
      if (b1) {
        cur_cluster=c;
        cur_sector=s;
      }
      // checks for intersection with s
      b2|=s->collision(x,y,view.z);
      if (b1 || b2==1) {
        coord3d fl=s->getzf(x,y);
        coord3d cl=s->getzc(x,y);
        if (zf<fl) zf=fl;
        if (zc>cl) zc=cl;
      }
      b2&=~1;
    }
  }
  zfloor=zf;
  zceiling=zc;
  if ((b2&2) || !cur_cluster || zc<zf) return true;
  if (height>zc-zf-HEAD_HEIGHT) height=zc-zf-HEAD_HEIGHT;
  if (height<MIN_HEIGHT) return true;
  if (*z<zf+height) {
    *z=zf+height;
  }
  if (*z>zc-HEAD_HEIGHT) {
    *z=zc-HEAD_HEIGHT;
  }
  return false;
}

// checks if (x,y) is inside the sector (in 2D)
bool Tsector::inside(coord3d x,coord3d y)
{
  int n=0;
  int i;
  if (x<minx || x>maxx || y<miny || y>maxy) return false;
  Tline *l;
  for (i=0,l=lines;i<linesnum;NEXTLINE(l),i++) {
    coord3d x1=verts[l->v1].x-x;
    coord3d y1=verts[l->v1].y-y;
    coord3d x2=verts[l->v2].x-x;
    coord3d y2=verts[l->v2].y-y;
    if (x1<0 && x2<0) continue;
    if (y1==y2) continue;
    if (y1>y2) {
      coord3d q=y1;y1=y2;y2=q;
      q=x1;x1=x2;x2=q;
    }
    if (y2<=0) continue;
    if (y1>0) continue;
    if (x1==0 && x2==0) return true;
    if (x1>=0 && x2>=0) n++;
    else {
      double q=x1*y2-x2*y1;
      if (q==0) return true;
      if (q>0) n++;
    }
  }
  return (n&1)!=0;
}

// checks if (x,y,z) is inside the sector (in 3D)
bool Tsector::inside(coord3d x,coord3d y,coord3d z)
{
  if (x<minx || x>maxx || y<miny || y>maxy) return false;
  if (z<getzf(x,y) || z>getzc(x,y)) return false;
  return inside(x,y);
}

double inline sqr(double d)
{
  return d*d;
}

// checks if the sphere with center (x,y,z) intersects the sector
//returns 1 if the sphere intersects the sector
//returns 3 if the sphere intersects one of the lines of the sector
int Tsector::collision(coord3d x,coord3d y,coord3d z)
{
  int f=0;
  int i;
  Tline *l;
  if (z<getzf(x,y) || z>getzc(x,y)) return 0;
  for (i=0,l=lines;i<linesnum;NEXTLINE(l),i++) {
    coord3d x1=verts[l->v1].x-x;
    coord3d y1=verts[l->v1].y-y;
    coord3d x2=verts[l->v2].x-x;
    coord3d y2=verts[l->v2].y-y;
    if (x1>RADIUS && x2>RADIUS) continue;
    if (y1>RADIUS && y2>RADIUS) continue;
    if (x1<-RADIUS && x2<-RADIUS) continue;
    if (y1<-RADIUS && y2<-RADIUS) continue;
    coord3d dx=x2-x1;
    coord3d dy=y2-y1;
    bool b1=(x2-RADIUS)*dy<(y2-RADIUS)*dx;
    bool b2=(x2+RADIUS)*dy<(y2-RADIUS)*dx;
    bool b3=(x2+RADIUS)*dy<(y2+RADIUS)*dx;
    bool b4=(x2-RADIUS)*dy<(y2+RADIUS)*dx;
    if (b1!=b2 || b2!=b3 || b3!=b4 || b4!=b1) {
      double l1=x1*x1+y1*y1;
      double l2=x2*x2+y2*y2;
      if (l1>l2) l1=l2;
      double q=dx*dx+dy*dy;
      double t=-(x1*dx+y1*dy)/q;
      if (t>0 && t<1) {
        double l3=sqr(fabs(x1*y2-x2*y1))/q;
        if (l1>l3) l1=l3;
      }
      else if (t<0) t=0;
      else if (t>1) t=1;
      if (l1+0.01>=RADIUS*RADIUS) continue;
      f|=1;
      if (dx*(verts[l->v1].y-view.y)-dy*(verts[l->v1].x-view.x)>0) {
        coord3d zf=splitab(getzf(verts[l->v1].x,verts[l->v1].y),
          getzf(verts[l->v2].x,verts[l->v2].y),t,1-t);
        coord3d zc=splitab(getzc(verts[l->v1].x,verts[l->v1].y),
          getzc(verts[l->v2].x,verts[l->v2].y),t,1-t);
        if (l->collide(z,zf,zc,t)) {
          clinedist[clinenum]=l1;
          clines[clinenum++]=l;
          f=3;
        }
      }
    }
  }
  return f;
}

// checks the line for collisions
bool Tline::collide(coord3d z,coord3d zf,coord3d zc,double t)
{
  Twall **w=walls;

  if (z<zf || z>zc) return false;
  if (!w) return false;

  int wi;
  for (wi=0;wi<wallsnum;wi++,NEXTWALL(w)) {
    coord3d zq=splitab((*w)->z1c,(*w)->z2c,t,1-t);
    if (is_solid(*w)) {
      if ((zf<z+DROP_HEIGHT) && (zq>z-climb_height)) return true;
    }
    else if ((z>=zf+climb_height) && (z<=zq-DROP_HEIGHT)) return false;
    zf=zq;
  }
  return false;
}
