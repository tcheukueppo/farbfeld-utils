#if 0
gcc -s -O2 -o ~/bin/ffxzip -Wno-unused-result ffxzip.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define fatal(...) do{ fprintf(stderr,__VA_ARGS__); fputc('\n',stderr); exit(1); }while(0)

typedef struct {
  FILE*fp;
  int w,h;
  char n[14];
} Picture;

static unsigned char buf[16];
static int gcount,goff,gsize,gflags,npic;
static Picture*pic;

static inline void put16(int x) {
  putchar(x>>8); putchar(x);
}

static void do_picture(const Picture*p) {
  int x,y,z;
  for(y=0;y<p->h;y++) for(x=z=0;x<p->w;x++) {
    fread(buf,1,8,p->fp);
    z=(z<<1)|(buf[6]&128?1:0);
    if(x==p->w-1 || (x&15)==15) put16(z),z=0;
  }
}

int main(int argc,char**argv) {
  int i;
  if(argc<3 || (argc>4 && !(argc&1))) fatal("Incorrect number of arguments");
  gflags=strtol(argv[1],0,0);
  goff=strtol(argv[2],0,0);
  if(argc==4) {
    pic=malloc(sizeof(Picture));
    if(!pic) fatal("Allocation failed");
    pic->fp=stdin;
    npic=1;
    gsize=strtol(argv[3],0,0);
    if(gsize<1 || gsize>255) fatal("Invalid character height");
    fread(buf,1,16,stdin);
    if(memcmp(buf,"farbfeld",8)) fatal("Not farbfeld");
    if(buf[8] || buf[9] || buf[10] || buf[12] || buf[13]) fatal("Picture too big");
    pic->w=buf[11];
    pic->h=(buf[12]<<8)|buf[13];
    if(!pic->w) fatal("Input width is zero");
    if(pic->h%gsize) fatal("Height of input is not divisible by %d",gsize);
    gcount=pic->h/gsize;
    if(gcount<1 || gcount>65535) fatal("Number of characters is not valid");
    gsize|=buf[11]<<8;
  } else {
    gsize=0;
    npic=gcount=(argc-2)>>1;
    pic=malloc(npic*sizeof(Picture));
    if(npic && !pic) fatal("Allocation failed");
    for(i=0;i<npic;i++) {
      snprintf(pic[i].n,13,"%s            ",argv[i+i+3]);
      if(argv[i+i+4][0]) {
        pic[i].fp=fopen(argv[i+i+4],"r");
        if(!pic[i].fp) {
          perror(argv[i+i+4]);
          fatal("Cannot input picture %d",i+goff);
        }
        fread(buf,1,16,pic[i].fp);
        if(memcmp(buf,"farbfeld",8)) fatal("Not farbfeld");
        if(buf[8] || buf[9] || buf[12] || buf[13]) fatal("Picture %d is too big",i+goff);
        pic[i].w=(buf[10]<<8)|buf[11];
        pic[i].h=(buf[14]<<8)|buf[15];
      } else {
        pic[i].fp=0;
        pic[i].w=pic[i].h=0;
      }
    }
  }
  put16(gcount); put16(goff); put16(gsize); put16(gflags);
  put16(0); put16(0); put16(0); put16(0);
  put16(0); put16(0); put16(0); put16(0);
  put16(0); put16(0); put16(0); put16(0);
  if(!gsize) {
    for(i=0;i<npic;i++) put16(pic[i].w),put16(pic[i].h);
    for(i=0;i<npic;i++) fwrite(pic[i].n,1,12,stdout);
  }
  for(i=0;i<npic;i++) do_picture(pic+i);
  return 0;
}
