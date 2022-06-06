#if 0
gcc -s -O2 -o ~/bin/ff-quantile -Wno-unused-result ff-quantile.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define fatal(x) do { fprintf(stderr,x "\n"); exit(1); } while(0)

typedef struct {
  unsigned short d[4];
} Color;

static unsigned char buf[16];
static int width,height;
static int awidth,aheight,quant,option;
static int rowsize,listsize,rowpos;
static Color*row;
static Color*list;

static Color*getpixel(void) {
  Color*p=row+rowpos++;
  if(rowpos==rowsize) rowpos=0;
  fread(buf,1,8,stdin);
  p->d[0]=(buf[0]<<8)|buf[1];
  p->d[1]=(buf[2]<<8)|buf[3];
  p->d[2]=(buf[4]<<8)|buf[5];
  p->d[3]=(buf[6]<<8)|buf[7];
  return p;
}

static int comparer(const void*a,const void*b) {
  // Should we figure quantiles individually per channel instead?
  // If so, maybe it should be an option.
  const Color*x=a;
  const Color*y=b;
  return x->d[0]-y->d[0]+x->d[1]-y->d[1]+x->d[2]-y->d[2];
}

static inline void do_row(void) {
  // I think it may be too slow for now; later we must make a fast one!
  int i,x,y;
  int r=rowpos;
  Color*p;
  for(i=1;i<awidth;i++) getpixel();
  for(i=0;i<=width-awidth;i++) {
    getpixel();
    p=list;
    for(y=0;y<aheight;y++) for(x=0;x<awidth;x++) *p++=row[(y*width+x+r+i)%rowsize];
    qsort(list,listsize,sizeof(Color),comparer);
    p=list+quant;
    for(x=0;x<4;x++) buf[x<<1]=p->d[x]>>8,buf[(x<<1)+1]=p->d[x];
    fwrite(buf,1,8,stdout);
    if(option&1) row[(r+i)%rowsize]=*p;
    if(option&2) quant=(quant+1)%(awidth*aheight);
  }
  if(option&4) quant=(quant+1)%(awidth*aheight);
}

int main(int argc,char**argv) {
  if(argc<4) fatal("Not enough arguments");
  awidth=strtol(argv[1],0,0);
  aheight=strtol(argv[2],0,0);
  quant=strtol(argv[3],0,0);
  option=argc>4?strtol(argv[4],0,0):0;
  fread(buf,1,16,stdin);
  width=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  height=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  if(awidth>width || aheight>height || quant<0 || quant>=awidth*aheight || awidth<=0 || aheight<=0) fatal("Argument out of range");
  row=malloc((rowsize=aheight*width)*sizeof(Color));
  list=malloc((listsize=aheight*awidth)*sizeof(Color));
  if(!row || !list) fatal("Out of memory");
  buf[8]=(width+1-awidth)>>24;
  buf[9]=(width+1-awidth)>>16;
  buf[10]=(width+1-awidth)>>8;
  buf[11]=(width+1-awidth)>>0;
  height+=1-aheight;
  buf[12]=height>>24;
  buf[13]=height>>16;
  buf[14]=height>>8;
  buf[15]=height>>0;
  fwrite(buf,1,16,stdout);
  while(rowpos<width*(aheight-1)) getpixel();
  while(height--) do_row();
  return 0;
}

