#if 0
gcc -s -O2 -o ~/bin/fftimask -Wno-unused-result fftimask.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static int width,height,total,mode,bits,bitv;
static signed char*pic;

static inline void emit_bit(int b) {
  bitv|=b<<bits++;
  if(bits==8) {
    putchar(bitv);
    bits=bitv=0;
  }
}

static void emit_number(int n) {
  while(n>1) {
    emit_bit(1);
    emit_bit(n&1);
    n>>=1;
  }
  emit_bit(0);
}

static int figure_box(int start) {
  int t=0;
  int v=pic[start];
  int x,y,z,x1,y1;
  int m=height;
  int xs=0;
  if(mode==1) m=1;
  for(x=0;x<total-start;x++) {
    if(pic[start+x]!=-1 && pic[start+x]!=v) break;
    if(x==width) m=1;
    for(y=0;y<m && (z=start+x+y*width)<total;y++) if(pic[z]!=-1 && pic[z]!=v) break;
    m=y;
    z=(x+1)*y;
    if(z>t) t=z,x1=x+1,y1=y;
  }
  if(!t) x1=x+1,y1=y;
  for(x=0;x<x1;x++) {
    for(y=0;y<y1;y++) if(pic[start+x+y*width]!=-1) break;
    if(y==y1) xs++;
  }
  for(x=0;x<x1;x++) {
    for(y=0;y<y1;y++) pic[start+x+y*width]=-1;
  }
  emit_bit(v);
  emit_number(x1-xs);
  emit_number(y1);
  return x1;
}

int main(int argc,char**argv) {
  int i;
  if(argc!=4) {
    fprintf(stderr,"Incorrect number of arguments\n");
    return 1;
  }
  mode=strtol(argv[3],0,10);
  fread(buf,1,16,stdin);
  if(memcmp(buf,"farbfeld",8)) {
    fprintf(stderr,"Not farbfeld\n");
    return 1;
  }
  if(buf[8]|buf[9]|buf[12]|buf[13]) {
    fprintf(stderr,"Input picture is too big\n");
    return 1;
  }
  width=(buf[10]<<8)|buf[11];
  height=(buf[14]<<8)|buf[15];
  pic=malloc(total=width*height);
  if(!pic) {
    fprintf(stderr,"Allocation failed\n");
    return 1;
  }
  for(i=0;i<total;i++) {
    fread(buf,1,8,stdin);
    pic[i]=buf[6]?1:0;
  }
  putchar(width>>8); putchar(width);
  putchar(height>>8); putchar(height);
  i=strtol(argv[1],0,10)+0x8000;
  putchar(i>>8); putchar(i);
  i=strtol(argv[2],0,10)+0x8000;
  putchar(i>>8); putchar(i);
  bits=0;
  for(i=0;;) {
    i+=figure_box(i);
    while(i<total && pic[i]==-1) i++;
    if(i==total) break;
  }
  for(i=0;i<7;i++) emit_bit(1);
  return 0;
}
