#if 0
gcc -s -O2 -o ~/bin/ff-frhs -Wno-unused-result ff-frhs.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>

static unsigned char buf[16];
static char mode; // c i l v

int main(int argc,char**argv) {
  if(argc!=2 || !argv[1][0] || argv[1][1]) {
    fprintf(stderr,"Incorrect arguments\n");
    return 1;
  }
  mode=argv[1][0];
  fread(buf,1,16,stdin);
  fwrite(buf,1,16,stdout);
  while(fread(buf,1,8,stdin)) {
    int h=(buf[0]<<8)|buf[1];
    int s=(buf[2]<<8)|buf[3];
    long long v=(buf[4]<<8)|buf[5];
    int Z=((h&0x1FFF)<<3)^(h&0x2000?65535:0);
    int C=(mode=='v'?v*s:mode=='l'?(v&32768?65535-v:v)*s:mode=='i'?(196605*v*s)/(Z+65535):v*65535)/65535;
    int X=(C*Z)/65535;
    int m=(mode=='v'?v-C:mode=='l'?v-C/2:mode=='i'?(v*(65535-s))/65535:s);
    switch(h>>13) {
      case 0: buf[0]=(C+m)>>8; buf[1]=C+m; buf[2]=(X+m)>>8; buf[3]=X+m; buf[4]=m>>8; buf[5]=m; break;
      case 1: buf[2]=(C+m)>>8; buf[3]=C+m; buf[0]=(X+m)>>8; buf[1]=X+m; buf[4]=m>>8; buf[5]=m; break;
      case 2: buf[2]=(C+m)>>8; buf[3]=C+m; buf[4]=(X+m)>>8; buf[5]=X+m; buf[0]=m>>8; buf[1]=m; break;
      case 3: buf[4]=(C+m)>>8; buf[5]=C+m; buf[2]=(X+m)>>8; buf[3]=X+m; buf[0]=m>>8; buf[1]=m; break;
      case 4: buf[4]=(C+m)>>8; buf[5]=C+m; buf[0]=(X+m)>>8; buf[1]=X+m; buf[2]=m>>8; buf[3]=m; break;
      case 5: buf[0]=(C+m)>>8; buf[1]=C+m; buf[4]=(X+m)>>8; buf[5]=X+m; buf[2]=m>>8; buf[3]=m; break;
    }
    fwrite(buf,1,8,stdout);
  }
  return 0;
}
