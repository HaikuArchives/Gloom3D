//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tsector.cpp - class Tsector implementation                         //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _TSECTOR_CPP
#include "boom.h"
#include "draw.h"
#include "geom.h"
#include "tclip.h"
#include "read.h"
#include "write.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>


Tsector::Tsector():
  Tmap_item()
{
  lines = NULL;
}

// saves the sector to the current file
void Tsector::save(Tmap *m,Tcluster *c)
{
  Tmap_item::save();

  // writes floor and ceiling info
  wrfloat( zfa );
  wrfloat( zfb );
  wrfloat( zfc );
  wrfloat( zca );
  wrfloat( zcb );
  wrfloat( zcc );
  wrlong( tfloor );
  wrlong( tceiling );
  wrlong( linesnum );

  // writes all the lines
  int i;
  Tline *l;
  for (i=0,l=lines;i<linesnum;NEXTLINE(l),i++)
    l->save(m,c,this);
}

// loads the sector from the current file
bool Tsector::load()
{
  Tmap_item::load();

  // reads floor and ceiling info
  zfa=rdfloat();
  zfb=rdfloat();
  zfc=rdfloat();
  zca=rdfloat();
  zcb=rdfloat();
  zcc=rdfloat();
  tfloor=rdlong();
  tceiling=rdlong();

  // reads all the lines
  int n=rdlong();
  if (n==0) return false;
  CONSTRUCT_AND_LOAD_ARRAY(lines,Tline,n);
  linesnum=n;
  return true;
}

// initializes the sector after the loading
void Tsector::postload(Tmap *m,Tcluster *c)
{
  // calculates the bounding box
  Tline *cn, *l;
  int i;
  minx=1000000;
  maxx=-1000000;
  miny=1000000;
  maxy=-1000000;
  for (i=0,cn=lines,l=lines;i<linesnum;NEXTLINE(l),i++) {
#ifndef EDITOR
    if (l->contour_end()) l->nextc=l-cn, cn=l+1;
#else
    if (l->contour_end()) l->nextc=cn, cn=l->next;
    else l->nextc=l->next;
#endif
    coord3d x1=verts[l->v1].x;
    coord3d y1=verts[l->v1].y;
    if (minx>x1) minx=x1;
    if (miny>y1) miny=y1;
    if (maxx<x1) maxx=x1;
    if (maxy<y1) maxy=y1;

    // calls postload() for all lines
    l->postload(m,c,this);
  }
}

// unloads the sector (releases all resources allocated by load())
void Tsector::unload()
{
  // unloads all the lines
  if (lines) {
    Tline *l=lines;
    for (int i=0;i<linesnum;i++,NEXTLINE(l))
      l->unload();
    DESTRUCT_ARRAY(lines);
  }
}

#define EPS 0.00001 // to fix problems with precision

// creates the contours from the lines of the sector
int Tsector::build_contours(Tcont *c)
{
  Tline *l=lines,*last;
  int i=0;
  coord3d x1,z1,x2,z2;
  coord2d sx1,sx2;
  Tcont *c1=c;
  int cn=0;

  // checks each line in the sector
  do {
    rotatez(verts[l->v1].x,verts[l->v1].y,&x2,&z2);
    if (z2>0) sx2=projectx(x2,z2);
    c--;
   loop2:

    // tries to find a visible line
    while (sx1=sx2,x1=x2,z1=z2,i<linesnum) {
      rotatez(verts[l->v2].x,verts[l->v2].y,&x2,&z2);
      if (z2>0) { // if the second end of the line is infront of the screen
        sx2=projectx(x2,z2);
        if (z1>0) { // if the first end of the line is infront of the screen
          if (sx1<=MAX_LX && sx2>=MIN_LX && sx1<sx2) {
            //if the projection of the line occupies the whole screen, creates a new contour
            c++;
            cn++;
            c->s=this;
            c->l1=l;
            c->x[0]=sx2;
            c->n=1;
            c->x1=sx1;
            c->x2=sx2;
            if (z1<z2) c->minz=z1,c->maxz=z2;
                  else c->minz=z2,c->maxz=z1;
            last=l;
            break;
          }
        }
        else if (sx2>=MIN_LX && x1*z2<=x2*z1+EPS) { // z1<=0 z2>0
          // if the first end is behind the screen and the second end is infront of the screen
          // start new contour
          c++;
          cn++;
          c->s=this;
          c->l1=l;
          c->x[0]=sx2;
          c->n=1;
          c->x1=MIN_LX;
          c->x2=sx2;
          c->minz=0;
          c->maxz=z2;
          last=l;
          break;
        }
      }
      else if (z1>0 && sx1<=MAX_LX && x1*z2<=x2*z1+EPS) { // z1>0 z2<=0
        // if the first end is infront of the screen and the second end is behind the screen
        // start new contour
        c++;
        cn++;
        c->s=this;
        c->l1=l;
        c->x[0]=MAX_LX;
        c->n=1;
        c->x1=sx1;
        c->x2=MAX_LX;
        c->minz=0;
        c->maxz=z1;
        last=l;
      }
      if (l->contour_end()) goto cont_end;
      NEXTLINE(l),i++;
    }

    if (l->contour_end()) goto cont_end;
    if (i+1>=linesnum) goto qqq;

    do {  // while the current line is visible
      NEXTLINE(l),i++;
      sx1=sx2,x1=x2,z1=z2;
      rotatez(verts[l->v2].x,verts[l->v2].y,&x2,&z2);
      if (z2>0) {
        sx2=projectx(x2,z2);
        if (z1>0) {
          if (sx1<=MAX_LX && sx1<sx2) { // z1>0 z2>0
            // if both ends are infront of the screen, add the line to the contour
            c->x[c->n]=sx2;
            c->n++;
            c->x2=sx2;
            if (z1<z2) {
              if (c->minz>z1) c->minz=z1;
              if (c->maxz<z2) c->maxz=z2;
            }
            else {
              if (c->minz>z2) c->minz=z2;
              if (c->maxz<z1) c->maxz=z1;
            }
            last=l;
            goto q1;
          }
        }
      }
      else if (z1>0 && x1*z2<=x2*z1+EPS) { // z1>0 z2<=0
        // if the first end is behind the screen and the second end is infront of the screen
        // add the line to the contour
        c->x[c->n]=MAX_LX;
        c->n++;
        c->x2=MAX_LX;
        if (c->minz>0) c->minz=0;
        if (c->maxz<z1) c->maxz=z1;
        last=l;
      }
      if (!l->contour_end()) {
        NEXTLINE(l),i++;
        goto loop2;
      }
    q1:;
    } while (!l->contour_end());
  cont_end:
    // tries to combine the current contour with the previous
    if (c1<c && last->next_in_contour()==c1->l1 && c->x2==c1->x1) {
      int i;
      for (i=c1->n-1;i>=0;i--)
        c1->x[i+c->n]=c1->x[i];
      for (i=0;i<c->n;i++)
        c1->x[i]=c->x[i];
      c1->l1=c->l1;
      c1->n+=c->n;
      c1->x1=c->x1;
      if (c1->minz>c->minz) c1->minz=c->minz;
      if (c1->maxz<c->maxz) c1->maxz=c->maxz;
      c1=c;
      cn--;
    }
    else c1=++c;
    NEXTLINE(l),i++;
  } while (i<linesnum);
 qqq:
  return cn;
}

// fills polygon fp with the texture of the floor
void Tsector::draw_floor(Tmonotone *mp)
{
  // sets the data for the texture mapping
  Tvector a={-view.x*cosz-view.y*sinz,view.x*sinz-view.y*cosz,-zfc+view.z};
  Tvector p={512*cosz,-512*sinz,-512*zfa};
  Tvector q={-512*sinz,-512*cosz,512*zfb};
  rotateyxv(&a);
  rotateyxv(&p);
  rotateyxv(&q);
  setdrawdata(&a,&p,&q);
  setdrawtexture(tfloor);

  // draws the polygon
  drawmp(mp);
}

// fills polygon fp with the texture of the ceiling
void Tsector::draw_ceiling(Tmonotone *mp)
{
  // sets the data for the texture mapping
  Tvector a={-view.x*cosz-view.y*sinz,view.x*sinz-view.y*cosz,-zcc+view.z};
  Tvector p={512*cosz,-512*sinz,-512*zca};
  Tvector q={512*sinz,512*cosz,-512*zcb};
  rotateyxv(&a);
  rotateyxv(&p);
  rotateyxv(&q);
  setdrawdata(&a,&p,&q);
  setdrawtexture(tceiling);

  // draws the polygon
  drawmp(mp);
}

// returns the height of the floor at point (x,y)
coord3d Tsector::getzf(coord3d x,coord3d y)
{
  return zfa*x+zfb*y+zfc;
}

// returns the height of the ceiling at point (x,y)
coord3d Tsector::getzc(coord3d x,coord3d y)
{
  return zca*x+zcb*y+zcc;
}

// must be called when the height of the floor or ceiling is changed
void Tsector::changeheight( void )
{
  Tline *l;
  int li;
  for (l=lines,li=0;li<linesnum;li++,NEXTLINE(l))
    l->changeheight(this,true);
}

// returns the line with ends v1 and v2
Tline *Tsector::getline(int v1,int v2)
{
  Tline *l;
  int li;
  for (l=lines,li=0;li<linesnum;li++,NEXTLINE(l))
    if (l->v1==v1 && l->v2==v2) return l;
  return NULL;
}
