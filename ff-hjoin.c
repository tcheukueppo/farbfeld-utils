#if 0
gcc -s -O2 -o ~/bin/ff-hjoin -Wno-unused-result ff-hjoin.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int w;
  int h;
  FILE*f;
} Item;

static int count;
static Item*item;
static int width;
static int height;

static inline int get32(FILE*fp) {
  int x=fgetc(fp)<<24;
  x|=fgetc(fp)<<16;
  x|=fgetc(fp)<<8;
  return x|fgetc(fp);
}

static inline void put32(int x) {
  putchar(x>>24);
  putchar(x>>16);
  putchar(x>>8);
  putchar(x);
}

static inline void load_headers(char**name) {
  int i,j;
  for(i=0;i<count;i++) {
    item[i].f=fopen(name[i],"rb");
    if(!item[i].f) {
      perror(name[i]);
      exit(1);
    }
    for(j=0;j<8;j++) fgetc(item[i].f);
    width+=item[i].w=get32(item[i].f);
    item[i].h=get32(item[i].f);
    if(height<item[i].h) height=item[i].h;
  }
}

static inline void copy_pictures(void) {
  int i,j,k;
  for(j=0;j<height;j++) {
    for(i=0;i<count;i++) {
      k=item[i].w<<3;
      if(j>=height-item[i].h) {
        while(k--) putchar(fgetc(item[i].f));
      } else {
        while(k--) putchar(0);
      }
    }
  }
}

int main(int argc,char**argv) {
  if(argc<2) {
    fprintf(stderr,"Too few arguments\n");
    return 1;
  }
  item=calloc(sizeof(Item),count=argc-1);
  if(!item) {
    fprintf(stderr,"Allocation failed\n");
    return 1;
  }
  load_headers(argv+1);
  fwrite("farbfeld",1,8,stdout);
  put32(width);
  put32(height);
  copy_pictures();
  return 0;
}
