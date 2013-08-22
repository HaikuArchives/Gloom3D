//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   player.cpp - player movement implementation                        //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _PLAYER_CPP
#include "player.h"
#include "boom.h"
#include "geom.h"
#include "collide.h"
#include "read.h"

#include <math.h>
#include <stdio.h>

#ifdef BOOMD3D
#include <malloc.h>
#include <windows.h>
#else
#include <time.h>
#endif

static angle ang[3]={0,0,0};

static Tvector v,a,f;
static angle da;
static bool jumpfl,groundfl=true,collisionfl=false;

#define DANG      3

#define FORCE     7000
#define GRAVITY   8000
#define FRICTION  5
#define MINSPEED  10
#define BOUNCE    0

#define HEIGHT    250
#define CROUCHHEIGHT 150
#define STAND     500
#define JUMPSPEED 1500
#define MAXCLIMB  1000
#define MINSLOPE  0.4

// calculates the force and the view angle according the pressed keys
static bool process_kbd(double time)
{
  dword kbd=player_keys;
  angle dang;
  coord3d force;
  f.x=0;f.y=0;da=0;

  dang=DANG;force=FORCE;
  if (kbd & kRUN) {
    dang*=2,force*=2;
  }
  if (kbd & kLEFT)  da += dang;
  if (kbd & kRIGHT) da -= dang;
  if (kbd & kLOOKUP)  ang[0] -= 4*time;
  if (kbd & kLOOKDOWN) ang[0] += 4*time;
  if (kbd & kROTLEFT)  ang[1] -= 4*time;
  if (kbd & kROTRIGHT) ang[1] += 4*time;
  if (kbd & kCENTER) {
    ang[0]=ang[1]=0;
    v.x=v.y=0;
  }

  if (kbd & kFORWARD)   f.y += force;
  if (kbd & kBACKWARD)  f.y -= force;
  if (kbd & kCROUCH)    height = CROUCHHEIGHT;
  if (kbd & kMOVELEFT)  f.x -= force;
  if (kbd & kMOVERIGHT) f.x += force;

  if (ang[0]<-0.6) ang[0]=-0.6;
  if (ang[0]> 0.6) ang[0]= 0.6;
  
  jumpfl=(kbd & kJUMP)!=0;
  return (kbd&kQUIT)==0;
}

// rotates the player with the given angles
void player_rotate(angle ax,angle ay,angle az)
{
  ang[0]+=ax;
  ang[1]+=ay;
  ang[2]+=az;
  if (ang[0]<-0.6) ang[0]=-0.6;
  if (ang[0]> 0.6) ang[0]= 0.6;
}

// advances the player movement with the elapsed time
bool player_idle(long time)
{
  double t; // time from the previous frame in seconds
  static long old_time=0;
  if (old_time==0) old_time=time;
  t=(time-old_time)/1000.;
  if (t>0.1) t=0.1;
  old_time=time;
  bool r=process_kbd(t);

  if (!cur_cluster) {
    // if the player is not in any cluster, don't move
    if (map->move(0,0,0,0,0,0)) {
      v.x=0;v.y=0;v.z=0;
      a.x=0;a.y=0;a.z=0;
      return r;
    }

    // if the current position is invalid, find a new one
    Tcluster *c;
    int ci;
    for (ci=0,c=map->clusters;ci<map->clustersnum;NEXTCLUSTER(c),ci++) {
      Tsector *s;
      int si;
      for (si=0,s=c->sectors;si<c->sectorsnum;NEXTSECTOR(s),si++)
        if (s->inside(view.x,view.y)) {
          coord3d dz=s->getzf(view.x,view.y)+CROUCHHEIGHT-view.z;
          if (map->move(0,0,dz,ang[0],ang[1],ang[2])) return r;
        }
    }
    return r;
  }

  // calculates the acceleration
  if (groundfl) {
    ang[2]+=da*t;
    a.x=f.x*cos(ang[2])-f.y*sin(ang[2]);
    a.y=f.x*sin(ang[2])+f.y*cos(ang[2]);
    a.x-=v.x*FRICTION;
    a.y-=v.y*FRICTION;
  }
  else a.x=0,a.y=0;
  // calculates the gravity according the current slope of the ground
  a.z=-GRAVITY;
  if (groundfl && cur_sector && view.z<cur_sector->getzf(view.x,view.y)+HEIGHT*2) {
    double sx=-cur_sector->zfa;
    double sy=-cur_sector->zfb;
    if (sx || sy) {
      double sz=-sx*sx-sy*sy;
      double l=1/sqrt(sx*sx+sy*sy+sz*sz);
      sx*=l;sy*=l;sz*=l;
      if (sz<-MINSLOPE) {
        double g=-GRAVITY*3*sz;
        a.x+=sx*g;
        a.y+=sy*g;
      }
    }
  }

  // calculates the current position and velocity
  coord3d dx=v.x*t+a.x*t*t/2;
  coord3d dy=v.y*t+a.y*t*t/2;
  coord3d dz=v.z*t+a.z*t*t/2;

  coord3d vx=v.x+a.x*t;
  coord3d vy=v.y+a.y*t;
  if (f.x==0 && f.y==0) {
    if (collisionfl || (vx<0 && v.x>=0) || (vx>0 && v.x<=0) || fabs(v.x)<MINSPEED) vx=0;
    if (collisionfl || (vy<0 && v.y>=0) || (vy>0 && v.y<=0) || fabs(v.y)<MINSPEED) vy=0;
  }
  v.x=vx;v.y=vy;
  v.z+=a.z*t;

  coord3d x=view.x;
  coord3d y=view.y;
  coord3d z=view.z+dz;

  // moves to the new position
  map->move(dx,dy,dz,ang[0],ang[1],ang[2]);
#if BOUNCE
  // if there is a collision, bounce from the wall
  if (view.x!=x+dx)
    v.x=(view.x-x)/t;
  if (view.y!=y+dy)
    v.y=(view.y-y)/t;
#endif
  // checks if the player touches the ground
  groundfl=view.z>z;
  if (zceiling-zfloor<CROUCHHEIGHT) view.z=(zfloor+zceiling)/2,groundfl=true;
  if (view.z<zfloor+10) view.z=zfloor+10;
  if (groundfl) {
    if (view.z>MAXCLIMB*t+z) view.z=MAXCLIMB*t+z;
    v.z=0;
    // handles the jump
    if (jumpfl) v.z=JUMPSPEED;
    else if (height<HEIGHT) {
      height+=STAND*t;
      if (height>HEIGHT) height=HEIGHT;
    }
  }
  else if (-v.z/GRAVITY>0.2) height=CROUCHHEIGHT;
  return r;
}

// initialize the player movement
void player_init(coord3d x,coord3d y,coord3d z)
{
  view.x=x;
  view.y=y;
  view.z=z;
  v.x=0;v.y=0;v.z=0;
  a.x=0;a.y=0;a.z=0;
  ang[0]=ang[1]=ang[2]=0;
  height=250;
}
