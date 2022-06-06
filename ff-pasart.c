#if 0
gcc -s -O2 -o ~/bin/ff-pasart -Wno-unused-result ff-pasart.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned char data[8];
} Color;

static unsigned char buf[16];
static int npal,mode,size;
static Color pal[256];
static unsigned char*grid;

static void do_mode_1(void) {
  int i;
  *grid=1;
  for(i=size;i<size*size;i++) grid[i]=(grid[i-size]+(i%size?grid[i-size-1]:0))%npal;
}

static void do_mode_2(void) {
  int i;
  memset(grid,1,size);
  for(i=size;i<size*size;i++) grid[i]=i%size?(grid[i-1]+grid[i-size])%npal:1;
}

static void do_mode_3(void) {
  int i;
  for(i=size;i<size*size;i++) grid[i]=(grid[i-size]+(i%size?grid[i-size-1]:0)+i)%npal;
}

static void do_mode_4(void) {
  int i;
  memset(grid,1,size);
  for(i=size;i<size*size;i++) grid[i]=i%size?(grid[i-1]+grid[i-size]+grid[i-size-1])%npal:1;
}

static const typeof(&do_mode_1) modefnc[]={
  do_mode_1,
  do_mode_2,
  do_mode_3,
  do_mode_4,
};

int main(int argc,char**argv) {
  int i,j;
  if(argc<5) {
    fprintf(stderr,"Too few arguments\n");
    return 1;
  }
  npal=argc-3;
  mode=strtol(argv[2],0,0);
  if(mode<1 || mode>sizeof(modefnc)/sizeof(*modefnc)) {
    fprintf(stderr,"Invalid mode: %d\n",mode);
    return 1;
  }
  size=strtol(argv[1],0,0);
  grid=calloc(size,size);
  if(!grid) {
    fprintf(stderr,"Allocation failed\n");
    return 1;
  }
  for(i=0;i+3<argc && i<256;i++) {
    unsigned char*d=pal[i].data;
    j=strlen(argv[i+3]);
    if(j==6) {
      sscanf(argv[i+3],"%2hhX%2hhX%2hhX",d+0,d+2,d+4);
      d[1]=d[0]; d[3]=d[2]; d[5]=d[4]; d[7]=d[6]=255;
    } else if(j==8) {
      sscanf(argv[i+3],"%2hhX%2hhX%2hhX%2hhX",d+0,d+2,d+4,d+6);
      d[1]=d[0]; d[3]=d[2]; d[5]=d[4]; d[7]=d[6];
    } else if(j==12) {
      sscanf(argv[i+3],"%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX",d+0,d+1,d+2,d+3,d+4,d+5);
      d[7]=d[6]=255;
    } else if(j==16) {
      sscanf(argv[i+3],"%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX",d+0,d+1,d+2,d+3,d+4,d+5,d+6,d+7);
    } else {
      fprintf(stderr,"Invalid color\n");
      return 1;
    }
  }
  modefnc[mode-1]();
  fwrite("farbfeld",1,8,stdout);
  putchar(size>>24); putchar(size>>16); putchar(size>>8); putchar(size);
  putchar(size>>24); putchar(size>>16); putchar(size>>8); putchar(size);
  for(i=0;i<size*size;i++) fwrite(pal[grid[i]].data,1,8,stdout);
  return 0;
}
