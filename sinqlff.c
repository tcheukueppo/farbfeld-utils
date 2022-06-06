#if 0
gcc -s -O2 -o ~/bin/sinqlff -Wno-unused-result sinqlff.c
exit
#endif

// Public domain Sinclair QL picture decoder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int form=4;

static inline void do_form_4(void) {
  int x,y,b,d0,d1;
  for(y=0;y<256;y++) for(x=0;x<512;x++) {
    b=7&~x;
    if(b==7) d0=fgetc(stdin),d1=fgetc(stdin);
    if((d1>>b)&1) putchar(255),putchar(255); else putchar(0),putchar(0);
    if((d0>>b)&1) putchar(255),putchar(255); else putchar(0),putchar(0);
    putchar(0),putchar(0); putchar(255),putchar(255);
  }
}

static inline void do_form_8(void) {
  int x,y,b,d0,d1;
  for(y=0;y<256;y++) for(x=0;x<256;x++) {
    b=(3&~x)<<1;
    if(b==6) d0=fgetc(stdin),d1=fgetc(stdin);
    if((d1>>b)&1) putchar(255),putchar(255); else putchar(0),putchar(0);
    if((d0>>b)&1) putchar(255),putchar(255); else putchar(0),putchar(0);
    if((d1>>b)&2) putchar(255),putchar(255); else putchar(0),putchar(0);
    putchar(255),putchar(255);
  }
}

int main(int argc,char**argv) {
  if(argc>1) form=strtol(argv[1],0,0);
  if(form!=4 && form!=8) {
    fprintf(stderr,"Invalid picture mode; must be 4 or 8\n");
    return 1;
  }
  fwrite("farbfeld\0",1,10,stdout);
  putchar(form==4?2:1);
  fwrite("\0\0\0\1",1,5,stdout);
  if(form==4) do_form_4();
  if(form==8) do_form_8();
  return 0;
}
