#if 0
gcc -s -O2 -o ~/bin/gemicnff -Wno-unused-result gemicnff.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int width;
static int height;
static int block;
static int mask_image;
static int icon_image;
static int size;
static int foreground;
static int background;
static unsigned char*mask_buf;
static unsigned char*icon_buf;

static const unsigned char palette[16*3]={
  255,255,255,
  0,0,0,
  255,0,0,
  0,255,0,
  0,0,255,
  0,255,255,
  255,255,0,
  255,0,255,
  173,170,173,
  82,85,82,
  173,0,0,
  0,170,0,
  0,0,173,
  0,170,173,
  173,170,0,
  173,0,173,
};

static inline void skip(int x) {
  while(x--) fgetc(stdin);
}

static void read_iconblk(void) {
  mask_image=fgetc(stdin);
  mask_image|=fgetc(stdin)<<8;
  skip(2);
  icon_image=fgetc(stdin);
  icon_image|=fgetc(stdin)<<8;
  skip(7);
  foreground=fgetc(stdin);
  background=foreground&15;
  foreground>>=4;
  skip(8);
  width=fgetc(stdin);
  width|=fgetc(stdin)<<8;
  height=fgetc(stdin);
  height|=fgetc(stdin)<<8;
  skip(8);
}

static void picture_out(void) {
  int x,y,z,a;
  int p=((width+15)>>4)<<1;
  for(y=0;y<height;y++) for(x=0;x<width;x++) {
    a=(y*p+(x>>3))^1;
    z=3*(icon_buf[a]&(1<<(7&~x))?foreground:background);
    putchar(palette[z]); putchar(palette[z]);
    putchar(palette[z+1]); putchar(palette[z+1]);
    putchar(palette[z+2]); putchar(palette[z+2]);
    z=mask_buf[a]&(1<<(7&~x))?255:0;
    putchar(z); putchar(z);
  }
}

int main(int argc,char**argv) {
  int n;
  if(argc<2) {
    fprintf(stderr,"Too few arguments\n");
    return 1;
  }
  block=strtol(argv[1],0,0);
  if(block<0 || block>=72) {
    fprintf(stderr,"Invalid icon number\n");
    return 1;
  }
  skip(34*block+4);
  read_iconblk();
  skip(34*(71-block));
  size=height*((width+15)>>4)<<1;
  mask_buf=malloc(size);
  icon_buf=malloc(size);
  if(!mask_buf || !icon_buf) {
    fprintf(stderr,"Allocation failed\n");
    return 2;
  }
  if(mask_image==icon_image) mask_buf=icon_buf;
  for(n=0;n<=mask_image && n<=icon_image;n++) fread(n==mask_image?mask_buf:icon_buf,1,size,stdin);
  for(;n<=mask_image || n<=icon_image;n++) fread(mask_image>icon_image?mask_buf:icon_buf,1,size,stdin);
  fwrite("farbfeld",1,8,stdout);
  putchar(0); putchar(0); putchar(width>>8); putchar(width);
  putchar(0); putchar(0); putchar(height>>8); putchar(height);
  picture_out();
  return 0;
}

