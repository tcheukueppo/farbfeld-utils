#if 0
gcc -s -O2 -o ~/bin/ff-strip -Wno-unused-result ff-strip.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char*picture;
static int width,height,cwidth,cheight,strip;

static inline void read_picture() {
  unsigned char*p=picture;
  int w=width/cwidth;
  int h=height/cheight;
  int i,j;
  while(h--) {
    for(j=0;j<cheight;j++) for(i=0;i<w;i++) fread(p+(j+i*cheight)*cwidth*8,8,cwidth,stdin);
    p+=w*cheight*cwidth*8;
  }
}

static inline void write_header() {
  int w=cwidth*strip;
  int h=(height*width)/w;
  if((height*width)%w) {
    fprintf(stderr,"Invalid cell size\n");
    exit(1);
  }
  putchar(w>>24);
  putchar(w>>16);
  putchar(w>>8);
  putchar(w>>0);
  putchar(h>>24);
  putchar(h>>16);
  putchar(h>>8);
  putchar(h>>0);
}

static inline void write_picture() {
  unsigned char*p=picture;
  int h=(width*height)/(cwidth*cheight*strip);
  int i,j;
  while(h--) {
    for(j=0;j<cheight;j++) for(i=0;i<strip;i++) fwrite(p+(j+i*cheight)*cwidth*8,8,cwidth,stdout);
    p+=strip*cheight*cwidth*8;
  }
}

int main(int argc,char**argv) {
  if(argc!=4) {
    fprintf(stderr,"Wrong number of arguments\n");
    return 1;
  }
  cwidth=strtol(argv[1],0,0);
  cheight=strtol(argv[2],0,0);
  strip=strtol(argv[3],0,0);
  putchar(fgetc(stdin));
  putchar(fgetc(stdin));
  putchar(fgetc(stdin));
  putchar(fgetc(stdin));
  putchar(fgetc(stdin));
  putchar(fgetc(stdin));
  putchar(fgetc(stdin));
  putchar(fgetc(stdin));
  width=fgetc(stdin)<<24;
  width|=fgetc(stdin)<<16;
  width|=fgetc(stdin)<<8;
  width|=fgetc(stdin);
  height=fgetc(stdin)<<24;
  height|=fgetc(stdin)<<16;
  height|=fgetc(stdin)<<8;
  height|=fgetc(stdin);
  if(cwidth<0) cwidth=width/-cwidth;
  if(cheight<0) cheight=height/-cheight;
  picture=malloc(width*height*8);
  if(!picture) {
    fprintf(stderr,"Allocation failed\n");
    return 1;
  }
  if(!cwidth || !cheight || !strip || width%cwidth || height%cheight) {
    fprintf(stderr,"Invalid cell size\n");
    return 1;
  }
  read_picture();
  write_header();
  write_picture();
  return 0;
}
