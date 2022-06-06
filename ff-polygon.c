#if 0
gcc -s -O2 -o ~/bin/ff-polygon -Wno-unused-result ff-polygon.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int x,y;
} Point;

static int width,height,nver;
static unsigned char buf[8];
static unsigned char color[8];
static Point*vertex;
static char opt[128];

static inline void get_color(const char*txt) {
  int i=strlen(txt);
  if(i==6) {
    sscanf(txt,"%2hhX%2hhX%2hhX",color+0,color+2,color+4);
    color[1]=color[0];
    color[3]=color[2];
    color[5]=color[4];
    color[7]=color[6]=255;
  } else if(i==8) {
    sscanf(txt,"%2hhX%2hhX%2hhX%2hhX",color+0,color+2,color+4,color+6);
    color[1]=color[0];
    color[3]=color[2];
    color[5]=color[4];
    color[7]=color[6];
  } else if(i==12) {
    sscanf(txt,"%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX",color+0,color+1,color+2,color+3,color+4,color+5);
    color[7]=color[6]=255;
  } else if(i==16) {
    sscanf(txt,"%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX",color+0,color+1,color+2,color+3,color+4,color+5,color+6,color+7);
  } else if(i!=1 || *txt!='-') {
    fprintf(stderr,"Invalid color format\n");
    exit(1);
  }
}

static inline void get_vertex(char**argv,int n) {
  nver=n;
  while(n--) {
    vertex[n].x=strtol(*argv++,0,0);
    vertex[n].y=strtol(*argv++,0,0);
  }
  vertex[nver]=*vertex;
}

static inline void process(void) {
  int x,y,i,n;
  float u,v;
  u=0.0;
  for(y=0;y<height;y++) for(x=0;x<width;x++) {
    fread(buf,1,8,stdin);
    if(opt['u']) u=width*(((buf[6]<<8)|buf[7])/65535.0-0.5);
    for(i=n=0;i<nver;i++) {
      if((vertex[i].y<=y && vertex[i+1].y>y) || (vertex[i].y>y && vertex[i+1].y<=y)) {
        v=(float)(y-vertex[i].y)/(vertex[i+1].y-vertex[i].y);
        if(opt['v']) v*=v;
        if(x+u<vertex[i].x+v*(vertex[i+1].x-vertex[i].x)) n^=1;
      }
    }
    if(n && opt['+']) {
      if(opt['t']) memcpy(color,buf,6);
      if(opt['a']) memcpy(color+6,buf+6,2);
      if(opt['i']) {
        v=1.0;
        for(i=0;i<nver;i++) {
          n=(x-vertex[i].x)*(x-vertex[i].x)+(y-vertex[i].y)*(y-vertex[i].y);
          if(n>v) v=n;
        }
        for(i=0;i<3;i++) {
          if(color[i+i] || color[i+i+1]) {
            n=((buf[i+i]<<8)|buf[i+i+1])*(double)((color[i+i]<<8)|color[i+i+1])/65535.0;
            if(opt['j'] && ((x^y)&1)) i=(i+1)%3;
            n=(n/v+x-vertex[i].x)*(n/v+x-vertex[i].x)+(y-vertex[i].y)*(y-vertex[i].y);
          } else {
            if(opt['j'] && ((x^y)&1)) i=(i+1)%3;
            n=(x-vertex[i].x)*(x-vertex[i].x)+(y-vertex[i].y)*(y-vertex[i].y);
          }
          if(opt['j'] && ((x^y)&1)) i=(i+2)%3;
          n=(n*65535.0)/v; n=n>65535?65535:n<0?0:n;
          buf[i+i]=n>>8; buf[i+i+1]=n;
        }
        buf[6]=color[6]; buf[7]=color[7]; n=0;
      }
    }
    fwrite(n?color:buf,1,8,stdout);
  }
}

int main(int argc,char**argv) {
  if(argc>2 && argv[1][0]=='+') {
    int i;
    for(i=0;argv[1][i];i++) opt[argv[1][i]&127]=1;
    argc--;
    argv++;
  }
  if(argc<8 || (argc&1)) {
    fprintf(stderr,"Improper number of arguments\n");
    return 1;
  }
  get_color(argv[1]);
  vertex=malloc((argc>>1)*sizeof(Point));
  if(!vertex) {
    fprintf(stderr,"Allocation failed\n");
    return 1;
  }
  get_vertex(argv+2,(argc-2)>>1);
  fread(buf,1,8,stdin);
  fwrite(buf,1,8,stdout);
  fread(buf,1,8,stdin);
  width=(buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
  height=(buf[4]<<24)|(buf[5]<<16)|(buf[6]<<8)|buf[7];
  fwrite(buf,1,8,stdout);
  process();
  return 0;
}
