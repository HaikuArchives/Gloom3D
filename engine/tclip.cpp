//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tclip.cpp - class Tclip implementation                             //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _CLIP_CPP
#include "tclip.h"

#include <stdio.h>

#define OPTIMIZE

#undef splitab

// splits 1D segment (a1,a2) with ratio b1:b2
coord2d splitab(coord2d a1,coord2d a2,coord2d b1,coord2d b2)
{
  return (a1*b2+a2*b1)/(b1+b2);
}

// finds the intersection between (ax1,ay1)-(ax2,ay2) and (bx1,by1)-(bx2,by2)
inline void split(coord2d ax1,coord2d ay1,coord2d ax2,coord2d ay2,
           coord2d bx1,coord2d by1,coord2d bx2,coord2d by2,
           coord2d *x,coord2d *y)
{
  double a1 = ay2 - ay1;
  double b1 = ax2 - ax1;
  double c1 = ay2*ax1 - ax2*ay1;
  double a2 = by2 - by1;
  double b2 = bx2 - bx1;
  double c2 = by2*bx1 - bx2*by1;
  double q = a1*b2 - a2*b1;
  *x = (b2*c1-b1*c2)/q;
  *y = (a2*c1-a1*c2)/q;
}

// adds a trapezoid to a list
inline void addtrap(Ttrap ***r,coord2d x2,coord2d y11,
           coord2d y12,coord2d y21,coord2d y22,int un,int dn)
{
  **r=new Ttrap(x2,y11,y12,y21,y22,un,dn,NULL);
  *r=&((**r)->next);
}

// cuts the clip into 3 parts: x<x1c, x1c<x<x2c, x2c<x
// stores the first part in beg and the third part in end
void Tclip::cut(coord2d x1c,coord2d x2c)
{
  coord2d x=xmin;

  if (xmin<x1c) {
    Ttrap **t;
    for (t=&traps;(*t)->x2<=x1c;t=&((*t)->next))
      x=(*t)->x2;
    beg=traps;
    Ttrap *tr=*t;
    if (x<x1c) {
      coord2d yu,yd;
      traps=new Ttrap(tr->x2,yu=splitab(tr->y11,tr->y21,x1c-x,tr->x2-x1c),
                yd=splitab(tr->y12,tr->y22,x1c-x,tr->x2-x1c),tr->y21,tr->y22,
                tr->un,tr->dn,tr->next);
      tr->y21=yu;
      tr->y22=yd;
      tr->x2=x1c;
      lbeg=&tr->next;
    }
    else {
      traps=tr;
      lbeg=t;
    }
  }
  else beg=NULL;

  if (x2c<xmax) {
    x=x1c;
    Ttrap *t;
    for (t=traps;t->x2<x2c;t=t->next)
      x=t->x2;
    last=t;
    if (x2c<t->x2) {
      coord2d yu,yd;
      end=new Ttrap(t->x2,yu=splitab(t->y11,t->y21,x2c-x,t->x2-x2c),
              yd=splitab(t->y12,t->y22,x2c-x,t->x2-x2c),t->y21,t->y22,
              t->un,t->dn,t->next);
      t->y21=yu;
      t->y22=yd;
      t->x2=x2c;
    }
    else {
      end=t->next;
    }
    t->x2=x2c;
    t->next=NULL;
  }
  else end=NULL;
}

// merges the adjacent trapezoids with common segments
void optimizetraps(Ttrap *t)
{
#ifdef OPTIMIZE
  Ttrap *n;
  for (;t->next;t=n) {
   qqq:
    n=t->next;
    if (n==NULL) break;
    if (t->un==n->un && t->dn==n->dn) {
      t->x2=n->x2;
      t->y21=n->y21;
      t->y22=n->y22;
      t->next=n->next;
      delete n;
      goto qqq;
    }
  }
#endif
}

// restore the clip after cut
void Tclip::restore(Ttrap *t)
{
  deltraps(traps);
  if (beg) {
    *lbeg=t;
    traps=beg;
  }
  else traps=t;
  if (end) {
    last->next=end;
  }
  optimizetraps(traps);
}

// adds beg part of the clip to poly
void Tclip::addbeg(Tmonotone *poly)
{
  if (beg) {
    *lbeg=poly->traps;
    poly->traps=beg;
    coord2d x=poly->xmin;
    poly->xmin=xmin;
    xmin=x;
    if (poly->ymin>ymin) poly->ymin=ymin;
    if (poly->ymax<ymax) poly->ymax=ymax;
    beg=NULL;
  }
  optimizetraps(poly->traps);
}

// adds end part of the clip to poly
void Tclip::addend(Tmonotone *poly)
{
  if (end) {
    Ttrap *t;
    for (t=poly->traps;t->next;t=t->next);
    t->next=end;
    coord2d x=poly->xmax;
    poly->xmax=xmax;
    xmax=x;
    if (poly->ymin>ymin) poly->ymin=ymin;
    if (poly->ymax<ymax) poly->ymax=ymax;
    end=NULL;
  }
  optimizetraps(poly->traps);
}

// clips a trapezoid with the clip. returns the result in poly
void Tclip::clip(coord2d x1,coord2d x2,coord2d y11,coord2d y12,coord2d y21,coord2d y22,Tmonotone *poly)
{
  poly->xmin=x1;
  poly->xmax=x2;
  poly->traps=NULL;
  Ttrap **r=&(poly->traps);
  int un=csegment++;
  int dn=csegment++;
  coord2d ymin=10000;
  coord2d ymax=-10000;
  coord2d cx1=x1;
  coord2d yu1=y11;
  coord2d yd1=y12;

  for (Ttrap *t=traps;t;t=t->next) {
    Ttrap **r0=r;
    coord2d cx2=t->x2;
    coord2d cy1=t->y11;
    coord2d cy2=t->y12;
    coord2d yu2=splitab(y11,y21,cx2-x1,x2-cx2);
    coord2d yd2=splitab(y12,y22,cx2-x1,x2-cx2);
    bool above1=yu1>t->y11;
    bool above2=yu2>t->y21;
    if (yu1==t->y11) above1=above2;
    if (yu2==t->y21) above2=above1;
    bool below1=yd1<t->y12;
    bool below2=yd2<t->y22;
    if (yd1==t->y12) below1=below2;
    if (yd2==t->y22) below2=below1;

    coord2d xu,yu,xd,yd;
    if (above1!=above2) {
      split(x1,y11,x2,y21,cx1,cy1,cx2,t->y21,&xu,&yu);
    }
    if (below1!=below2) {
      split(x1,y12,x2,y22,cx1,cy2,cx2,t->y22,&xd,&yd);
    }
    if (!above1) {
      if (!above2) {
        if (!below1) {
          if (!below2) {
            addtrap(&r,cx2,yu1,yd1,yu2,yd2,un,dn);
          }
          else { //!above1 && !above2 && !below1 && below2
            coord2d yy=splitab(y11,y21,xd-x1,x2-xd);
            addtrap(&r,xd,yu1,yd1,yy,yd,un,dn);
            addtrap(&r,cx2,yy,yd,yu2,t->y22,un,t->dn);
          }
        }
        else { //!above1 && !above2 && below1
          if (!below2) {
            coord2d yy=splitab(y11,y21,xd-x1,x2-xd);
            addtrap(&r,xd,yu1,cy2,yy,yd,un,t->dn);
            addtrap(&r,cx2,yy,yd,yu2,yd2,un,dn);
          }
          else { //!above1 && !above2 && below1 && below2
            addtrap(&r,cx2,yu1,cy2,yu2,t->y22,un,t->dn);
          }
        }
      }
      else { //!above1 && above2
        if (!below1) {
          if (!below2) {
            coord2d yy=splitab(y12,y22,xu-x1,x2-xu);
            addtrap(&r,xu,yu1,yd1,yu,yy,un,dn);
            addtrap(&r,cx2,yu,yy,t->y21,yd2,t->un,dn);
          }
          else { //!above1 && above2 && !below1 && below2
            if (xu<xd) {
              coord2d yy1=splitab(y12,y22,xu-x1,x2-xu);
              coord2d yy2=splitab(cy1,t->y21,xd-cx1,cx2-xd);
              addtrap(&r,xu,yu1,yd1,yu,yy1,un,dn);
              addtrap(&r,xd,yu,yy1,yy2,yd,t->un,dn);
              addtrap(&r,cx2,yy2,yd,t->y21,t->y22,t->un,t->dn);
            }
            else if (xd==xu) {
              addtrap(&r,xd,yu1,yd1,yu,yd,un,dn);
              addtrap(&r,cx2,yu,yd,t->y21,t->y22,t->un,t->dn);
            }
            else {
              coord2d yy1=splitab(y11,y21,xd-x1,x2-xd);
              coord2d yy2=splitab(cy2,t->y22,xu-cx1,cx2-xu);
              addtrap(&r,xd,yu1,yd1,yy1,yd,un,dn);
              addtrap(&r,xu,yy1,yd,yu,yy2,un,t->dn);
              addtrap(&r,cx2,yu,yy2,t->y21,t->y22,t->un,t->dn);
            }
          }
        }
        else { //!above1 && above2 && below1
          if (!below2) {
            if (xu<xd) {
              coord2d yy1=splitab(cy2,t->y22,xu-cx1,cx2-xu);
              coord2d yy2=splitab(cy1,t->y21,xd-cx1,cx2-xd);
              addtrap(&r,xu,yu1,cy2,yu,yy1,un,t->dn);
              addtrap(&r,xd,yu,yy1,yy2,yd,t->un,t->dn);
              addtrap(&r,cx2,yy2,yd,t->y21,yd2,t->un,dn);
            }
            else if (xd==xu) {
              addtrap(&r,xd,yu1,cy2,yu,yd,un,t->dn);
              addtrap(&r,cx2,yu,yd,t->y21,yd2,t->un,dn);
            }
            else {
              coord2d yy1=splitab(y11,y21,xd-x1,x2-xd);
              coord2d yy2=splitab(y12,y22,xu-x1,x2-xu);
              addtrap(&r,xd,yu1,cy2,yy1,yd,un,t->dn);
              addtrap(&r,xu,yy1,yd,yu,yy2,un,dn);
              addtrap(&r,cx2,yu,yy2,t->y21,yd2,t->un,dn);
            }
          }
          else { //!above1 && above2 && below1 && below2
            coord2d yy=splitab(cy2,t->y22,xu-cx1,cx2-xu);
            addtrap(&r,xu,yu1,cy2,yu,yy,un,t->dn);
            addtrap(&r,cx2,yu,yy,t->y21,t->y22,t->un,t->dn);
          }
        }
      }
    }
    else { //above1
      if (!above2) {
        if (!below1) {
          if (!below2) {
            coord2d yy=splitab(y12,y22,xu-x1,x2-xu);
            addtrap(&r,xu,cy1,yd1,yu,yy,t->un,dn);
            addtrap(&r,cx2,yu,yy,yu2,yd2,un,dn);
          }
          else { //above1 && !above2 && !below1 && below2
            if (xu<xd) {
              coord2d yy1=splitab(y12,y22,xu-x1,x2-xu);
              coord2d yy2=splitab(y11,y21,xd-x1,x2-xd);
              addtrap(&r,xu,cy1,yd1,yu,yy1,t->un,dn);
              addtrap(&r,xd,yu,yy1,yy2,yd,un,dn);
              addtrap(&r,cx2,yy2,yd,yu2,t->y22,un,t->dn);
            }
            else if (xd==xu) {
              addtrap(&r,xd,cy1,yd1,yu,yd,t->un,dn);
              addtrap(&r,cx2,yu,yd,yu2,t->y22,un,t->dn);
            }
            else {
              coord2d yy1=splitab(cy1,t->y21,xd-cx1,cx2-xd);
              coord2d yy2=splitab(cy2,t->y22,xu-cx1,cx2-xu);
              addtrap(&r,xd,cy1,yd1,yy1,yd,t->un,dn);
              addtrap(&r,xu,yy1,yd,yu,yy2,t->un,t->dn);
              addtrap(&r,cx2,yu,yy2,yu2,t->y22,un,t->dn);
            }
          }
        }
        else { //above1 && !above2 && below1
          if (!below2) {
            if (xu<xd) {
              coord2d yy1=splitab(cy2,t->y22,xu-cx1,cx2-xu);
              coord2d yy2=splitab(y11,y21,xd-x1,x2-xd);
              addtrap(&r,xu,cy1,cy2,yu,yy1,t->un,t->dn);
              addtrap(&r,xd,yu,yy1,yy2,yd,un,t->dn);
              addtrap(&r,cx2,yy2,yd,yu2,yd2,un,dn);
            }
            else if (xd==xu) {
              addtrap(&r,xd,cy1,cy2,yu,yd,t->un,t->dn);
              addtrap(&r,cx2,yu,yd,yu2,yd2,un,dn);
            }
            else {
              coord2d yy1=splitab(cy1,t->y21,xd-cx1,cx2-xd);
              coord2d yy2=splitab(y12,y22,xu-x1,x2-xu);
              addtrap(&r,xd,cy1,cy2,yy1,yd,t->un,t->dn);
              addtrap(&r,xu,yy1,yd,yu,yy2,t->un,dn);
              addtrap(&r,cx2,yu,yy2,yu2,yd2,un,dn);
            }
          }
          else { //above1 && !above2 && below1 && below2
            coord2d yy=splitab(cy2,t->y22,xu-cx1,cx2-xu);
            addtrap(&r,xu,cy1,cy2,yu,yy,t->un,t->dn);
            addtrap(&r,cx2,yu,yy,yu2,t->y22,un,t->dn);
          }
        }
      }
      else { //above1 && above2
        if (!below1) {
          if (!below2) {
            addtrap(&r,cx2,cy1,yd1,t->y21,yd2,t->un,dn);
          }
          else { //above1 && above2 && !below1 && below2
            coord2d yy=splitab(cy1,t->y21,xd-cx1,cx2-xd);
            addtrap(&r,xd,cy1,yd1,yy,yd,t->un,dn);
            addtrap(&r,cx2,yy,yd,t->y21,t->y22,t->un,t->dn);
          }
        }
        else { //above1 && above2 && below1
          if (!below2) {
            coord2d yy=splitab(cy1,t->y21,xd-cx1,cx2-xd);
            addtrap(&r,xd,cy1,cy2,yy,yd,t->un,t->dn);
            addtrap(&r,cx2,yy,yd,t->y21,yd2,t->un,dn);
          }
          else { //above1 && above2 && below1 && below2
            addtrap(&r,cx2,cy1,cy2,t->y21,t->y22,t->un,t->dn);
          }
        }
      }
    }
    while (r0!=r) {
      if (ymax<(*r0)->y11) ymax=(*r0)->y11;
      if (ymax<(*r0)->y21) ymax=(*r0)->y21;
      if (ymin>(*r0)->y12) ymin=(*r0)->y12;
      if (ymin>(*r0)->y22) ymin=(*r0)->y22;
      r0=&((*r0)->next);
    }
    cx1=cx2;
    yu1=yu2;
    yd1=yd2;
  }
  poly->ymin=ymin;
  poly->ymax=ymax;
  optimizetraps(poly->traps);
}

// deletes a linked list of Ttraps
void deltraps(Ttrap *t)
{
  Ttrap *n;
  for (;t;t=n) {
    n=t->next;
    delete t;
  }
}

//Set the current clip. All stuff that are drawn on the screen are first
//clipped with the current clip.
void set_cur_clip( Tclip *clip )
{
  cur_clip = clip;
}
