#if 0
gcc -s -O2 -o ~/bin/ff-xwin -Wno-unused-result ff-xwin.c -lX11
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#define fatal(...) do { fprintf(stderr,__VA_ARGS__); fputc(10,stderr); exit(1); } while(0)

static unsigned char buf[16];
static Display*dis;
static XImage img;
static Window win;
static XColor xcol;
static XStandardColormap*stdmap;
static XSetWindowAttributes attr;
static int attrmask=CWCursor|CWEventMask|CWColormap;
static int border;
static GC gc;
static XVisualInfo vin;
static XVisualInfo*vout;
static Window root;
static char*opt[128];
static XSizeHints sizehints;
static XClassHint classhint;
static char txt[128];
static XEvent ev;
static KeySym key;

int main(int argc,char**argv) {
  int i,x,y;
  for(i=1;i<argc-1;i+=2) opt[argv[i][0]&127]=argv[i+1];
  fread(buf,1,16,stdin);
  if(memcmp(buf,"farbfeld",8)) fatal("Not farbfeld");
  if(!(dis=XOpenDisplay(0))) fatal("Unable to connect to X display");
  attr.cursor=XCreateFontCursor(dis,XC_tcross);
  attr.event_mask=KeyPressMask|ButtonPressMask|ButtonReleaseMask|ExposureMask;
  root=opt['s']?RootWindow(dis,strtol(opt['s'],0,0)):DefaultRootWindow(dis);
  if(opt['m']) {
    Atom a=XInternAtom(dis,opt['m'],1);
    if(!a) fatal("Atom '%s' is not defined",opt['m']);
    XGetRGBColormaps(dis,root,&stdmap,&i,a);
    if(!i || !stdmap) fatal("Standard colormap '%s' cannot be loaded",opt['m']);
    vin.visualid=stdmap->visualid;
    vout=XGetVisualInfo(dis,VisualIDMask,&vin,&i);
    if(!i || !vout) fatal("Invalid visual ID (%d) in standard colormap",(int)vin.visualid);
    attr.colormap=stdmap->colormap;
  } else {
    vin.visualid=opt['v']?strtol(opt['v'],0,0):XVisualIDFromVisual(DefaultVisual(dis,opt['s']?strtol(opt['s'],0,0):DefaultScreen(dis)));
    vout=XGetVisualInfo(dis,VisualIDMask,&vin,&i);
    if(!i || !vout) fatal("Invalid visual ID");
    if(vout->class!=TrueColor) fatal("Visual %d is not a TrueColor visual",(int)vin.visualid);
    stdmap=XAllocStandardColormap();
    if(!stdmap) fatal("Allocation failed");
    stdmap->red_mult=vout->red_mask&~(vout->red_mask-1);
    stdmap->green_mult=vout->green_mask&~(vout->green_mask-1);
    stdmap->blue_mult=vout->blue_mask&~(vout->blue_mask-1);
    stdmap->red_max=vout->red_mask/stdmap->red_mult;
    stdmap->green_max=vout->green_mask/stdmap->green_mult;
    stdmap->blue_max=vout->blue_mask/stdmap->blue_mult;
    stdmap->base_pixel=0;
    attr.colormap=DefaultColormap(dis,opt['s']?strtol(opt['s'],0,0):DefaultScreen(dis));
  }
  img.width=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  img.height=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  img.format=ZPixmap;
  img.byte_order=LSBFirst;
  img.bitmap_unit=img.bitmap_pad=8;
  img.bits_per_pixel=((img.depth=vout->depth)+7)&~7;
  img.red_mask=vout->red_mask;
  img.green_mask=vout->green_mask;
  img.blue_mask=vout->blue_mask;
  img.data=malloc(img.width*img.height*(img.bits_per_pixel>>3));
  if(!img.data) fatal("Allocation failed");
  XInitImage(&img);
  for(y=0;y<img.height;y++) for(x=0;x<img.width;x++) {
    fread(buf,1,8,stdin);
#define Px(a,b) (stdmap->a##_mult*((((buf[b]<<8)|buf[b+1])*(unsigned long long)stdmap->a##_max)/65535))
    XPutPixel(&img,x,y,stdmap->base_pixel+Px(red,0)+Px(green,2)+Px(blue,4));
#undef Px
  }
  win=XCreateWindow(dis,root,opt['x']?strtol(opt['x'],0,0):0,opt['y']?strtol(opt['y'],0,0):0,img.width,img.height,border,img.depth,InputOutput,vout->visual,attrmask,&attr);
  sizehints.flags=PSize|PMinSize|PMaxSize|PBaseSize;
  sizehints.width=sizehints.min_width=sizehints.max_width=sizehints.base_width=img.width;
  sizehints.height=sizehints.min_height=sizehints.max_height=sizehints.base_height=img.height;
  if(opt['x'] || opt['y']) sizehints.flags|=USPosition;
  classhint.res_name=classhint.res_class=opt['c']?:"Farbfeld";
  XmbSetWMProperties(dis,win,opt['n']?:"Farbfeld Viewer",opt['n']?:"Farbfeld Viewer",argv,argc,&sizehints,0,&classhint);
  if(opt['t']) XSetTransientForHint(dis,win,strtol(opt['t'],0,0));
  gc=XCreateGC(dis,win,0,0);
  XMapWindow(dis,win);
  for(;;) {
    XNextEvent(dis,&ev);
    switch(ev.type) {
      case Expose:
        if(!ev.xexpose.count) XPutImage(dis,win,gc,&img,0,0,0,0,img.width,img.height);
        break;
      case ButtonPress:
        if(ev.xbutton.button==1) {
          sprintf(txt,"(%d,%d)",ev.xbutton.x,ev.xbutton.y);
          XStoreName(dis,win,txt);
          if(ev.xbutton.state&ShiftMask) {
            XDrawLine(dis,win,gc,0,ev.xbutton.y,img.width,ev.xbutton.y);
            XDrawLine(dis,win,gc,ev.xbutton.x,0,ev.xbutton.x,img.height);
          }
        } else if(ev.xbutton.button==3 && !opt['b']) {
          printf("%d %d\n",ev.xbutton.x,ev.xbutton.y);
        }
        break;
      case ButtonRelease:
        if(opt['b']) {
          printf("%d %d %d\n",ev.xbutton.x,ev.xbutton.y,ev.xbutton.button);
          return 0;
        }
        if(ev.xbutton.button==2) return 0;
        break;
      case KeyPress:
        if(XLookupString(&ev.xkey,txt,2,&key,0)) {
          if(key==32) return 0;
          if(*txt=='=') XPutImage(dis,win,gc,&img,0,0,0,0,img.width,img.height);
          if(!opt['k']) {
            if(*txt>='0' && *txt<='7') {
              x=y=stdmap->base_pixel;
              if(*txt&1) x+=stdmap->blue_mult*stdmap->blue_max,y+=stdmap->blue_mult*(stdmap->blue_max>>1);
              if(*txt&2) x+=stdmap->green_mult*stdmap->green_max,y+=stdmap->green_mult*(stdmap->green_max>>1);
              if(*txt&4) x+=stdmap->red_mult*stdmap->red_max,y+=stdmap->red_mult*(stdmap->red_max>>1);
              XSetState(dis,gc,x,y,3,-1);
              XSetLineAttributes(dis,gc,0,LineDoubleDash,CapButt,JoinMiter);
              XSetDashes(dis,gc,0,"\x01\x01",2);
            }
            break;
          }
          if(*txt>='0' && *txt<='9') return *txt;
          if(*txt>='A' && *txt<='Z') return *txt;
          if(*txt>='a' && *txt<='z') return *txt+'A'-'a';
        }
        break;
      case MappingNotify:
        XRefreshKeyboardMapping(&ev.xmapping);
        break;
    }
  }
  return 0;
}

