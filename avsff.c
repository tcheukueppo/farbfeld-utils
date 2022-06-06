#if 0
gcc -s -O2 -o ~/bin/avsff -Wno-unused-result avsff.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[8];

int main(int argc,char**argv) {
  fread(buf,1,8,stdin);
  fwrite("farbfeld",1,8,stdout);
  fwrite(buf,1,8,stdout);
  while(fread(buf,1,4,stdin)>0) {
    putchar(buf[1]); putchar(buf[1]);
    putchar(buf[2]); putchar(buf[2]);
    putchar(buf[3]); putchar(buf[3]);
    putchar(buf[0]); putchar(buf[0]); // is 0 or 255 opaque?
  }
  return 0;
}
