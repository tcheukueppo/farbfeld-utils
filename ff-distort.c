#if 0
gcc -s -O2 -o ~/bin/ff-distort -Wno-unused-result ff-distort.c -lm
exit
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned short c[4];
} Pixel;

typedef struct {
  double x;
  double y;
} Distort;

typedef void(*Function)(void);

static unsigned char buf[16];
static char interp;
static double iarg[64];
static int niarg;
static char opcode;
static int owidth,oheight,osize,iwidth,iheight,isize,vwidth,vheight,vsize;
static Pixel*pic;
static double arg[64];
static int narg;
static Distort*map;
static Distort*map2;
static int EMIT_channel;

#define TAU (6.283185307179586476925286766559005768394)
#define fatal(...) do { fprintf(stderr,__VA_ARGS__); fputc('\n',stderr); exit(1); } while(0)
#define MAP(x,y) (map[(y)*owidth+(x)])
#define MAP2(x,y) (map2[(y)*owidth+(x)])
#define MAPXCHG() do { Distort*m=map2; map2=map; map=m; } while(0)
#define PM(xx,yy) (getpixel(MAP(xx,yy).x,MAP(xx,yy).y)[EMIT_channel])
#define P(x,y) (getpixel(x,y)[EMIT_channel])
#define EMITCH(c,z) (EMIT_channel=c,(z))
#define EMIT(z) putpixel(EMITCH(0,z),EMITCH(1,z),EMITCH(2,z),EMITCH(3,z))

static inline unsigned short*getpixel(int x,int y) {
  x+=vsize; y+=vsize;
  return pic[(x<0?0:x>=vwidth?vwidth-1:x)+vwidth*(y<0?0:y>=vheight?vheight-1:y)].c;
}

static inline void putpixel(int r,int g,int b,int a) {
  r=r<0?0:r>65535?65535:r;
  g=g<0?0:g>65535?65535:g;
  b=b<0?0:b>65535?65535:b;
  a=a<0?0:a>65535?65535:a;
  putchar(r>>8); putchar(r);
  putchar(g>>8); putchar(g);
  putchar(b>>8); putchar(b);
  putchar(a>>8); putchar(a);
}

static inline double cubic(double a,double b,double c,double d,double x) {
  return b+0.5*x*(c-a+x*(2.0*a-5.0*b+4.0*c-d+x*(3.0*(b-c)+d-a)));
}

static int pointInTriangle(Distort p1,Distort p2,Distort p3,double x,double y) {
  double s1=(p2.y-p1.y)*(x-p1.x)+(p1.x-p2.x)*(y-p1.y);
  double s2=(p3.y-p2.y)*(x-p2.x)+(p2.x-p3.x)*(y-p2.y);
  double s3=(p1.y-p3.y)*(x-p3.x)+(p3.x-p1.x)*(y-p3.y);
  if(s1<=0.0) s1=-s1,s2=-s2,s3=-s3;
  return s1>=*iarg && s2>=*iarg && s3>=*iarg;
}

static void ifunc_a(void) {
  int x,y,xx,yy;
  double X,Y,Xl,Xr,Yt,Yb,z;
  double c[4];
  double total;
  unsigned short*p;
  for(y=0;y<oheight;y++) for(x=0;x<owidth;x++) {
    X=MAP(x,y).x; Y=MAP(x,y).y;
    Xl=MAP(x?x-1:0,y).x; Xr=MAP(x==owidth-1?x:x+1,y).x;
    Yt=MAP(x,y?y-1:0).y; Yb=MAP(x,y==oheight-1?y:y+1).y;
    if(isnan(X+Y+Xl+Xr+Yt+Yb)) {
      putpixel(0,0,0,0);
      continue;
    }
    if(Xl>Xr) z=Xl,Xl=Xr,Xr=z;
    if(Yt>Yb) z=Yt,Yt=Yb,Yb=z;
    if(Xl>X) Xl=X;
    if(Xr<X) Xr=X;
    if(Yt>Y) Yt=Y;
    if(Yb<Y) Yb=Y;
    c[0]=c[1]=c[2]=c[3]=total=0.0;
    if(ceil(Xl)==ceil(Xr)) Xl-=0.5,Xr+=0.5;
    if(ceil(Yt)==ceil(Yb)) Yt-=0.5,Yb+=0.5;
    for(xx=ceil(Xl);xx<ceil(Xr);xx++) for(yy=ceil(Yt);yy<ceil(Yb);yy++) {
      p=getpixel(xx,yy);
      z=p[3]+65535;
      X=(xx-(Xr+Xl)/2.0)/(Xr-Xl);
      Y=(yy-(Yb+Yt)/2.0)/(Yb-Yt);
      z*=pow(X*X+Y*Y+arg[1],-*iarg);
      c[0]+=z*p[0]; c[1]+=z*p[1]; c[2]+=z*p[2]; c[3]+=z*p[3];
      total+=z;
    }
    putpixel(c[0]/total,c[1]/total,c[2]/total,c[3]/total);
  }
}

static void ifunc_c(void) {
  int i,x,y;
  double xf,yf;
  for(i=0;i<osize;i++) {
    if(isnan(map[i].x) || isnan(map[i].y)) {
      putpixel(0,0,0,0);
      continue;
    }
    x=floor(map[i].x);
    y=floor(map[i].y);
    xf=map[i].x-x;
    yf=map[i].y-y;
    EMIT(cubic(
      cubic(P(x-1,y-1),P(x-1,y),P(x-1,y+1),P(x-1,y+2),yf),
      cubic(P(x,y-1),P(x,y),P(x,y+1),P(x,y+2),yf),
      cubic(P(x+1,y-1),P(x+1,y),P(x+1,y+1),P(x+1,y+2),yf),
      cubic(P(x+2,y-1),P(x+2,y),P(x+2,y+1),P(x+2,y+2),yf),
    xf));
  }
}

static void ifunc_l(void) {
  int i,x,y;
  double xf,yf;
  for(i=0;i<osize;i++) {
    if(isnan(map[i].x) || isnan(map[i].y)) {
      putpixel(0,0,0,0);
      continue;
    }
    x=floor(map[i].x);
    y=floor(map[i].y);
    xf=map[i].x-x;
    yf=map[i].y-y;
    EMIT(xf*yf*P(x,y)+(1.0-xf)*yf*P(x-1,y)+xf*(1.0-yf)*P(x,y-1)+(1.0-xf)*(1.0-yf)*P(x-1,y-1));
  }
}

static void ifunc_m(void) {
  int x,y,xx,yy;
  double c[4];
  double tot,z;
  Distort tl,br;
  unsigned short*p;
  for(y=0;y<oheight;y++) for(x=0;x<owidth;x++) {
    tl=br=MAP(x,y);
    xx=(x?x-1:0);
    if(MAP(xx,y).x<tl.x) tl.x=MAP(xx,y).x;
    if(MAP(xx,y).y<tl.y) tl.y=MAP(xx,y).y;
    if(MAP(xx,y).x>br.x) br.x=MAP(xx,y).x;
    if(MAP(xx,y).y>br.y) br.y=MAP(xx,y).y;
    xx=(x==owidth-1?x:x+1);
    if(MAP(xx,y).x<tl.x) tl.x=MAP(xx,y).x;
    if(MAP(xx,y).y<tl.y) tl.y=MAP(xx,y).y;
    if(MAP(xx,y).x>br.x) br.x=MAP(xx,y).x;
    if(MAP(xx,y).y>br.y) br.y=MAP(xx,y).y;
    yy=(y?y-1:0);
    if(MAP(x,yy).x<tl.x) tl.x=MAP(x,yy).x;
    if(MAP(x,yy).y<tl.y) tl.y=MAP(x,yy).y;
    if(MAP(x,yy).x>br.x) br.x=MAP(x,yy).x;
    if(MAP(x,yy).y>br.y) br.y=MAP(x,yy).y;
    yy=(y==oheight-1?y:y+1);
    if(MAP(x,yy).x<tl.x) tl.x=MAP(x,yy).x;
    if(MAP(x,yy).y<tl.y) tl.y=MAP(x,yy).y;
    if(MAP(x,yy).x>br.x) br.x=MAP(x,yy).x;
    if(MAP(x,yy).y>br.y) br.y=MAP(x,yy).y;
    if(isnan(tl.x) || isnan(tl.y) || isnan(br.x) || isnan(br.y)) {
      putpixel(0,0,0,0);
      continue;
    }
    if(niarg>2) tl.x-=iarg[2],br.x+=iarg[2],tl.y-=iarg[3],br.y+=iarg[3];
    tot=iarg[1]; p=getpixel(floor(MAP(x,y).x),floor(MAP(x,y).y));
    if(niarg>4) tot+=iarg[4]*hypot(MAP(x,y).x-floor(MAP(x,y).x)-0.5,MAP(x,y).y-floor(MAP(x,y).y)-0.5);
    c[0]=p[0]*tot; c[1]=p[1]*tot; c[2]=p[2]*tot; c[3]=p[3]*tot;
    if(z=(ceil(tl.x)-tl.x)*(ceil(tl.y)-tl.y)) {
      p=getpixel(floor(tl.x),floor(tl.y));
      tot+=z=pow(z,iarg[0]);
      c[0]+=p[0]*z; c[1]+=p[1]*z; c[2]+=p[2]*z; c[3]+=p[3]*z;
    }
    if(z=(br.x-floor(br.x))*(ceil(tl.y)-tl.y)) {
      p=getpixel(floor(br.x),floor(tl.y));
      tot+=z=pow(z,iarg[0]);
      c[0]+=p[0]*z; c[1]+=p[1]*z; c[2]+=p[2]*z; c[3]+=p[3]*z;
    }
    if(z=(ceil(tl.x)-tl.x)*(br.y-floor(br.y))) {
      p=getpixel(floor(tl.x),floor(br.y));
      tot+=z=pow(z,iarg[0]);
      c[0]+=p[0]*z; c[1]+=p[1]*z; c[2]+=p[2]*z; c[3]+=p[3]*z;
    }
    if(z=(br.x-floor(br.x))*(br.y-floor(br.y))) {
      p=getpixel(floor(br.x),floor(br.y));
      tot+=z=pow(z,iarg[0]);
      c[0]+=p[0]*z; c[1]+=p[1]*z; c[2]+=p[2]*z; c[3]+=p[3]*z;
    }
    for(yy=ceil(tl.y);yy<floor(br.y);yy++) for(xx=ceil(tl.x);xx<floor(br.x);xx++) {
      p=getpixel(xx,yy);
      tot+=1.0;
      c[0]+=p[0]; c[1]+=p[1]; c[2]+=p[2]; c[3]+=p[3];
    }
    if(tot<=0.0) putpixel(0,0,0,0); else putpixel(c[0]/tot,c[1]/tot,c[2]/tot,c[3]/tot);
  }
}

static void ifunc_n(void) {
  int x,y;
  for(y=0;y<oheight;y++) for(x=0;x<owidth;x++)
    if(isnan(MAP(x,y).x) || isnan(MAP(x,y).y)) putpixel(0,0,0,0); else EMIT(PM(x,y));
}

static void ifunc_t(void) {
  int x,y,xx,yy,x0,x1,y0,y1,xx0,xx1,yy0,yy1;
  double c[4];
  double tot,z;
  unsigned short*p;
  *iarg*=-1.0;
  for(y=0;y<oheight;y++) for(x=0;x<owidth;x++) {
    x0=x?x-1:0;
    x1=x==owidth-1?x:x+1;
    y0=y?y-1:0;
    y1=y==oheight-1?y:y+1;
    c[0]=c[1]=c[2]=c[3]=tot=0.0;
    if(niarg>1) {
      tot+=z=(MAP(x,y).x-floor(MAP(x,y).x))*(MAP(x,y).y-floor(MAP(x,y).y))*iarg[1];
      p=getpixel(floor(MAP(x,y).x),floor(MAP(x,y).y));
      c[0]+=p[0]*z; c[1]+=p[1]*z; c[2]+=p[2]*z; c[3]+=p[3]*z;
      tot+=z=(floor(MAP(x,y).x)+1.0-MAP(x,y).x)*(MAP(x,y).y-floor(MAP(x,y).y))*iarg[1];
      p=getpixel(floor(MAP(x,y).x)-1,floor(MAP(x,y).y));
      c[0]+=p[0]*z; c[1]+=p[1]*z; c[2]+=p[2]*z; c[3]+=p[3]*z;
      tot+=z=(MAP(x,y).x-floor(MAP(x,y).x))*(floor(MAP(x,y).y)+1.0-MAP(x,y).y)*iarg[1];
      p=getpixel(floor(MAP(x,y).x),floor(MAP(x,y).y)-1);
      c[0]+=p[0]*z; c[1]+=p[1]*z; c[2]+=p[2]*z; c[3]+=p[3]*z;
      tot+=z=(floor(MAP(x,y).x)+1.0-MAP(x,y).x)*(floor(MAP(x,y).y)+1.0-MAP(x,y).y)*iarg[1];
      p=getpixel(floor(MAP(x,y).x)-1,floor(MAP(x,y).y)-1);
      c[0]+=p[0]*z; c[1]+=p[1]*z; c[2]+=p[2]*z; c[3]+=p[3]*z;
    }
    xx0=floor(MAP(x,y).x);
    xx1=ceil(MAP(x,y).x);
    yy0=floor(MAP(x,y).y);
    yy1=ceil(MAP(x,y).y);
    for(yy=y0;yy<=y1;yy++) for(xx=x0;xx<=x1;xx++) {
      if(MAP(xx,yy).x<xx0) xx0=floor(MAP(xx,yy).x);
      if(MAP(xx,yy).x>xx1) xx1=ceil(MAP(xx,yy).x);
      if(MAP(xx,yy).y<yy0) yy0=floor(MAP(xx,yy).y);
      if(MAP(xx,yy).y>yy1) yy1=ceil(MAP(xx,yy).y);
    }
    if(yy1>iheight-1) yy1=iheight-1;
    if(xx1>iwidth-1) xx1=iwidth-1;
    for(yy=yy0<0?0:yy0;yy<=yy1;yy++) for(xx=xx0<0?0:xx0;xx<xx1;xx++) {
      if(pointInTriangle(MAP(x,y),MAP(x0,y0),MAP(x,y0),xx,yy)
       ||pointInTriangle(MAP(x,y),MAP(x,y0),MAP(x1,y0),xx,yy)
       ||pointInTriangle(MAP(x,y),MAP(x1,y0),MAP(x1,y),xx,yy)
       ||pointInTriangle(MAP(x,y),MAP(x1,y),MAP(x1,y1),xx,yy)
       ||pointInTriangle(MAP(x,y),MAP(x1,y1),MAP(x,y1),xx,yy)
       ||pointInTriangle(MAP(x,y),MAP(x,y1),MAP(x0,y1),xx,yy)
       ||pointInTriangle(MAP(x,y),MAP(x0,y1),MAP(x0,y),xx,yy)
       ||pointInTriangle(MAP(x,y),MAP(x0,y),MAP(x0,y0),xx,yy)) {
        p=getpixel(xx,yy);
        tot+=1.0;
        c[0]+=p[0]; c[1]+=p[1]; c[2]+=p[2]; c[3]+=p[3];
      }
    }
    if(isnan(tot) || tot<=0.0) putpixel(0,0,0,0); else putpixel(c[0]/tot,c[1]/tot,c[2]/tot,c[3]/tot);
  }
}

static void dfunc_a(void) {
  int x,y;
  if(narg<7) arg[6]=0;
  for(y=0;y<oheight;y++) for(x=0;x<owidth;x++) {
    Distort*m=&MAP(x,y);
    double d=arg[0]*m->x+arg[1]*m->y+arg[2]+arg[6]*y;
    m->y=arg[3]*m->x+arg[4]*m->y+arg[5]+arg[6]*y;
    m->x=d;
  }
}

static void dfunc_b(void) {
  int i;
  if(narg<6) arg[5]=1.0-arg[2]-arg[3]-arg[4];
  for(i=0;i<osize;i++) {
    Distort*m=map+i;
    double d=hypot(m->x-arg[0],m->y-arg[1])/(arg[0]<arg[1]?arg[0]:arg[1]);
    double r=arg[2]*d*d*d+arg[3]*d*d+arg[4]*d+arg[5];
    m->x=(m->x-arg[0])*r+arg[0];
    m->y=(m->y-arg[1])*r+arg[1];
  }
}

static void dfunc_c(void) {
  int x,y;
  double t=arg[0]+arg[1]*4.0+arg[2]*4.0+(narg<4?0.0:arg[3]+arg[4]*4.0+arg[5]*4.0);
  for(y=0;y<oheight;y++) for(x=0;x<owidth;x++) {
    MAP2(x,y).x=MAP(x,y).x*arg[0]
     +(MAP((x+1)%owidth,y).x+MAP((x+owidth-1)%owidth,y).x+MAP(x,(y+1)%oheight).x+MAP(x,(y+oheight-1)%oheight).x)*arg[1]
     +(MAP((x+1)%owidth,(y+1)%oheight).x+MAP((x+owidth-1)%owidth,(y+1)%oheight).x
     +MAP((x+1)%owidth,(y+oheight-1)%oheight).x+MAP((x+owidth-1)%owidth,(y+oheight-1)%oheight).x)*arg[2];
    MAP2(x,y).y=MAP(x,y).y*arg[0]
     +(MAP((x+1)%owidth,y).y+MAP((x+owidth-1)%owidth,y).y+MAP(x,(y+1)%oheight).y+MAP(x,(y+oheight-1)%oheight).y)*arg[1]
     +(MAP((x+1)%owidth,(y+1)%oheight).y+MAP((x+owidth-1)%owidth,(y+1)%oheight).y
     +MAP((x+1)%owidth,(y+oheight-1)%oheight).y+MAP((x+owidth-1)%owidth,(y+oheight-1)%oheight).y)*arg[2];
    if(narg>=4) {
      MAP2(x,y).x+=MAP(x,y).y*arg[3]
       +(MAP((x+1)%owidth,y).y+MAP((x+owidth-1)%owidth,y).y+MAP(x,(y+1)%oheight).y+MAP(x,(y+oheight-1)%oheight).y)*arg[4]
       +(MAP((x+1)%owidth,(y+1)%oheight).y+MAP((x+owidth-1)%owidth,(y+1)%oheight).y
       +MAP((x+1)%owidth,(y+oheight-1)%oheight).y+MAP((x+owidth-1)%owidth,(y+oheight-1)%oheight).y)*arg[5];
      MAP2(x,y).y+=MAP(x,y).x*arg[3]
       +(MAP((x+1)%owidth,y).x+MAP((x+owidth-1)%owidth,y).x+MAP(x,(y+1)%oheight).x+MAP(x,(y+oheight-1)%oheight).x)*arg[4]
       +(MAP((x+1)%owidth,(y+1)%oheight).x+MAP((x+owidth-1)%owidth,(y+1)%oheight).x
       +MAP((x+1)%owidth,(y+oheight-1)%oheight).x+MAP((x+owidth-1)%owidth,(y+oheight-1)%oheight).x)*arg[5];
    }
    MAP2(x,y).x/=t;
    MAP2(x,y).y/=t;
  }
  MAPXCHG();
}

static void dfunc_d(void) {
  int x,y;
  arg[0]/=0.5*owidth;
  arg[1]/=0.5*oheight;
  for(y=0;y<oheight;y++) for(x=0;x<owidth;x++) {
    MAP(x,y).x+=(x-0.5*owidth)*arg[0];
    MAP(x,y).y+=(y-0.5*oheight)*arg[1];
  }
}

static void dfunc_i(void) {
  int i;
  for(i=0;i<osize;i++) {
    Distort*m=map+i;
    double d=hypot(m->x-arg[0],m->y-arg[1]);
    m->x=pow(d,arg[2])*(m->x-arg[0])/d+arg[0];
    m->y=pow(d,arg[2])*(m->y-arg[1])/d+arg[1];
  }
}

static void dfunc_m(void) {
  int x,y;
  for(y=0;y<oheight;y++) for(x=0;x<owidth;x++) {
    MAP(x,y).x=x*arg[0];
    MAP(x,y).y=y*arg[0];
  }
}

static void dfunc_p(void) {
  int x,y;
  if(narg<5) arg[4]=1;
  for(y=0;y<oheight;y++) for(x=0;x<owidth;x++) {
    Distort*m=&MAP(x,y);
    double X=hypot(m->x/arg[0]-arg[2],m->y/arg[1]-arg[3]);
    double Y=fmod(TAU+atan2(m->y-arg[3],m->x-arg[2]),TAU)*arg[4]/TAU;
    m->x=X;
    m->y=Y;
  }
}

static void dfunc_q(void) {
  int x,y;
  if(narg<5) arg[4]=1;
  for(y=0;y<oheight;y++) for(x=0;x<owidth;x++) {
    Distort*m=&MAP(x,y);
    double X=m->x*arg[0]*cos(m->y*TAU/arg[4])+arg[2];
    double Y=m->x*arg[1]*sin(m->y*TAU/arg[4])+arg[3];
    m->x=X;
    m->y=Y;
  }
}

static void dfunc_r(void) {
  int i;
  double s,c;
  s=sin(*arg*TAU/360.0);
  c=cos(*arg*TAU/360.0);
  if(narg<2) arg[1]=arg[2]=0.0;
  for(i=0;i<osize;i++) {
    double x=(map[i].x-arg[1])*c+(map[i].y-arg[2])*s;
    double y=(map[i].y-arg[2])*c-(map[i].x-arg[1])*s;
    map[i].x=x+arg[1];
    map[i].y=y+arg[2];
  }
}

static void dfunc_s(void) {
  int i;
  double xs=0.5*iwidth;
  double ys=0.5*iheight;
  if(narg<3) arg[2]=1.0;
  arg[0]*=TAU;
  for(i=0;i<osize;i++) {
    double x=map[i].x;
    double y=map[i].y;
    double h=pow(hypot((x-xs)/xs,(y-ys)/ys)+arg[2],-arg[1])*arg[0];
    map[i].x=(x-xs)*cos(h)+(y-ys)*sin(h)+xs;
    map[i].y=(y-ys)*cos(h)-(x-xs)*sin(h)+ys;
  }
}

static void dfunc_t(void) {
  int i;
  for(i=0;i<osize;i++) {
    while(map[i].x<0.0) map[i].x+=iwidth;
    while(map[i].y<0.0) map[i].y+=iheight;
    map[i].x=fmod(map[i].x,iwidth);
    map[i].y=fmod(map[i].y,iheight);
  }
}

static void dfunc_v(void) {
  int i;
  if(!narg) arg[0]=0.0;
  if(narg<2) arg[1]=arg[0];
  for(i=0;i<osize;i++)
    if(map[i].x<-arg[0] || map[i].x>=iwidth+arg[0] || map[i].y<-arg[1] || map[i].y>=iheight+arg[1]) map[i].x=map[i].y=0.0/0.0;
}

static void dfunc_w(void) {
  int i;
  double x,y;
  arg[1]*=TAU;
  arg[4]*=TAU;
  for(i=0;i<osize;i++) {
    x=map[i].x+arg[0]*sin(arg[1]*(map[i].y-arg[2]));
    y=map[i].y+arg[3]*sin(arg[4]*(map[i].x-arg[5]));
    map[i].x=x;
    map[i].y=y;
  }
}

static void dfunc_x(void) {
  int i;
  for(i=0;i<osize;i++) {
    map[i].x=floor(map[i].x*arg[0])/arg[0];
    map[i].y=floor(map[i].y*arg[1])/arg[1];
  }
}

static void dfunc_y(void) {
  int i;
  *arg/=65535.0;
  for(i=0;i<osize;i++) if(!isnan(map[i].x) && !isnan(map[i].y)) {
    unsigned short*p=getpixel(map[i].x,map[i].y);
    map[i].x+=*arg*(65535-p[3]);
  }
}

static const Function ifuncs[128]={
  ['a']=ifunc_a,
  ['c']=ifunc_c,
  ['l']=ifunc_l,
  ['m']=ifunc_m,
  ['n']=ifunc_n,
  ['t']=ifunc_t,
};

static const Function dfuncs[128]={
  ['a']=dfunc_a,
  ['b']=dfunc_b,
  ['c']=dfunc_c,
  ['d']=dfunc_d,
  ['i']=dfunc_i,
  ['m']=dfunc_m,
  ['p']=dfunc_p,
  ['q']=dfunc_q,
  ['r']=dfunc_r,
  ['s']=dfunc_s,
  ['t']=dfunc_t,
  ['v']=dfunc_v,
  ['w']=dfunc_w,
  ['x']=dfunc_x,
  ['y']=dfunc_y,
};

int main(int argc,char**argv) {
  char*s;
  int i,j;
  if(argc<4) fatal("Too few arguments");
  if(argv[1][0]=='+') {
    argv++;
    vsize=strtol(argv[0]+1,0,0);
    if(--argc<4) fatal("Too few arguments");
  }
  interp=argv[1][0]&127;
  if(!ifuncs[interp]) fatal("Invalid interpolation mode (%c)",interp);
  niarg=0; s=argv[1]+1; while(niarg<64 && (*s==',' || *s==':')) iarg[niarg++]=strtod(s+1,&s);
  owidth=strtol(argv[2],0,0);
  oheight=strtol(argv[3],0,0);
  if(!(map=malloc((osize=owidth*oheight)*sizeof(Distort)))) fatal("Allocation failed");
  if(!(map2=malloc(osize*sizeof(Distort)))) fatal("Allocation failed");
  fread(buf,1,16,stdin);
  iwidth=(vwidth=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11])-(vsize<<1);
  iheight=(vheight=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15])-(vsize<<1);
  if(iwidth<=0 || iheight<=0 || vwidth<=0 || vheight<=0) fatal("Invalid picture size");
  for(j=0;j<oheight;j++) for(i=0;i<owidth;i++) {
    MAP(i,j).x=argv[2][0]=='+'?(double)i:(iwidth*(double)i)/owidth;
    MAP(i,j).y=argv[3][0]=='+'?(double)j:(iheight*(double)j)/oheight;
  }
  if(!(pic=malloc((isize=vwidth*vheight)*sizeof(Pixel)))) fatal("Allocation failed");
  buf[8]=owidth>>24; buf[9]=owidth>>16; buf[10]=owidth>>8; buf[11]=owidth;
  buf[12]=oheight>>24; buf[13]=oheight>>16; buf[14]=oheight>>8; buf[15]=oheight;
  fwrite(buf,1,16,stdout);
  for(i=0;i<isize;i++) {
    fread(buf,1,8,stdin);
    pic[i].c[0]=(buf[0]<<8)|buf[1];
    pic[i].c[1]=(buf[2]<<8)|buf[3];
    pic[i].c[2]=(buf[4]<<8)|buf[5];
    pic[i].c[3]=(buf[6]<<8)|buf[7];
  }
  for(i=4;i<argc;i++) {
    opcode=argv[i][0]&127;
    narg=0; s=argv[i]+1; while(narg<64 && (*s==',' || *s==':')) arg[narg++]=strtod(s+1,&s);
    if(!dfuncs[opcode]) fatal("Invalid distortion mode (%c)",opcode);
    dfuncs[opcode]();
  }
  ifuncs[interp]();
  return 0;
}

