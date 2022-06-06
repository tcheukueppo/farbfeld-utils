#if 0
gcc -s -O2 -o ~/bin/ff-tensor -Wno-unused-result ff-tensor.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static int width1,height1,width2,height2;
static unsigned char*pic;
static unsigned char*row;
static FILE*in2;
static char opt[128];

static inline void process(const unsigned char*p,const unsigned char*r) {
  long long c;
  int i;
  for(i=0;i<4;i++) {
    c=(p[0]<<8)|p[1];
    c*=(r[0]<<8)|r[1];
    c/=65535;
    putchar(c>>8);
    putchar(c);
    p+=2;
    r+=2;
  }
}

int main(int argc,char**argv) {
  int x,y,xx,yy;
  if(argc<2 || !(in2=fopen(argv[1],"r"))) {
    fprintf(stderr,"Cannot open file\n");
    return 1;
  }
  if(argc>2) {
    for(x=0;argv[2][x];x++) opt[argv[2][x]&127]=1;
  }
  fread(buf,1,16,in2);
  width2=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  height2=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  fread(buf,1,16,stdin);
  width1=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  height1=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  if(!(pic=malloc(width2*height2*8)) || !(row=malloc(width1*8))) {
    fprintf(stderr,"Allocation failed\n");
    return 1;
  }
  if(opt['t']) {
    fwrite(buf,1,16,stdout);
    fread(pic,8,width2*height2,in2);
    fclose(in2);
    for(y=0;y<height1;y++) {
      fread(row,8,width1,stdin);
      yy=y%height2;
      for(x=0;x<width1;x++) {
        xx=x%width2;
        process(pic+(yy*width2+xx)*8,row+x*8);
      }
      if(opt['v']) memmove(pic,pic+width2*8,(height2-1)*width2*8);
    }
  } else {
    x=width1*width2;
    y=height1*height2;
    buf[8]=x>>24; buf[9]=x>>16; buf[10]=x>>8; buf[11]=x;
    buf[12]=y>>24; buf[13]=y>>16; buf[14]=y>>8; buf[15]=y;
    fwrite(buf,1,16,stdout);
    fread(pic,8,width2*height2,in2);
    fclose(in2);
    for(y=0;y<height1;y++) {
      fread(row,8,width1,stdin);
      for(yy=0;yy<height2;yy++) for(x=0;x<width1;x++) for(xx=0;xx<width2;xx++) {
        process(pic+(yy*width2+xx)*8,row+x*8);
      }
      if(opt['v']) memmove(pic,pic+width2*8,(height2-1)*width2*8);
    }
  }
  return 0;
}
