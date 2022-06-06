#if 0
gcc -s -O2 -o ~/bin/ff-flood -Wno-unused-result ff-flood.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned char d[8];
} Pixel;

static unsigned char buf[16];
static Pixel*pic;
static int width,height,floodx,floody,chan;
static Pixel targetc,floodc,borderc;
static int opt[128];

static Pixel parse_color(const char*s) {
  Pixel c;
  switch(strlen(s)) {
    case 6:
      sscanf(s,"%2hhx%2hhx%2hhx",c.d+0,c.d+2,c.d+4);
      c.d[1]=c.d[0];
      c.d[3]=c.d[2];
      c.d[5]=c.d[4];
      c.d[6]=c.d[7]=255;
      break;
    case 8:
      sscanf(s,"%2hhx%2hhx%2hhx%2hhx",c.d+0,c.d+2,c.d+4,c.d+6);
      c.d[1]=c.d[0];
      c.d[3]=c.d[2];
      c.d[5]=c.d[4];
      c.d[6]=c.d[7];
      break;
    case 12:
      sscanf(s,"%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",c.d+0,c.d+1,c.d+2,c.d+3,c.d+4,c.d+5);
      c.d[6]=c.d[7]=255;
      break;
    case 16:
      sscanf(s,"%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",c.d+0,c.d+1,c.d+2,c.d+3,c.d+4,c.d+5,c.d+6,c.d+7);
      break;
    default:
      fprintf(stderr,"Invalid color format (%d)\n",(int)strlen(s));
      exit(1);
  }
  return c;
}

#define Match(a) ( opt[1] ? memcmp(p[a].d,borderc.d,chan) && memcmp(p[a].d,floodc.d,chan) : !memcmp(p[a].d,targetc.d,chan) )
static void do_flood(int x,int y,Pixel*p) {
  int r;
  p[r=x]=floodc;
  if(opt['r']>=0) {
    while(r<width-1 && Match(r+1)) p[++r]=floodc;
    if(opt['h'] && r==width-1 && memcmp(p->d,floodc.d,chan)) do_flood(0,y,p);
  }
  if(opt['r']<=0) {
    while(x && Match(x-1)) p[--x]=floodc;
    if(opt['h'] && !x && memcmp(p[width-1].d,floodc.d,chan)) do_flood(width-1,y,p);
  }
  if(opt['a']) {
    floodc.d[7]+=opt['a'];
    floodc.d[6]+=(opt['a']>>8)+(floodc.d[7]<(opt['a']&255)?1:0);
  }
  while(x<=r) {
    if(y) {
      p-=width;
      if(Match(x)) {
        do_flood(x,y-1,p);
      } else if(opt['d']) {
        if(x && Match(x-1)) do_flood(x-1,y-1,p);
        if(x!=width-1 && Match(x+1)) do_flood(x+1,y-1,p);
      }
      p+=width;
    } else if(opt['v']) {
      p=pic+width*(height-1);
      if(Match(x)) {
        do_flood(x,height-1,p);
      } else if(opt['d']) {
        if(x && Match(x-1)) do_flood(x-1,height-1,p);
        if(x!=width-1 && Match(x+1)) do_flood(x+1,height-1,p);
      }
      p=pic;
    }
    if(y!=height-1) {
      p+=width;
      if(Match(x)) {
        do_flood(x,y+1,p);
      } else if(opt['d']) {
        if(x && Match(x-1)) do_flood(x-1,y+1,p);
        if(x!=width-1 && Match(x+1)) do_flood(x+1,y+1,p);
      }
      p-=width;
    } else if(opt['v']) {
      p=pic;
      if(Match(x)) {
        do_flood(x,0,p);
      } else if(opt['d']) {
        if(x && Match(x-1)) do_flood(x-1,0,p);
        if(x!=width-1 && Match(x+1)) do_flood(x+1,0,p);
      }
      p=pic+width*(height-1);
    }
    x++;
  }
  if(opt['a']) {
    floodc.d[7]-=opt['a'];
    floodc.d[6]-=(opt['a']>>8)+(floodc.d[7]+(opt['a']&255)>255?1:0);
  }
}

int main(int argc,char**argv) {
  int i,j;
  if(argc<4) {
    fprintf(stderr,"Too few arguments\n");
    return 1;
  }
  floodx=strtol(argv[1],0,0);
  floody=strtol(argv[2],0,0);
  floodc=parse_color(argv[3]);
  i=4;
  if(argc>4 && argv[4][0]!='-' && argv[4][0] && argv[4][1]!='=') opt[1]=1,borderc=parse_color(argv[4]),i++;
  for(;i<argc;i++) {
    if(argv[i][0]=='-') {
      j=1;
      while(argv[i][j]) opt[argv[i][j++]]++;
    } else if(argv[i][0] && argv[i][1]=='=') {
      opt[argv[i][0]]=strtol(argv[i]+2,0,0);
    } else {
      fprintf(stderr,"Invalid option\n");
      return 1;
    }
  }
  fread(buf,1,16,stdin);
  width=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  height=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  if(floodx<0 || floodx>=width || floody<0 || floody>=height) {
    fprintf(stderr,"Flood coordinate out of range\n");
    return 1;
  }
  fwrite(buf,1,16,stdout);
  pic=malloc(width*height<<3);
  if(!pic) {
    fprintf(stderr,"Allocation failed\n");
    return 1;
  }
  fread(pic,width<<3,height,stdin);
  targetc=pic[floody*width+floodx];
  if(memcmp(targetc.d,floodc.d,chan=(opt['c']?:4)<<1)) do_flood(floodx,floody,pic+floody*width);
  fwrite(pic,width<<3,height,stdout);
  return 0;
}

