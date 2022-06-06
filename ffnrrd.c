#if 0
gcc -s -O2 -o ~/bin/ffnrrd -Wno-unused-result ffnrrd.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static int width,height;

int main(int argc,char**argv) {
  fread(buf,1,16,stdin);
  if(memcmp(buf,"farbfeld",8)) {
    fprintf(stderr,"Not farbfeld\n");
    return 1;
  }
  width=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  height=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  printf("NRRD0003\n");
  printf("type: ushort\n");
  printf("endian: big\n");
  printf("dimension: 3\n");
  printf("sizes: 4 %d %d\n",width,height);
  printf("kinds: RGBA-color space space\n");
  printf("encoding: raw\n");
  putchar('\n');
  while(fread(buf,1,8,stdin)>0) fwrite(buf,1,8,stdout);
  return 0;
}

// I am not sure if the above is a correct use of NRRD format; it is untested
