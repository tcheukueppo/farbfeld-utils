#if 0
gcc -s -O2 -o ~/bin/ff-maxrgb -Wno-unused-result ff-maxrgb.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static int mask,value;

static inline void process(void) {
  int i,j,y;
  unsigned short x=mask&16?65535:0;
  for(i=0,j=1;i<4;i++,j+=j) if(mask&j) {
    y=(buf[i+i]<<8)|buf[i+i+1];
    if(mask&16?(x>y):(x<y)) x=y;
  }
  for(i=0,j=1;i<4;i++,j+=j) if(mask&j) {
    y=(buf[i+i]<<8)|buf[i+i+1];
    if(x!=y) {
      buf[i+i]=value>>8;
      buf[i+i+1]=value;
    }
  }
  fwrite(buf,1,8,stdout);
}

int main(int argc,char**argv) {
  int i;
  if(argc>1) {
    for(i=0;argv[1][i];i++) {
      if(argv[1][i]=='r') mask|=1;
      if(argv[1][i]=='g') mask|=2;
      if(argv[1][i]=='b') mask|=4;
      if(argv[1][i]=='a') mask|=8;
      if(argv[1][i]=='m') mask|=16;
    }
  } else {
    mask=7;
  }
  if(argc>2) value=strtol(argv[2],0,0);
  fread(buf,1,16,stdin);
  fwrite(buf,1,16,stdout);
  while(fread(buf,1,8,stdin)) process();
  return 0;
}

