#if 0
gcc -s -O2 -o ~/bin/ff-xcapture ff-xcapture.c -lX11
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#define fatal(...) do { fprintf(stderr,__VA_ARGS__); fputc(10,stderr); exit(1); } while(0)

static Display*dis;
static Drawable win;
static XWindowAttributes attr;
static XImage*img;
static XVisualInfo vin;
static XVisualInfo*vout;
static XColor*cmap;
static unsigned long pixmult;
static XColor pixel;
static int rshift,gshift,bshift;
static XRectangle rect;

static void select_area(void) {
  Cursor cur=XCreateFontCursor(dis,XC_cross_reverse);
  XEvent ev;
  Time tim=CurrentTime;
  int cou=1;
  XGrabPointer(dis,win,0,ButtonPressMask|ButtonReleaseMask,GrabModeSync,GrabModeAsync,win,cur,CurrentTime);
  while(cou) {
    XAllowEvents(dis,SyncPointer,tim);
    XWindowEvent(dis,win,ButtonPressMask|ButtonReleaseMask,&ev);
    tim=ev.xbutton.time;
    if(ev.type==ButtonPress) {
      ++cou;
      if(ev.xbutton.button==1 && !rect.width) {
        rect.x=ev.xbutton.x;
        rect.y=ev.xbutton.y;
      } else if(ev.xbutton.button==3 && !rect.width) {
        int w=ev.xbutton.x-rect.x+1;
        int h=ev.xbutton.y-rect.y+1;
        if(w>0 && h>0) {
          rect.width=w;
          rect.height=h;
          --cou;
        }
      }
    } else if(ev.type==ButtonRelease) {
      --cou;
    }
  }
  XUngrabPointer(dis,tim);
}

int main(int argc,char**argv) {
  int i,x,y;
  if(!(dis=XOpenDisplay(0))) fatal("Unable to connect to X display");
  if(argc<2 || !argv[1][0] || ((argv[1][0]=='r' || argv[1][0]=='+') && !argv[1][1])) win=DefaultRootWindow(dis);
  else if(argv[1][0]=='r' || argv[1][0]=='+') win=RootWindow(dis,strtol(argv[1]+1,0,0));
  else win=strtol(argv[1],0,0);
  if(argc>3 && argv[1][0]!='+') {
    XGetGeometry(dis,win,&attr.root,&attr.x,&attr.y,&attr.width,&attr.height,&attr.border_width,&attr.depth);
    attr.colormap=strtol(argv[2],0,0);
    vin.visualid=strtol(argv[3],0,0);
  } else {
    XGetWindowAttributes(dis,win,&attr);
    vin.visualid=XVisualIDFromVisual(attr.visual);
  }
  vout=XGetVisualInfo(dis,VisualIDMask,&vin,&i);
  if(!vout) fatal("Unexpected invalid visual ID number: %d",(int)vin.visualid);
  if(vout->class==TrueColor || vout->class==DirectColor) {
    pixmult=(vout->red_mask&~(vout->red_mask-1))|(vout->green_mask&~(vout->green_mask-1))|(vout->blue_mask&~(vout->blue_mask-1));
  } else {
    pixmult=1;
    vout->red_mask=vout->green_mask=vout->blue_mask=(1<<vout->depth)-1;
  }
  rshift=0; i=vout->red_mask; while(!(i&1)) i>>=1,rshift++;
  gshift=0; i=vout->green_mask; while(!(i&1)) i>>=1,gshift++;
  bshift=0; i=vout->blue_mask; while(!(i&1)) i>>=1,bshift++;
  cmap=malloc(vout->colormap_size*sizeof(XColor));
  if(!cmap) fatal("Allocation of colormap failed");
  for(i=0;i<vout->colormap_size;i++) cmap[i].pixel=i*pixmult;
  XQueryColors(dis,attr.colormap,cmap,vout->colormap_size);
  if(argc>1 && argv[1][0]=='+') select_area();
  else rect.width=attr.width,rect.height=attr.height;
  img=XGetImage(dis,win,rect.x,rect.y,rect.width,rect.height,-1,ZPixmap);
  if(!img) fatal("Failed to get image");
  fwrite("farbfeld",1,8,stdout);
  putchar(rect.width>>24); putchar(rect.width>>16); putchar(rect.width>>8); putchar(rect.width);
  putchar(rect.height>>24); putchar(rect.height>>16); putchar(rect.height>>8); putchar(rect.height);
  for(y=0;y<rect.height;y++) for(x=0;x<rect.width;x++) {
    pixel.pixel=XGetPixel(img,x,y);
    pixel.red=cmap[(pixel.pixel&vout->red_mask)>>rshift].red;
    pixel.green=cmap[(pixel.pixel&vout->green_mask)>>gshift].green;
    pixel.blue=cmap[(pixel.pixel&vout->blue_mask)>>bshift].blue;
    putchar(pixel.red>>8); putchar(pixel.red);
    putchar(pixel.green>>8); putchar(pixel.green);
    putchar(pixel.blue>>8); putchar(pixel.blue);
    putchar(255); putchar(255);
  }
  return 0;
}

