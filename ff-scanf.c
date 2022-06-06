#if 0
gcc -s -O2 -o ~/bin/ff-scanf -Wno-unused-result ff-scanf.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[8];
static int count,width,height;

int main(int argc,char**argv) {
  int r,g,b,a;
  char*form=argc>3?argv[3]:"%d %d %d %d\n";
  if(argc<3) {
    fprintf(stderr,"Too few arguments\n");
    return 1;
  }
  width=strtol(argv[1],0,0);
  height=strtol(argv[2],0,0);
  fwrite("farbfeld",1,8,stdout);
  buf[0]=width>>24;
  buf[1]=width>>16;
  buf[2]=width>>8;
  buf[3]=width;
  buf[4]=height>>24;
  buf[5]=height>>16;
  buf[6]=height>>8;
  buf[7]=height;
  fwrite(buf,1,8,stdout);
  count=width*height;
  r=g=b=0;
  a=65535;
  if(argc>4 && argv[4][0]=='+') {
    while(count--) {
      scanf(form,&r,&g,&b,&a);
      buf[0]=buf[1]=r;
      buf[2]=buf[3]=g;
      buf[4]=buf[5]=b;
      buf[6]=buf[7]=a;
      fwrite(buf,1,8,stdout);
    }
  } else {
    while(count--) {
      scanf(form,&r,&g,&b,&a);
      buf[0]=r>>8;
      buf[1]=r;
      buf[2]=g>>8;
      buf[3]=g;
      buf[4]=b>>8;
      buf[5]=b;
      buf[6]=a>>8;
      buf[7]=a;
      fwrite(buf,1,8,stdout);
    }
  }
  return 0;
}
