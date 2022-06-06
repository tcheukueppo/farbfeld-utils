#if 0
gcc -s -O2 -o ~/bin/ff-lowpass -Wno-unused-result ff-lowpass.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int width,height;
static unsigned char buf[16];
static double real[4];
static double imag[4];
static double p1,p2,p3;
static char d1,d2,d3;

static void process(int x,int y) {
  double v=(buf[y+y]<<8)|buf[y+y+1];
  double w=0.0;
  if(d1) p1=((buf[6]<<8)|buf[7])/65535.0;
  if(d2) p2=((buf[6]<<8)|buf[7])/65535.0;
  if(d3) p3=((buf[6]<<8)|buf[7])-32767.25;
  if(x) {
    w=p1*imag[y]+p2*(real[y]-v);
    v+=p1*real[y]-p1*v-p2*imag[y];
  }
  real[y]=v;
  imag[y]=w;
  v+=p3;
  y=v<0.1?0:v>=65535.0?65535:v;
  putchar(y>>8);
  putchar(y);
}

int main(int argc,char**argv) {
  int x;
  p1=argc>1?strtod(argv[1],0):0.5;
  p2=argc>2?strtod(argv[2],0):0.0;
  p3=argc>3?strtod(argv[3],0):0.5;
  if(argc>1 && argv[1][0]=='/') d1=1;
  if(argc>2 && argv[2][0]=='/') d2=1;
  if(argc>3 && argv[3][0]=='/') d3=1;
  fread(buf,1,16,stdin);
  fwrite(buf,1,16,stdout);
  width=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  height=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  while(height--) {
    for(x=0;x<width;x++) {
      fread(buf,1,8,stdin);
      process(x,0);
      process(x,1);
      process(x,2);
      process(x,3);
    }
  }
  return 0;
}
