//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   draw.cpp - low level rendering implementation                      //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _DRAW_CPP
#include "draw.h"

#include "geom.h"

#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// vectors for the perspective texture mapping
static Tvector PERSP_A,PERSP_B,PERSP_C;

// curent texture
static Ttexture *curtxt;

#define EPS 0.0001
// set to 1 to use vertical rasterization for vertical polygons
#define USE_DRAWPOLYVER 0

#ifndef DIRECT_3D
#define SOFTWARE_RENDER
#endif

#ifdef SOFTWARE_RENDER
// software polyfgon rendering
static byte **ytbl=NULL;  // table with pointers to every line in the buffer
static int scr_len;       // length of one line
byte *buffer;             // buffer for the rendering

// function for polygon rendering
static void (*drawpoly)(coord2d *vx,coord2d *vy,int n);
// function for line rendering
static void (*drawline)(int x1,int x2,int y);

#define PIXELSPERDIV 8
//#define INTERPOLATE

// draws a horizontal line with variable step in the texture (non horizontal polygons)
static void drawlinetilt(int x1,int x2,int y)
{
  byte *b=ytbl[y]+x1;
  y-=scr_oy;
  int x=x1-scr_ox;
  dword wd=curtxt->wd;
  dword he=curtxt->he;
  register double A=wd*(x*PERSP_A.x+y*PERSP_A.y+PERSP_A.z);
  register double B=he*(x*PERSP_B.x+y*PERSP_B.y+PERSP_B.z);
  register double C=x*PERSP_C.x+y*PERSP_C.y+PERSP_C.z;
  byte *txt=curtxt->datap;
  x2-=x1;

#ifdef INTERPOLATE
  // fast variant (makes one division per PIXELSPERDIV pixels and interpolates linearly between them)
  // for increased performance the division is made in paralel with the rendering
  double DC=PIXELSPERDIV*PERSP_C.x;
  double A0=wd*PERSP_A.x*C-A*PERSP_C.x;
  double B0=he*PERSP_B.x*C-B*PERSP_C.x;
  double cc=65536;
  dword k=cc*A/C;
  dword l=cc*B/C;
  double C1=C;
  dword dk,dl1; 

  asm (
	"fld     %0\n\t"
    "fld     %1\n\t"
    "fld     %2\n\t"
    "fld     %3\n\t"
    "fld     %4\n\t"
    "fld     %%st\n\t"
    "fadd    %%st(2), %%st\n\t"
    "fld     %%st\n\t"
    "fmul    %%st(2), %%st\n\t"
    "fdivr   %%st(6), %%st\n\t"
    :
    :"m"(cc), "m"(A0), "m"(B0), "m"(DC), "m"(C)
    :"memory"
  );
  for (x1=0;x1<x2;x1++,b++) {
    if ((x1&(PIXELSPERDIV-1))==0) {
//      C=C1;C1+=DC;
//      dk=A0*65536/(C*C1);
//      dl=B0*65536/(C*C1);
      asm (
        "fld     %%st\n\t"
        "fmul    %%st(6), %%st\n\t"
        "fistp   %0\n\t"	//dword ptr dk"
        "fmul    %%st(4), %%st\n\t"
        "fistp   %1\n\t"	//dword ptr dl1"
        "fst     %%st(1)\n\t"
        "fadd    %%st(2), %%st\n\t"
        "fld     %%st\n\t"
        "fmul    %%st(2), %%st\n\t"
        "fdivr   %%st(6), %%st\n\t"
        :
        :"m"(dk), "m"(dl1)
        :"memory"
      );
    }
    dword u=(k>>16)&(wd-1);
    dword v=(l>>16)&(he-1);
    *b=txt[v*wd+u];
    k+=dk;
    l+=dl1;
  }
  asm (
    "fstp    %%st\n\t"
    "fstp    %%st\n\t"
    "fstp    %%st\n\t"
    "fstp    %%st\n\t"
    "fstp    %%st\n\t"
    "fstp    %%st\n\t"
    "fstp    %%st\n\t"
    :
    :
    :"memory"
  );
#else
  // slow variant (calculates every pixel individually)
  double DA=wd*PERSP_A.x;
  double DB=he*PERSP_B.x;
  double DC=PERSP_C.x;
  for (x1=0;x1<x2;x1++,b++) {
    register dword u=((dword)(A/C))&(wd-1);
    register dword v=((dword)(B/C))&(he-1);
    *b=txt[v*wd+u];
    A+=DA;
    B+=DB;
    C+=DC;
  }
#endif
}

// draws a horizontal line with constant step in the texture (horizontal polygons)
static void drawlinehor(int x1,int x2,int y)
{
  byte *b=ytbl[y]+x1;
  y-=scr_oy;
  int x=x1-scr_ox;
  dword wd=curtxt->wd;
  dword he=curtxt->he;
  double A=wd*(x*PERSP_A.x+y*PERSP_A.y+PERSP_A.z);
  double B=he*(x*PERSP_B.x+y*PERSP_B.y+PERSP_B.z);
  double C=y*PERSP_C.y+PERSP_C.z;
  byte *txt=curtxt->datap;
  x2-=x1;
  double q=65536/C;

  dword k=A*q;
  dword l=B*q;
  dword dk,dl;
  dk=wd*PERSP_A.x*q;
  dl=he*PERSP_B.x*q;

  for (x1=0;x1<x2;x1++,b++) {
    dword u=(k>>16)&(wd-1);
    dword v=(l>>16)&(he-1);
    *b=txt[v*wd+u];
    k+=dk;
    l+=dl;
  }
}

// fills a polygon with horizontal rows if the polygon is not vertical
static void drawpolyhor(coord2d *vx,coord2d *vy,int n)
{
  int l,r,l1,r1;
  int i;
  double x1,x2,dx1,dx2;
  int y,ymax,xmin,xmax;

  double yd=vy[0];l=0;
  double ymaxd=vy[0];
  xmin=vx[0];
  xmax=vx[0];
  for (i=1;i<n;i++) {
    if (yd>vy[i]) yd=vy[i],l=i;
    if (ymaxd<vy[i]) ymaxd=vy[i];
    if (xmax<(int)vx[i]) xmax=vx[i];
    if (xmin>(int)vx[i]) xmin=vx[i];
  }
  if (xmax<0 || ymaxd<0 || xmin>=scr_dx || yd>=scr_dy) return;
  ymax=ymaxd;
  r=l;
  int p0=l;
  y=ceil(yd-EPS);

  l1=(l+1)%n;
  while (ceil(vy[l1]-EPS)<=y && vy[l]<ymaxd && l1!=p0)
    l=l1,l1=(l+1)%n;

  r1=(r+n-1)%n;
  while (ceil(vy[r1]-EPS)<=y && vy[r]<ymaxd && r1!=p0)
    r=r1,r1=(r+n-1)%n;

  if (vy[l1]==vy[l]) dx1=0;
  else dx1=(vx[l1]-vx[l])/(vy[l1]-vy[l]);
  x1=vx[l]+dx1*(y-vy[l]);
  if (vy[r1]==vy[r]) dx2=0;
  else dx2=(vx[r1]-vx[r])/(vy[r1]-vy[r]);
  x2=vx[r]+dx2*(y-vy[r]);

  if (ymax>=scr_dy) ymax=scr_dy-1;
  for (;y<=ymax;y++) {
    if (y>vy[l1]) {
      while (y>vy[l1] && l1!=p0)
        l=l1,l1=(l+1)%n;
      if (l1==p0) return;
      dx1=(vx[l1]-vx[l])/(vy[l1]-vy[l]);
      x1=vx[l]+dx1*(y-vy[l]);
    }
    if (y>vy[r1]) {
      while (y>vy[r1] && r1!=p0)
        r=r1,r1=(r+n-1)%n;
      if (r1==p0) return;
      dx2=(vx[r1]-vx[r])/(vy[r1]-vy[r]);
      x2=vx[r]+dx2*(y-vy[r]);
    }
    drawline(ceil(x1-EPS),floor(x2+EPS)+1,y);
    x1+=dx1;
    x2+=dx2;
  }
}

#if USE_DRAWPOLYVER
// draws a single vertical column
static void drawlinever(int y1,int y2,int x)
{
  byte *b=ytbl[y1]+x;
  x-=scr_ox;
  int y=y1-scr_oy;
  dword wd=curtxt->wd;
  dword he=curtxt->he;
  double A=wd*(x*PERSP_A.x+y*PERSP_A.y+PERSP_A.z);
  double B=he*(x*PERSP_B.x+y*PERSP_B.y+PERSP_B.z);
  double C=x*PERSP_C.x+PERSP_C.z;
  byte *txt=curtxt->datap;
  y2-=y1;
  double q=65536/C;

  dword k=A*q;
  dword l=B*q;
  dword dl=he*PERSP_B.y*q;

  dword u=(k>>16)&(wd-1);
  byte *t=txt+u;
  for (y1=0;y1<y2;y1++,b+=scr_len) {
    dword v=(l>>16)&(he-1);
    *b=t[v*wd];
    l+=dl;
  }
}

// fills a polygon with vertical columns if the polygon is vertical (wall)
static void drawpolyver(coord2d *vx,coord2d *vy,int n)
{
  int u,d,u1,d1;
  int i;
  double y1,y2,dy1,dy2;
  int x,xmax,ymin,ymax;

  double xd=vx[0];d=0;
  double xmaxd=vx[0];
  ymin=vy[0];
  ymax=vy[0];
  for (i=1;i<n;i++) {
    if (xd>vx[i]) xd=vx[i],d=i;
    if (xmaxd<vx[i]) xmaxd=vx[i];
    if (ymax<(int)vy[i]) ymax=vy[i];
    if (ymin>(int)vy[i]) ymin=vy[i];
  }
  if (xmaxd<0 || ymax<0 || ymin>=scr_dy || xd>=scr_dx) return;
  xmax=xmaxd;
  u=d;
  int p0=d;
  x=ceil(xd-EPS);

  d1=(d+n-1)%n;
  while (ceil(vx[d1]-EPS)<=x && vx[d]<xmaxd && d1!=p0)
    d=d1,d1=(d+n-1)%n;

  u1=(u+1)%n;
  while (ceil(vx[u1]-EPS)<=x && vx[u]<xmaxd && u1!=p0)
    u=u1,u1=(u+1)%n;

  if (vx[d]==vx[d1]) dy1=0;
  else dy1=(vy[d1]-vy[d])/(vx[d1]-vx[d]);
  y1=vy[d]+dy1*(x-vx[d]);
  if (vx[u]==vx[u1]) dy2=0;
  else dy2=(vy[u1]-vy[u])/(vx[u1]-vx[u]);
  y2=vy[u]+dy2*(x-vx[u]);

  if (xmax>=scr_dx) xmax=scr_dx-1;
  for (;x<=xmax;x++) {
    if (x>vx[d1]) {
      while (x>vx[d1] && d1!=p0)
        d=d1,d1=(d+n-1)%n;
      if (d1==p0) return;
      dy1=(vy[d1]-vy[d])/(vx[d1]-vx[d]);
      y1=vy[d]+dy1*(x-vx[d]);
    }
    if (x>vx[u1]) {
      while (x>vx[u1] && u1!=p0)
        u=u1,u1=(u+1)%n;
      if (u1==p0) return;
      dy2=(vy[u1]-vy[u])/(vx[u1]-vx[u]);
      y2=vy[u]+dy2*(x-vx[u]);
    }
    drawlinever(ceil(y1-EPS),floor(y2+EPS)+1,x);
    y1+=dy1;
    y2+=dy2;
  }
}
#endif

// initializes y table
static void initytbl(int w,int h)
{
  w=(w+3)&(~3);
  scr_len=w;
  if (ytbl) free(ytbl),ytbl=NULL;
  ytbl=(byte **)malloc(h*sizeof(byte*));
  for (int i=0;i<h;i++)
    ytbl[i]=buffer+i*w;
}

// frees the allocated memory
void draw_done( void )
{
  if (ytbl) free(ytbl),ytbl=NULL;
}

// initializes the software renderer
void draw_init(byte *_buffer,int w,int h)
{
  buffer=_buffer;
  initytbl(w,h);
}
#endif

#ifdef DIRECT_3D

static coord3d az,pz,qz;

static D3DTLVERTEX tlvertices[4]={
  {0,0,0,0,RGB_MAKE(255,255,255)},
  {0,0,0,0,RGB_MAKE(255,255,255)},
  {0,0,0,0,RGB_MAKE(255,255,255)},
  {0,0,0,0,RGB_MAKE(255,255,255)}
};

// draws a polygon with Direct 3D functions
void drawpoly(coord2d *vx,coord2d *vy,int n)
{
  for (int i=0;i<n;i++) {
    coord2d x=vx[i];
    coord2d y=vy[i];
    tlvertices[i].sx = x;
    tlvertices[i].sy = y;
    tlvertices[i].tu = (PERSP_A.x*(x-scr_ox)+PERSP_A.y*(y-scr_oy)+PERSP_A.z)/(PERSP_C.x*(x-scr_ox)+PERSP_C.y*(y-scr_oy)+PERSP_C.z);
    tlvertices[i].tv = (PERSP_B.x*(x-scr_ox)+PERSP_B.y*(y-scr_oy)+PERSP_B.z)/(PERSP_C.x*(x-scr_ox)+PERSP_C.y*(y-scr_oy)+PERSP_C.z);
    tlvertices[i].rhw = 1/(az+tlvertices[i].tu*pz+tlvertices[i].tv*qz);
    tlvertices[i].sz = 1-tlvertices[i].rhw;
  }
  d3d_device->DrawPrimitive(D3DPT_TRIANGLEFAN,D3DFVF_TLVERTEX,tlvertices,n,D3DDP_DONOTCLIP|D3DDP_DONOTLIGHT|D3DDP_DONOTUPDATEEXTENTS);
}

#endif

// finds the intersection between (x1,ay1)-(x2,ay2) and (x1,by1)-(x2,by2)
inline void split(coord2d x1,coord2d x2,coord2d ay1,coord2d ay2,
           coord2d by1,coord2d by2,coord2d *x,coord2d *y)
{
  double a1 = ay2 - ay1;
  double b = x2 - x1;
  double c1 = ay2*x1 - x2*ay1;
  double a2 = by2 - by1;
  double c2 = by2*x1 - x2*by1;
  double q = (a1-a2)*b;
  *x = b*(c1-c2)/q;
  *y = (a2*c1-a1*c2)/q;
}

// rotate n points in vx and stores them in vy
void rotateyxpn(coord2d *vx,coord2d *vy,int n)
{
  for (int i=0;i<n;i++) {
    if (sinx || siny) rotateyxp(vx[i],scr_foc,vy[i],vx+i,vy+i);
    vx[i]+=scr_ox;
    vy[i]=scr_oy-vy[i];
  }
}

// draws a Tmonotone
void drawmp(Tmonotone *mp)
{
  coord2d x=mp->xmin;
  Ttrap *t;
  coord2d vx[4];
  coord2d vy[4];

  for (t=mp->traps;t;t=t->next) {
    if (x==t->x2) continue;
    // if the upper and lower segments of the trapezoids don't intersect
    if (t->y11>=t->y12 && t->y21>=t->y22) {
      vx[0]=vx[1]=x;
      vx[2]=vx[3]=t->x2;
      vy[0]=t->y11;
      vy[1]=t->y12;
      vy[2]=t->y22;
      vy[3]=t->y21;
      rotateyxpn(vx,vy,4);
      drawpoly(vx,vy,4);
    }
    // if the upper and lower segments of the trapezoids intersect
    else if (t->y11>=t->y12 || t->y21>=t->y22) {
      if (t->y11<t->y12) {
        coord2d xx,yy;
        split(x,t->x2,t->y11,t->y21,t->y12,t->y22,&xx,&yy);
        vx[0]=xx;
        vx[1]=vx[2]=t->x2;
        vy[0]=yy;
        vy[1]=t->y22;
        vy[2]=t->y21;
        rotateyxpn(vx,vy,3);
        drawpoly(vx,vy,3);
      }
      else {
        coord2d xx,yy;
        split(x,t->x2,t->y11,t->y21,t->y12,t->y22,&xx,&yy);
        vx[0]=vx[1]=x;
        vx[2]=xx;
        vy[0]=t->y11;
        vy[1]=t->y12;
        vy[2]=yy;
        rotateyxpn(vx,vy,3);
        drawpoly(vx,vy,3);
      }
    }
    x=t->x2;
  }
}

// sets the data for the perspective correct texture mapping
// a is from point from a polygon with (u,v)=(0,0)
// p is a vector in the polygon plane along u
// q is a vector in the polygon plane along v
void setdrawdata(Tvector *a,Tvector *p,Tvector *q)
{
  PERSP_A.x=a->y*q->z-a->z*q->y;
  PERSP_A.y=a->x*q->y-a->y*q->x;
  PERSP_A.z=(a->z*q->x-a->x*q->z)*scr_foc;

  PERSP_B.x=a->z*p->y-a->y*p->z;
  PERSP_B.y=a->y*p->x-a->x*p->y;
  PERSP_B.z=(a->x*p->z-a->z*p->x)*scr_foc;

  PERSP_C.x=p->z*q->y-p->y*q->z;
  PERSP_C.y=p->y*q->x-p->x*q->y;
  PERSP_C.z=(p->x*q->z-p->z*q->x)*scr_foc;

#ifdef SOFTWARE_RENDER
  double ax=fabs(PERSP_C.x);
  double ay=fabs(PERSP_C.y);
  double az=fabs(PERSP_C.z/scr_foc);
  if (ax*1000<ay || ax*1000<az)
    drawline=drawlinehor;
  else
    drawline=drawlinetilt;
#if USE_DRAWPOLYVER
  if ((ay*1000<ax || ay*1000<az) && sinx==0 && siny==0)
    drawpoly=drawpolyver;
  else
#endif
    drawpoly=drawpolyhor;
#else
  az=a->y;
  pz=p->y;
  qz=q->y;
#endif
}

// sets the current texture
void setdrawtexture(int txt)
{
  curtxt=textures+txt;
#ifdef DIRECT_3D
  texture_request(curtxt->h);
#endif
}
