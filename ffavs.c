#if 0
gcc -s -O2 -o ~/bin/ffavs -Wno-unused-result ffavs.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[8];

int main(int argc,char**argv) {
  fread(buf,1,8,stdin);
  if(memcmp(buf,"farbfeld",8)) {
    fprintf(stderr,"Not farbfeld\n");
    return 1;
  }
  fread(buf,1,8,stdin);
  fwrite(buf,1,8,stdout);
  while(fread(buf,1,8,stdin)>0) {
    putchar(buf[6]);
    putchar(buf[0]);
    putchar(buf[2]);
    putchar(buf[4]);
  }
  return 0;
}
