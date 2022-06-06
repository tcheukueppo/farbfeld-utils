#if 0
gcc -s -O2 -o ~/bin/timaskff -Wno-unused-result timaskff.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static int width,height,total,bits,bitv;
static signed char*pic;
static const char pal[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255};

static inline int get_bit(void) {
  int i;
  if(!bits) bitv=fgetc(stdin);
  if(bitv==EOF) {
    fprintf(stderr,"Unexpected end of file\n");
    exit(1);
  }
  i=(bitv>>bits++)&1;
  if(bits==8) bits=0;
  return i;
}

static int get_number(void) {
  int n=0;
  int i=0;
  while(get_bit()) n|=get_bit()<<i++;
  return n|(1<<i);
}

static int figure_box(int start) {
  int v=get_bit();
  int x1=get_number();
  int y1=get_number();
  int x,y,z;
  for(x=0;x<x1;x++) {
    for(y=0;y<y1;y++) if(pic[start+x+y*width]==-1) break;
    if(y==y1) x1++;
  }
  for(x=0;x<x1;x++) {
    for(y=0;y<y1;y++) if(pic[z=start+x+y*width]==-1 && z<total) pic[z]=v;
  }
  return x1;
}

int main(int argc,char**argv) {
  int i;
  width=fgetc(stdin)<<8;
  width|=fgetc(stdin);
  height=fgetc(stdin)<<8;
  height|=fgetc(stdin);
  fread(buf,1,4,stdin); // Ignore X/Y offset
  if(argc==3) width=strtol(argv[1],0,10),height=strtol(argv[2],0,10);
  pic=malloc(total=width*height);
  if(!pic) {
    fprintf(stderr,"Allocation failed\n");
    return 1;
  }
  memset(pic,-1,total);
  for(i=0;;) {
    i+=figure_box(i);
    while(i<total && pic[i]!=-1) i++;
    if(i==total) break;
  }
  fwrite("farbfeld\0\0",1,10,stdout);
  putchar(width>>8); putchar(width);
  putchar(0); putchar(0); putchar(height>>8); putchar(height);
  for(i=0;i<total;i++) fwrite(pal+8*pic[i],1,8,stdout);
  return 0;
}
