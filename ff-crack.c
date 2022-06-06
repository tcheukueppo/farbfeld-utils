#if 0
gcc -s -O2 -o ~/bin/ff-crack -Wno-unused-result ff-crack.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define fatal(...) do{ fprintf(stderr,__VA_ARGS__); exit(1); }while(0)

typedef struct {
  unsigned char d[8];
} Pixel;

typedef struct {
  int x,y;
} XY;

static unsigned char buf[16];
static int width,height;
static int xmask,ymask,cycles,qcount;
static Pixel*pic;
static int opt[128];
static XY*queue;

static inline void process(void) {
  int x1,x2,y1,y2,dx,dy,x,y,x1i,y1i,x2i,y2i,n,den,num,add,i,xx,yy;
  Pixel z;
  do x1=random()&xmask; while(x1>=width);
  do y1=random()&ymask; while(y1>=height);
  if(!pic[y1*width+x1].d[6]) {
    i=1;
    switch(random()&3) {
      case 0:
        while(x1-i>=0 || x1+i<width) {
          if(x1-i>=0 && pic[y1*width+x1-i].d[6]) { x1-=i; goto ok; }
          if(x1+i<width && pic[y1*width+x1+i].d[6]) { x1+=i; goto ok; }
          ++i;
        }
        return;
      case 1:
        while(y1-i>=0 || y1+i<height) {
          if(y1-i>=0 && pic[(y1-i)*width+x1].d[6]) { y1-=i; goto ok; }
          if(y1+i<height && pic[(y1+i)*width+x1].d[6]) { y1+=i; goto ok; }
          ++i;
        }
        return;
      default:
        return;
    }
  }
  ok:
  do x2=random()&xmask; while(x2>=width);
  do y2=random()&ymask; while(y2>=height);
  switch(opt['d']) {
    case 1:
      if(random()&1) x2=x1; else y2=y1;
      break;
    case 2:
      if(y2<=y1) return;
      break;
    case 3:
      if(y2>=y1) return;
      break;
    case 4:
      x2=(x2+width-x1)/2;
      y2=(y2+height-y1)/2;
      break;
    case 5:
      if(abs(y2-y1)>abs(x2-x1)) return;
      break;
    case 6:
      if(abs(y2-y1)<abs(x2-x1)) return;
      break;
    case 7:
      x2=width-1-y1;
      y2=x1;
      break;
  }
  if(x1==x2 && y1==y2) return;
  qcount=0;
  z=pic[y1*width+x1];
  dx=abs(x1-x2);
  dy=abs(y1-y2);
  x=x1;
  y=y1;
  x1i=dx>=dy?0:x2>=x1?1:-1;
  y1i=dx<dy?0:y2>=y1?1:-1;
  x2i=dx<dy?0:x2>=x1?1:-1;
  y2i=dx>=dy?0:y2>=y1?1:-1;
  n=dx>=dy?dx:dy;
  den=n;
  num=n>>1;
  add=dx<dy?dx:dy;
  for(;;) {
    if(x<0 || x>=width || y<0 || y>=height) break;
    if(pic[y*width+x].d[6]) {
      if(qcount || !cycles) break;
      z=pic[y*width+x];
      if(opt['e']) {
        for(i=0;i<6;i++) z.d[i]=z.d[i]>opt['e']?z.d[i]-opt['e']:0;
      }
      goto skip;
    } else if(qcount>=opt['q']) {
      cycles=0;
      xx=queue[qcount-opt['q']].x;
      yy=queue[qcount-opt['q']].y;
      pic[yy*width+xx]=z;
      queue[qcount-opt['q']].x=x;
      queue[qcount-opt['q']].y=y;
      if(++qcount==opt['q']<<1) qcount=opt['q'];
      if(opt['k'] && (random()&1023)<opt['k']) break;
    } else {
      queue[qcount].x=x;
      queue[qcount].y=y;
      ++qcount;
    }
    if(opt['c']) {
      i=0;
      for(yy=y-opt['c'];yy<=y+opt['c'];yy++) {
        for(xx=x-opt['c'];xx<=x+opt['c'];xx++) {
          if((xx<x && x1<x2) || (xx>x && x1>x2) || (yy<y && y1<y2) || (yy>y && y1>y2)) continue;
          if(opt['o'] && (yy-y)*(yy-y)+(xx-x)*(xx-x)>opt['c']*opt['c']) continue;
          if(xx<0 || xx>=width || yy<0 || yy>=height || pic[yy*width+xx].d[6]) ++i;
        }
      }
      if(i>opt['b']) break;
    }
    skip:
    num+=add;
    if(num>=den) {
      num-=den;
      x+=x1i;
      y+=y1i;
    }
    x+=x2i;
    y+=y2i;
  }
}

int main(int argc,char**argv) {
  int i;
  for(i=1;i<argc;i++) if(argv[i][0]) {
    if(argv[i][1]!='=') fatal("Improper argument: %s\n",argv[i]);
    opt[argv[i][0]&127]=strtol(argv[i]+2,0,0);
  }
  if(!opt['q']) opt['q']=1;
  fread(buf,1,16,stdin);
  width=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  height=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  pic=malloc(width*height*8);
  queue=malloc(opt['q']*sizeof(XY));
  if(!pic || !queue) fatal("Allocation failed\n");
  fwrite(buf,1,16,stdout);
  fread(pic,width,height*8,stdin);
  srandom(opt['s']);
  xmask=width-1; xmask|=xmask>>16; xmask|=xmask>>8; xmask|=xmask>>4; xmask|=xmask>>2; xmask|=xmask>>1;
  ymask=height-1; ymask|=ymask>>16; ymask|=ymask>>8; ymask|=ymask>>4; ymask|=ymask>>2; ymask|=ymask>>1;
  for(;;) {
    if(cycles++>opt['u']) break;
    process();
  }
  fwrite(pic,width,height*8,stdout);
  return 0;
}
