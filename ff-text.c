#if 0
gcc -s -O2 -o ~/bin/ff-text -Wno-unused-result ff-text.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static unsigned char*font;
static int cwidth,cheight,iwidth,iheight,owidth,oheight,ccount,icount,ocount,ismap;
static short map[256];

static void process(const char*s) {
  int n,h,c;
  const char*p;
  for(h=0;h<cheight;h++) {
    p=s;
    c=ocount;
    while(*p) {
      c--;
      n=*p++&255;
      if(ismap) {
        n=map[n];
        if(n==-1) {
          n=cwidth;
          while(n--) putchar(0);
          continue;
        }
      }
      fwrite(font+(n%icount)*cwidth+(h+(n/icount)*cheight)*iwidth,1,cwidth,stdout);
    }
    c*=cwidth;
    while(c--) putchar(0);
  }
}

int main(int argc,char**argv) {
  int i;
  if(argc<3) {
    fprintf(stderr,"Too few arguments\n");
    return 0;
  }
  if(argv[1][0]=='+') {
    argv++;
    ismap=1;
    for(i=0;i<256;i++) map[i]=-1;
    for(i=1;argv[0][i];i++) map[argv[0][i]&255]=i-1;
    if(--argc<3) {
      fprintf(stderr,"Too few arguments\n");
      return 0;
    }
  }
  fread(buf,1,16,stdin);
  iwidth=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  iheight=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  font=malloc(iwidth*iheight*8);
  if(!font) {
    fprintf(stderr,"Allocation failed\n");
    return 0;
  }
  fread(font,8,iwidth*iheight,stdin);
  cwidth=strtol(argv[1],0,0);
  cheight=strtol(argv[2],0,0);
  if(!cwidth || !cheight || iwidth%cwidth || iheight%cheight) {
    fprintf(stderr,"Improper tile size\n");
    return 0;
  }
  ocount=0;
  oheight=(argc-3)*cheight;
  for(i=3;i<argc;i++) if(ocount<strlen(argv[i])) ocount=strlen(argv[i]);
  owidth=ocount*cwidth;
  buf[8]=owidth>>24;
  buf[9]=owidth>>16;
  buf[10]=owidth>>8;
  buf[11]=owidth>>0;
  buf[12]=oheight>>24;
  buf[13]=oheight>>16;
  buf[14]=oheight>>8;
  buf[15]=oheight>>0;
  fwrite(buf,1,16,stdout);
  icount=iwidth/cwidth;
  iwidth<<=3;
  cwidth<<=3;
  for(i=3;i<argc;i++) process(argv[i]);
  return 0;
}
