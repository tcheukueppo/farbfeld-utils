#if 0
gcc -s -O2 -o ~/bin/ff-swizzle -Wno-unused-result ff-swizzle.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];

int main(int argc,char**argv) {
  int i;
  if(argc<2) {
    fprintf(stderr,"Too few arguments\n");
    return 1;
  }
  i=strlen(argv[1]);
  if(!i || i==3 || (i>4 && (i&3))) {
    fprintf(stderr,"Improper swizzle\n");
    return 1;
  }
  fread(buf,1,16,stdin);
  i=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  i=(i*strlen(argv[1]))>>2;
  buf[8]=i>>24;
  buf[9]=i>>16;
  buf[10]=i>>8;
  buf[11]=i;
  fwrite(buf,1,16,stdout);
  while(fread(buf,1,8,stdin)) {
    for(i=0;argv[1][i];i++) switch(argv[1][i]) {
      case '0': putchar(0); putchar(0); break;
      case '1': putchar(255); putchar(255); break;
      case 'r': putchar(buf[0]); putchar(buf[1]); break;
      case 'g': putchar(buf[2]); putchar(buf[3]); break;
      case 'b': putchar(buf[4]); putchar(buf[5]); break;
      case 'a': putchar(buf[6]); putchar(buf[7]); break;
      case 'R': putchar(~buf[0]); putchar(~buf[1]); break;
      case 'G': putchar(~buf[2]); putchar(~buf[3]); break;
      case 'B': putchar(~buf[4]); putchar(~buf[5]); break;
      case 'A': putchar(~buf[6]); putchar(~buf[7]); break;
      case '_': default: putchar(128); putchar(0); break;
    }
  }
  return 0;
}

