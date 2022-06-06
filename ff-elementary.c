#if 0
gcc -s -O2 -o ~/bin/ff-elementary -Wno-unused-result ff-elementary.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static unsigned char rule[3];
static unsigned char*rowin;
static unsigned char*rowout;
static int width;

int main(int argc,char**argv) {
  int i,j;
  if(argc==2) {
    rule[0]=rule[1]=rule[2]=strtol(argv[1],0,0);
  } else if(argc==4) {
    rule[0]=strtol(argv[1],0,0);
    rule[1]=strtol(argv[2],0,0);
    rule[2]=strtol(argv[3],0,0);
  } else {
    fprintf(stderr,"Improper number of arguments\n");
    return 1;
  }
  fread(buf,1,16,stdin);
  width=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  if(!(rowout=malloc(width<<1))) {
    fprintf(stderr,"Allocation failed\n");
    return 1;
  }
  rowin=rowout+width;
  fwrite(buf,1,16,stdout);
  for(i=0;i<width;i++) {
    fread(buf,1,8,stdin);
    rowout[i]=((buf[6]>>4)&8)|((buf[0]>>5)&4)|((buf[2]>>6)&2)|((buf[4]>>7)&1);
  }
  for(;;) {
    for(i=0;i<width;i++) {
      j=rowin[i]=rowout[i];
      putchar(j&4?255:0); putchar(j&4?255:0);
      putchar(j&2?255:0); putchar(j&2?255:0);
      putchar(j&1?255:0); putchar(j&1?255:0);
      putchar(j&8?255:0); putchar(j&8?255:0);
    }
    for(i=0;i<width;i++) {
      if(fread(buf,1,8,stdin)<=0) return 0;
      if(buf[6]&128) {
        rowout[i]=8;
        j=(rowin[(i+1)%width]&1)|(rowin[i]&2)|(rowin[(i+width-1)%width]&4);
        if(rule[0]&(1<<j)) rowout[i]|=4;
        if(rule[1]&(1<<j)) rowout[i]|=2;
        if(rule[2]&(1<<j)) rowout[i]|=1;
      } else {
        rowout[i]=((buf[0]>>5)&4)|((buf[2]>>6)&2)|((buf[4]>>7)&1);
      }
    }
  }
}

