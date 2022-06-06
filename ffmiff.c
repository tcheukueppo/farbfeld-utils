#if 0
gcc -s -O2 -o ~/bin/ffmiff -Wno-unused-result ffmiff.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lodepng.h"

static unsigned char buf[16];
static int width,height;

int main(int argc,char**argv) {
  int n;
  fread(buf,1,16,stdin);
  if(memcmp("farbfeld",buf,8)) {
    fprintf(stderr,"Not farbfeld\n");
    return 1;
  }
  width=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  height=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  printf("id=ImageMagick\ndepth=16\nmatte=True\ncolumns=%d\nrows=%d\n\f\n:\x1A",width,height);
  n=width*height;
  while(n--) fread(buf,1,8,stdin),fwrite(buf,1,8,stdout);
  return 0;
}
