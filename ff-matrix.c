#if 0
gcc -s -O2 -o ~/bin/ff-matrix ff-matrix.c -lm
exit
#endif

#define _BSD_SOURCE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static double param[20];
static double pixel[4];

static inline void inconv() {
  int i,v;
  for(i=0;i<4;i++) {
    v=(buf[i<<1]<<8)|buf[(i<<1)+1];
    pixel[i]=((double)v)/65535.0;
  }
}

static inline void process() {
  int i;
  double p[4];
  for(i=0;i<4;i++) p[i]=pixel[0]*param[i]+pixel[1]*param[i+4]+pixel[2]*param[i+8]+pixel[3]*param[i+12]+param[i+16];
  for(i=0;i<4;i++) pixel[i]=p[i];
}

static inline void outconv() {
  double v;
  int i,r;
  for(i=0;i<4;i++) {
    v=pixel[i]*65535.0;
    r=0.0<v?v<65535.0?(int)rint(v):65535:0;
    if(v>65535) v=65535;
    buf[i<<1]=r>>8;
    buf[(i<<1)+1]=r;
  }
}

int main(int argc,char**argv) {
  int i;
  i=fread(buf,1,16,stdin);
  i=fwrite(buf,1,16,stdout);
  for(i=0;i<20;i++) if(argc>i+1) param[i]=strtod(argv[i+1],0);
  while(fread(buf,1,8,stdin)) {
    inconv();
    process();
    outconv();
    i=fwrite(buf,1,8,stdout);
  }
  return 0;
}

