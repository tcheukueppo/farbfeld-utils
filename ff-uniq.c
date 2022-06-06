#if 0
gcc -s -O2 -o ~/bin/ff-uniq -Wno-unused-result ff-uniq.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static int width,totalheight,tileheight,ntiles,tilesize,otiles;
static unsigned char*pic;
static unsigned char*mark;

static void process(void) {
  int i,j;
  unsigned char*p;
  unsigned char*q;
  otiles=ntiles;
  for(i=0,p=pic;i<ntiles;i++,p+=tilesize) if(!mark[i]) {
    for(j=i+1,q=p+tilesize;j<ntiles;j++,q+=tilesize) {
      if(!memcmp(p,q,tilesize)) mark[j]=1,otiles--;
    }
  }
}

static void output(void) {
  int i;
  unsigned char*p;
  for(i=0,p=pic;i<ntiles;i++,p+=tilesize) if(!mark[i]) fwrite(p,1,tilesize,stdout);
}

int main(int argc,char**argv) {
  if(argc!=2) {
    fprintf(stderr,"Incorrect number of arguments\n");
    return 1;
  }
  tileheight=strtol(argv[1],0,0);
  fread(buf,1,16,stdin);
  width=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  totalheight=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  if(totalheight%tileheight) {
    fprintf(stderr,"Incorrect tile height\n");
    return 1;
  }
  ntiles=totalheight/tileheight;
  tilesize=width*tileheight<<3;
  mark=malloc(ntiles);
  pic=malloc(tilesize*ntiles);
  if(!mark || !pic) {
    fprintf(stderr,"Allocation failed\n");
    return 1;
  }
  memset(mark,0,ntiles);
  fwrite(buf,1,12,stdout);
  fread(pic,tilesize,ntiles,stdin);
  process();
  totalheight=otiles*tileheight;
  putchar(totalheight>>24);
  putchar(totalheight>>16);
  putchar(totalheight>>8);
  putchar(totalheight);
  output();
  return 0;
}

