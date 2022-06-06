#if 0
gcc -s -O2 -o ~/bin/ff-ycocg -Wno-unused-result ff-ycocg.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static unsigned int total,bias;
static char opt[128];

// Implements "YCoCg24" (or in this case, 48)

static void do_encode(void) {
  int R=(buf[0]<<8)|buf[1];
  int G=(buf[2]<<8)|buf[3];
  int B=(buf[4]<<8)|buf[5];
  // (R,B) -> (t,Co)
  int Co=(B-R)&0xFFFF;
  int t=(R+(Co>>1)+(Co&0x8000))&0xFFFF;
  // (G,t) -> (Y,Cg)
  int Cg=(t-G)&0xFFFF;
  int Y=(G+(Cg>>1)+(Cg&0x8000))&0xFFFF;
  // out
  buf[0]=Y>>8; buf[1]=Y;
  buf[2]=(Co>>8)+bias; buf[3]=Co;
  buf[4]=(Cg>>8)+bias; buf[5]=Cg;
}

static void do_decode(void) {
  int Y=(buf[0]<<8)|buf[1];
  int Co=(((buf[2]^bias)<<8)|buf[3])-0x8000;
  int Cg=(((buf[4]^bias)<<8)|buf[5])-0x8000;
  // (Y,Cg) -> (G,t)
  int G=(Y-(Cg>>1))&0xFFFF;
  int t=(G+Cg)&0xFFFF;
  // (t,Co) -> (R,B)
  int R=(t-(Co>>1))&0xFFFF;
  int B=(R+Co)&0xFFFF;
  // out
  buf[0]=R>>8; buf[1]=R;
  buf[2]=G>>8; buf[3]=G;
  buf[4]=B>>8; buf[5]=B;
}

static void do_encode8(void) {
  int R=buf[0];
  int G=buf[2];
  int B=buf[4];
  // (R,B) -> (t,Co)
  int Co=(B-R)&0xFF;
  int t=(R+(Co>>1)+(Co&0x80))&0xFF;
  // (G,t) -> (Y,Cg)
  int Cg=(t-G)&0xFF;
  int Y=(G+(Cg>>1)+(Cg&0x80))&0xFF;
  // out
  buf[0]=buf[1]=Y;
  buf[2]=buf[3]=Co^bias;
  buf[4]=buf[5]=Cg^bias;
}

static void do_decode8(void) {
  int Y=buf[0];
  int Co=(buf[2]^bias)-0x80;
  int Cg=(buf[4]^bias)-0x80;
  // (Y,Cg) -> (G,t)
  int G=(Y-(Cg>>1))&0xFF;
  int t=(G+Cg)&0xFF;
  // (t,Co) -> (R,B)
  int R=(t-(Co>>1))&0xFF;
  int B=(R+Co)&0xFF;
  // out
  buf[0]=buf[1]=R;
  buf[2]=buf[3]=G;
  buf[4]=buf[5]=B;
}

#define PROCESS(x) do{ while(total--) fread(buf,1,8,stdin),x(),fwrite(buf,1,8,stdout); }while(0)
int main(int argc,char**argv) {
  int i;
  if(argc>1) for(i=0;argv[1][i];i++) opt[argv[1][i]&127]=1;
  fread(buf,1,16,stdin);
  fwrite(buf,1,16,stdout);
  total=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  total*=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  if(opt['b']^opt['d']) bias=128;
  if(opt['8']) {
    if(opt['d']) PROCESS(do_decode8); else PROCESS(do_encode8);
  } else {
    if(opt['d']) PROCESS(do_decode); else PROCESS(do_encode);
  }
  return 0;
}
