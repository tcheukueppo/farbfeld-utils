#if 0
gcc -s -O2 -o ~/bin/ff-probaut -Wno-unused-result ff-probaut.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define fatal(...) do{ fprintf(stderr,__VA_ARGS__); exit(1); }while(0)

static unsigned char buf[16];
static int width,height;
static unsigned char*row;

static void process(void) {
  int w,x,y,z;
  w=row[width-1];
  for(x=0;x<width;x++) {
    fread(buf,1,8,stdin);
    y=(row[(x+1)%width]<<1)|(w<<2);
    w=row[x];
    y=(buf[y]<<8)|buf[y+1];
    z=random()&0xFFFF;
    if(z<y || y==0xFFFF) {
      row[x]=1;
      putchar(255); putchar(255);
      putchar(255); putchar(255);
      putchar(255); putchar(255);
    } else {
      row[x]=0;
      putchar(0); putchar(0);
      putchar(0); putchar(0);
      putchar(0); putchar(0);
    }
    putchar(255); putchar(255);
  }
}

int main(int argc,char**argv) {
  int i;
  if(argc>2) fatal("Too many arguments\n");
  if(argc==2) srandom(strtol(argv[1],0,0));
  fread(buf,1,16,stdin);
  width=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  height=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  row=malloc(width);
  if(!row) fatal("Allocation failed\n");
  fwrite(buf,1,16,stdout);
  for(i=0;i<width;i++) row[i]=random()&1;
  for(i=0;i<height;i++) process();
  return 0;
}
