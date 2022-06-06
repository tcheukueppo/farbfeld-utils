#if 0
gcc -s -O2 -o ~/bin/ff-idclut -Wno-unused-result ff-idclut.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char head[16]={'f','a','r','b','f','e','l','d',0,0,16,0,0,0,16,0};

int main(int argc,char**argv) {
  int r,g,b;
  fwrite(head,1,16,stdout);
  for(b=0;b<256;b++) for(g=0;g<256;g++) for(r=0;r<256;r++) {
    putchar(r); putchar(r);
    putchar(g); putchar(g);
    putchar(b); putchar(b);
    putchar(255); putchar(255);
  }
  return 0;
}

