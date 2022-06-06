#if 0
gcc -s -O2 -o ~/bin/bldff -Wno-unused-result bldff.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int width,height,comp;

static void do_byte(int c) {
  static const char s[]="\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\0\0\0\0\0\0\xFF\xFF";
  static int x=0;
  int n=x<width-8?8:width-x;
  if(n>=width-8) x=0;
  while(n--) {
    fwrite(s+(c&128?8:0),1,8,stdout);
    c<<=1;
  }
}

int main(int argc,char**argv) {
  int c,n;
  width=fgetc(stdin)<<8;
  width|=fgetc(stdin);
  height=fgetc(stdin)<<8;
  height|=fgetc(stdin);
  if(width&0x8000) width=0x10000-width,comp=1;
  ++width;
  ++height;
  fwrite("farbfeld\0",1,10,stdout);
  putchar(width>>8);
  putchar(width);
  putchar(0);
  putchar(0);
  putchar(height>>8);
  putchar(height);
  while(height && (c=fgetc(stdin))!=EOF) {
    do_byte(c);
    if(comp && (c==0 || c==255)) {
      n=fgetc(stdin);
      if(n>0) while(n--) do_byte(c);
    }
  }
  return 0;
}
