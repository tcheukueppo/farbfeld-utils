#if 0
gcc -s -O2 -o ~/bin/ff-thresh -Wno-unused-result ff-thresh.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static int thr[4];

int main(int argc,char**argv) {
  if(argc==2) {
    thr[0]=thr[1]=thr[2]=thr[3]=strtol(argv[1],0,0);
  } else if(argc==5) {
    thr[0]=strtol(argv[1],0,0);
    thr[1]=strtol(argv[2],0,0);
    thr[2]=strtol(argv[3],0,0);
    thr[3]=strtol(argv[4],0,0);
  } else {
    fprintf(stderr,"Wrong number of arguments\n");
    return 1;
  }
  fread(buf,1,16,stdin);
  fwrite(buf,1,16,stdout);
  while(fread(buf,1,8,stdin)>0) {
    buf[0]=buf[1]=((buf[0]<<8)|buf[1])<thr[0]?0:255;
    buf[2]=buf[3]=((buf[2]<<8)|buf[3])<thr[1]?0:255;
    buf[4]=buf[5]=((buf[4]<<8)|buf[5])<thr[2]?0:255;
    buf[6]=buf[7]=((buf[6]<<8)|buf[7])<thr[3]?0:255;
    fwrite(buf,1,8,stdout);
  }
  return 0;
}

