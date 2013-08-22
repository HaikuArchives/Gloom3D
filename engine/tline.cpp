//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tline.cpp - class Tline implementation                             //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _TLINE_CPP
#include "boom.h"
#include "read.h"
#include "write.h"
#include "draw.h"
#include "geom.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

Tline::Tline():
  Tmap_item()
{
  walls = NULL;
}

// saves the line to the current file
void Tline::save(Tmap *m,Tcluster *c,Tsector *s)
{
  Tmap_item::save();
  // writes first and second ends
  wrlong( v1 );
  wrlong( v2 );

  // writes all the walls
  wrlong(wallsnum);
  Twall **w;
  int i;
  for (i=0,w=walls;i<wallsnum;NEXTWALL(w),i++)
    (*w)->save(m,c,s,this);
}

// loads the line from the current file
bool Tline::load()
{
  Tmap_item::load();
  // reads first and second ends
  v1 = rdlong();
  v2 = rdlong();

  // reads all the walls
  int n = rdlong();
  if (n==0) return false;
#ifndef EDITOR
  walls = (Twall **)malloc( n*sizeof(Twall*) );
#else
  walls = (Twall **)malloc( sizeof(Twall*) );
#endif
  if (!walls) return false;
  Twall **w;
  int i;
  for (i=0,w=walls;i<n;i++,NEXTWALL(w)) {
    byte kind = rdchar();
    // create the walls
    switch (kind) {
      case wtWALL:
        (*w) = new Twall();
        break;
      case wtHOLE:
        (*w) = new Thole();
        break;
      case wtPORTAL:
        (*w) = new Tportal();
        break;
    }
    if (!(*w)->load()) {
#ifndef EDITOR
      for (;w>=walls;w--)
        delete *w;
#else
      Twall *w0,*n;
      for (w0=*walls;w0;w0=n) {
        n=w0->next;
        delete w0;
      }
#endif      
      free(walls);
      return false;
    }
  }
  wallsnum=n;
  return true;
}

// initializes the line after the loading
void Tline::postload(Tmap *m,Tcluster *c,Tsector *s)
{
  // calcultes the length of the line
  coord3d dx=verts[v2].x-verts[v1].x;
  coord3d dy=verts[v2].y-verts[v1].y;
  len=sqrt(dx*dx+dy*dy);

  //calls postload for all the walls
  Twall **w=walls;
  for (int i=0;i<wallsnum;i++,NEXTWALL(w))
    (*w)->postload(m,c,s,this);
}

// unloads the line (releases all resources allocated by load())
void Tline::unload()
{
  // unloads all the walls
  if (walls) {
#ifndef EDITOR
    Twall **w = walls;
    for (int i=0;i<wallsnum;i++,w++)
      delete (*w);
#else
    Twall *w = *walls;
    Twall *n;
    for (int i=0;i<wallsnum;i++,w=n) {
      n=w->next;
      delete  w;
    }
#endif
    free( walls );
    walls = NULL;
  }
}

#define MINZ 0.1 // near plane

#define clCLIPLEFT  1 // the line is clipped on the left
#define clCLIPRIGHT 2 // the line is clipped on the right
#define clDRAWLEFT  4 // the left end if the line is too close. fill to the left of the screen
#define clDRAWRIGHT 8 // the right end if the line is too close. fill to the right of the screen
#define clLEFT (clCLIPLEFT|clDRAWLEFT)
#define clRIGHT (clCLIPRIGHT|clDRAWRIGHT)
#define clCLIP (clCLIPLEFT|clCLIPRIGHT)
#define clDRAW (clDRAWLEFT|clDRAWRIGHT)

// checks if (x,y) is infront of the current portal
static bool infront(coord3d x,coord3d y)
{
  return (x-portx1)*portdy<(y-porty1)*portdx;
}

// draws the line
void Tline::draw( Tsector *owner )
{
  coord3d x1r,y1r,x2r,y2r;
  coord3d t1,t2;
  coord2d x1,x2,x1c,x2c;
  Tmonotone mp,mph;
  Twall **w;

  coord3d ox1=verts[v1].x;
  coord3d oy1=verts[v1].y;
  coord3d ox2=verts[v2].x;
  coord3d oy2=verts[v2].y;
  // if the line is behind the portal, do nothing
  if (!infront(ox1,oy1) && !infront(ox2,oy2)) return;
  coord3d _portx1=portx1;
  coord3d _porty1=porty1;
  coord3d _portdx=portdx;
  coord3d _portdy=portdy;
  rotatez(ox1,oy1,&x1r,&y1r); // find the first vertex in camera space
  rotatez(ox2,oy2,&x2r,&y2r); // find the second vertex in camera space
  coord3d orx1=x1r;
  coord3d orx2=x2r;

  // calculates the vectors for the texture mapping
  Tvector a={x1r,y1r,view.z};
  Tvector p={512*(x2r-x1r)/len,512*(y2r-y1r)/len,0};
  Tvector q={0,0,512};
  rotateyxv(&a);
  rotateyxv(&p);
  rotateyxv(&q);
  int clipfl=0;
  // checks if the line is visible on the screen
  if (y1r<=MINZ && y2r<=MINZ) {
    if (y1r<0 && y2r<0) return;
    if (x1r<0 && x2r<0) return;
    if (x1r>0 && x2r>0) return;
    if (y1r*fabs(x2r)+y2r*fabs(x1r)<0) return;
    x1c=x2c=0;
    // if the line is too close and is visible, it fills the whole screen
    clipfl=clLEFT|clRIGHT;
  }
  else {
    // if the line crosses the near plane, clip it
    if (y1r<MINZ || y2r<MINZ) {
      if (y1r<MINZ) {
        clipfl=clLEFT;
        t1=MINZ-y1r;
        t2=y2r-MINZ;
        x1r=splitab(x1r,x2r,t1,t2);
        y1r=MINZ;
      }
      if (y2r<MINZ) {
        clipfl=clRIGHT;
        t1=y1r-MINZ;
        t2=MINZ-y2r;
        x2r=splitab(x1r,x2r,t1,t2);
        y2r=MINZ;
      }
    }
    x1=projectx(x1r,y1r);
    x2=projectx(x2r,y2r);
    x1c=x1;
    x2c=x2;

    if (x1c<cur_clip->xmin) {
      x1c=cur_clip->xmin;
      clipfl&=~clDRAWLEFT;
    }
    if (x2c>cur_clip->xmax) {
      x2c=cur_clip->xmax;
      clipfl&=~clDRAWRIGHT;
    }
  }

  // if the line is perpendicular to the screen, or backfacing
  if (x1c>=x2c) {
    if (clipfl&clDRAW) {
      if (clipfl&clDRAWLEFT) x1c=cur_clip->xmin;
      if (clipfl&clDRAWRIGHT) x2c=cur_clip->xmax;
      if (x1c>=x2c) return;
      coord3d z0=(owner->getzf(ox1,oy1)*orx2-owner->getzf(ox2,oy2)*orx1)/(orx2-orx1);
      w=walls;
      for (int wi=0;wi<wallsnum;wi++,NEXTWALL(w)) {
        Twall *wp=*w;
        coord3d z1=(wp->z1c*orx2-wp->z2c*orx1)/(orx2-orx1);
        if (z0<=view.z && z1>=view.z) {
          // if there is a visible wall, fill the whole clip with it
          if (is_visible(wp)) goto dr;
          else break;
        }
        z0=z1;
      }
      return;
    dr:
      // fills the whole clip
      portx1=ox1;
      porty1=oy1;
      portdx=ox2-ox1;
      portdy=oy2-oy1;
      setdrawdata(&a,&p,&q);
      cur_clip->cut(x1c,x2c);
      cur_clip->clip(x1c,x2c,gymax,gymin,gymax,gymin,&mp);
      (*w)->draw(&mp);
      deltraps(mp.traps);
      cur_clip->last=mp.traps=new Ttrap(x2c,gymin,gymax,gymin,gymax,0,0,NULL);
      cur_clip->restore(mp.traps);
      portx1=_portx1;
      porty1=_porty1;
      portdx=_portdx;
      portdy=_portdy;
      return;
    }
    return;
  }


  portx1=ox1;
  porty1=oy1;
  portdx=ox2-ox1;
  portdy=oy2-oy1;
  // clips the line in world space (x and y in world space are needed to calculate the correct z)
  if (clipfl&clCLIPLEFT) {
    ox1=splitab(ox1,ox2,t1,t2);
    oy1=splitab(oy1,oy2,t1,t2);
  }
  else if (clipfl&clCLIPRIGHT) {
    ox2=splitab(ox1,ox2,t1,t2);
    oy2=splitab(oy1,oy2,t1,t2);
  }
  coord3d zf1=owner->getzf(ox1,oy1);
  coord3d zf2=owner->getzf(ox2,oy2);
  coord3d zc1=owner->getzc(ox1,oy1);
  coord3d zc2=owner->getzc(ox2,oy2);

  // finds the screen y coordinates (the screen x-es are x1c and x2c)
  coord2d pf1=projectz(zf1,y1r);// floor
  coord2d pf2=projectz(zf2,y2r);
  coord2d pc1=projectz(zc1,y1r);// ceiling
  coord2d pc2=projectz(zc2,y2r);

  // interpolates the z-s if the x-es are changed by cur_clip
  if (x1<x1c) {
    pf1=splitab(pf1,pf2,x1c-x1,x2-x1c);
    pc1=splitab(pc1,pc2,x1c-x1,x2-x1c);
  }
  if (x2>x2c) {
    pf2=splitab(pf1,pf2,x2c-x1c,x2-x2c);
    pc2=splitab(pc1,pc2,x2c-x1c,x2-x2c);
  }
  // cuts the portion of cur_clip that is over the walls
  cur_clip->cut(x1c,x2c);
  // clips the floor polygon with cur_clip
  cur_clip->clip(x1c,x2c,pf1,cur_clip->ymin,pf2,cur_clip->ymin,&mp);
  owner->draw_floor(&mp);
  deltraps(mp.traps);
  // clips the ceiling polygon with cur_clip
  cur_clip->clip(x1c,x2c,cur_clip->ymax,pc1,cur_clip->ymax,pc2,&mp);
  owner->draw_ceiling(&mp);
  deltraps(mp.traps);

  mph.traps=NULL;
  int wi;
  w=walls;
  // draws all the walls
  for (wi=0;wi<wallsnum;wi++,NEXTWALL(w)) {
    Twall *wp=*w;
    coord3d zq1=wp->z1c;
    coord3d zq2=wp->z2c;
    // interpolates z-s if necessary
    if (clipfl&clCLIPLEFT) zq1=splitab(zq1,zq2,t1,t2);
    if (clipfl&clCLIPRIGHT) zq2=splitab(zq1,zq2,t1,t2);
    // project the z-s
    coord2d pq1=projectz(zq1,y1r);
    coord2d pq2=projectz(zq2,y2r);
    // interpolates the projection
    if (x1<x1c) pq1=splitab(pq1,pq2,x1c-x1,x2-x1c);
    if (x2>x2c) pq2=splitab(pq1,pq2,x2c-x1c,x2-x2c);
    if (is_visible(wp)) {
      // if the wall is visible (wall or portal), draw it
      cur_clip->clip(x1c,x2c,pq1,pf1,pq2,pf2,&mp);
      if ((clipfl&clDRAW)) {
        if ((clipfl&clDRAWLEFT) && pq1>0 && pf1<0) cur_clip->addbeg(&mp);
        else if ((clipfl&clDRAWRIGHT) && pq2>0 && pf2<0) cur_clip->addend(&mp);
      }
      setdrawdata(&a,&p,&q);
      (*w)->draw(&mp);
      deltraps(mp.traps);
    }
    else {
      // if the wall is a hole, clip it only
      cur_clip->clip(x1c,x2c,pq1,pf1,pq2,pf2,&mph);
    }
    pf1=pq1;
    pf2=pq2;
  }

  // restores cur_clip
  if (mph.traps) {
    for (Ttrap *t=mph.traps;t->next;t=t->next);
    cur_clip->last=t;
  }
  else {
    cur_clip->last=mph.traps=new Ttrap(x2c,gymin,gymax,gymin,gymax,0,0,NULL);
  }
  cur_clip->restore(mph.traps);
  portx1=_portx1;
  porty1=_porty1;
  portdx=_portdx;
  portdy=_portdy;
}

#ifdef EDITOR
static coord3d xx,yy;

// compares two holes by their height
static int cmpholes(const void *h1,const void *h2)
{
  coord3d z1=(*(Thole **)h1)->sector->getzf(xx,yy);
  coord3d z2=(*(Thole **)h2)->sector->getzf(xx,yy);
  if (z1<z2) return -1;
  if (z1>z2) return 1;
  return 0;
}
#endif

// modifies the walls in the line if one if the heights is changed
void Tline::changeheight(Tsector *s,bool wave)
{
#ifdef EDITOR
extern int defwalltxt;
// in case of linked list, a wall can be added or deleted
  coord3d x1=verts[v1].x;
  coord3d y1=verts[v1].y;
  coord3d x2=verts[v2].x;
  coord3d y2=verts[v2].y;
  xx=x1;yy=y1;
  coord3d sf1=s->getzf(x1,y1);
  coord3d sf2=s->getzf(x2,y2);
  coord3d sc1=s->getzc(x1,y1);
  coord3d sc2=s->getzc(x2,y2);
  bool fl1,fl2=false;

  fl1=!is_hole(*walls);
  Twall *w;
  Twall *ww[MAX_WPL]; // walls
  Thole *wh[MAX_WPL]; // holes
  int wn=0; // number of walls
  int hn=0; // number of holes

  // fills ww and wh
  for (w=*walls;w;w=w->next) {
    if (is_hole(w)) {
      wh[hn]=(Thole *)w;
      hn++;
      fl2=false;
    }
    else {
      ww[wn]=w;
      wn++;
      fl2=true;
    }
  }

  int hi,wi=0,i;

  // sorts the holes by height
  if (hn) {
    qsort(wh,hn,sizeof(wh[0]),cmpholes);
    coord3d z1=wh[0]->sector->getzf(x1,y1);
    coord3d z2=wh[0]->sector->getzf(x2,y2);
    // adds a new wall at the bottom if necessary
    if (z1>sf1 || z2>sf2) {
      if (!fl1) {
        memmove(ww+1,ww,wn*sizeof(ww[0]));
        ww[0]=new Twall();
        ww[0]->options=waSOLID;
        ww[0]->texture=defwalltxt;
        wn++;
        wallsnum++;
        fl1=true;
      }
    }
    if (z1<sf1) z1=sf1;
    if (z2<sf2) z2=sf2;
    if (fl1) {
      ww[0]->z1c=z1;
      ww[0]->z2c=z2;
      wi++;
    }
  }

  // updates the heights of the walls. adds a new wall if necessary
  for (i=0;i<hn;i++) {
    Tsector *sc=wh[i]->sector;
    coord3d tf1=sc->getzf(x1,y1);
    coord3d tf2=sc->getzf(x2,y2);
    coord3d tc1=sc->getzc(x1,y1);
    coord3d tc2=sc->getzc(x2,y2);
    if (tf1<sf1) tf1=sf1;
    if (tf1>sc1) tf1=sc1;
    if (tf2<sf2) tf2=sf2;
    if (tf2>sc2) tf2=sc2;
    if (tc1<sf1) tc1=sf1;
    if (tc1>sc1) tc1=sc1;
    if (tc2<sf2) tc2=sf2;
    if (tc2>sc2) tc2=sc2;
    wh[i]->z1c=tc1;
    wh[i]->z2c=tc2;
    if (i>0) {
      if (wi>=wn) {
        ww[wi]=new Twall();
        ww[wi]->options=waSOLID;
        ww[wi]->texture=defwalltxt;
        wallsnum++;
        wn++;
      }
      ww[wi]->z1c=tf1;
      ww[wi]->z2c=tf2;
      wi++;
    }
  }

  // updates the last wall. adds a wall at the top if necessary
  if (hn) {
    coord3d z1=wh[hn-1]->sector->getzc(x1,y1);
    coord3d z2=wh[hn-1]->sector->getzc(x2,y2);
    if (fl2 || z1<s->getzc(x1,y1) || z2<s->getzc(x2,y2)) {
      if (wi>=wn) {
        ww[wi]=new Twall();
        ww[wi]->options=waSOLID;
        ww[wi]->texture=defwalltxt;
        wn++;
        wallsnum++;
      }
      wi++;
      fl2=true;
    }
  }
  else wi++;
  if (fl2) {
    ww[wi-1]->z1c=s->getzc(x1,y1);
    ww[wi-1]->z2c=s->getzc(x2,y2);
  }

  // removes the unnecessary walls
  for (;wn>wi;wn--) {
    delete ww[wn-1];
    wallsnum--;
  }

  // links the walls and the holes in a list again
  Twall **w0=walls;
  wi=0;
  if (fl1) {
    *w0=ww[0];
    w0=&(*w0)->next;
    wi++;
  }
  for (hi=0;hi<hn;hi++,wi++) {
    *w0=wh[hi];
    w0=&(*w0)->next;
    if (wi<wn) {
      *w0=ww[wi];
      w0=&(*w0)->next;
    }
  }
  for (;wi<wn;wi++) {
    *w0=ww[wi];
    w0=&(*w0)->next;
  }
  *w0=NULL;
#else
  // in case of array, just updates the heights
  Twall **w;
  int wi;
  coord3d x1=verts[v1].x;
  coord3d y1=verts[v1].y;
  coord3d x2=verts[v2].x;
  coord3d y2=verts[v2].y;
  coord3d sf1=s->getzf(x1,y1);
  coord3d sf2=s->getzf(x2,y2);
  coord3d sc1=s->getzc(x1,y1);
  coord3d sc2=s->getzc(x2,y2);
  int fl=0;
  coord3d h1[MAX_WPL+1];
  coord3d h2[MAX_WPL+1];
  int i=0;
  for (wi=0,w=walls;wi<wallsnum;wi++,NEXTWALL(w)) {
    if (is_hole(*w)) {
    Tsector *sc=((Thole *)(*w))->sector;
      coord3d tf1=sc->getzf(x1,y1);
      coord3d tf2=sc->getzf(x2,y2);
      coord3d tc1=sc->getzc(x1,y1);
      coord3d tc2=sc->getzc(x2,y2);
      if (tf1<sf1) tf1=sf1;
      if (tf1>sc1) tf1=sc1;
      if (tf2<sf2) tf2=sf2;
      if (tf2>sc2) tf2=sc2;
      if (tc1<sf1) tc1=sf1;
      if (tc1>sc1) tc1=sc1;
      if (tc2<sf2) tc2=sf2;
      if (tc2>sc2) tc2=sc2;
      h1[i]=tf1;
      h2[i]=tf2;
      i++;
      h1[i]=tc1;
      h2[i]=tc2;
      i++;
    }
  }
  h1[i]=sc1;
  h2[i]=sc2;
  if (i==0 || h1[i-1]<h1[i] || h2[i-1]<h2[i]) i++;

  for (wi=0,w=walls;wi<wallsnum;wi++,NEXTWALL(w)) {
    (*w)->z1c=h1[wi+fl];
    (*w)->z2c=h2[wi+fl];
  }
#endif
  // calls changeheight for opposite line
  {
    int wi;
    Twall **w;
    if (wave)
      for (wi=0,w=walls;wi<wallsnum;wi++,NEXTWALL(w)) {
        if (is_hole(*w)) {
          Tsector *sc=((Thole *)(*w))->sector;
          Tline *l=sc->getline(v2,v1);
          if (l) l->changeheight(sc,false);
        }
      }
  }
}
