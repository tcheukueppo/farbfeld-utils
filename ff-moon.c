#if 0
gcc -s -O2 -o ~/bin/ff-moon -Wno-unused-result ff-moon.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static double arg[4];

int main(int argc,char**argv) {
  if(argc!=5) {
    fprintf(stderr,"Too %s arguments\n",argc<5?"few":"many");
    return 1;
  }
  arg[0]=strtod(argv[1],0);
  arg[1]=strtod(argv[2],0);
  arg[2]=strtod(argv[3],0);
  arg[3]=strtod(argv[4],0);
  fread(buf,1,16,stdin);
  fwrite(buf,1,16,stdout);
  while(fread(buf,1,8,stdin)) {
    int r=(buf[0]<<8)|buf[1];
    int g=(buf[2]<<8)|buf[3];
    int b=(buf[4]<<8)|buf[5];
    double A=((buf[6]<<8)|buf[7])/65535.0;
    int M=r>g?r>b?r:b:b>g?b:g;
    int m=r<g?r<b?r:b:b<g?b:g;
    r=A*arg[3]*(arg[3]*(r+arg[0]*m)+arg[0]*M)+(1.0-A)*r;
    g=A*arg[3]*(arg[3]*(g+arg[1]*m)+arg[1]*M)+(1.0-A)*g;
    b=A*arg[3]*(arg[3]*(b+arg[2]*m)+arg[2]*M)+(1.0-A)*b;
    if(r<0) r=0; else if(r>65535) r=65535;
    if(g<0) g=0; else if(g>65535) g=65535;
    if(b<0) b=0; else if(b>65535) b=65535;
    buf[0]=r>>8; buf[1]=r;
    buf[2]=g>>8; buf[3]=g;
    buf[4]=b>>8; buf[5]=b;
    fwrite(buf,1,8,stdout);
  }
  return 0;
}

