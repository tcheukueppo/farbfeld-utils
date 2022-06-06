#if 0
gcc -s -O2 -o ~/bin/ff-back ff-back.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static double alpha;
static int param[3];

static void process(int pos) {
  int v=(buf[pos]<<8)|buf[pos+1];
  v=alpha*v+(1.0-alpha)*param[pos>>1];
  buf[pos]=v>>8;
  buf[pos+1]=v;
}

int main(int argc,char**argv) {
  int i;
  i=fread(buf,1,16,stdin);
  i=fwrite(buf,1,16,stdout);
  if(argc==4) {
    param[0]=strtol(argv[1],0,0);
    param[1]=strtol(argv[2],0,0);
    param[2]=strtol(argv[3],0,0);
  } else if(argc==2 && strlen(argv[1])==6) {
    sscanf(argv[1],"%2X%2X%2X",param+0,param+1,param+2);
    param[0]*=0x101;
    param[1]*=0x101;
    param[2]*=0x101;
  } else if(argc==2 && strlen(argv[1])==12) {
    sscanf(argv[1],"%4X%4X%4X",param+0,param+1,param+2);
  } else {
    fprintf(stderr,"Incorrect number of arguments\n");
    return 1;
  }
  while(fread(buf,1,8,stdin)) {
    alpha=((double)((buf[6]<<8)|buf[7]))/65535.0;
    process(0);
    process(2);
    process(4);
    buf[6]=buf[7]=255;
    i=fwrite(buf,1,8,stdout);
  }
  return 0;
}

