#if 0
gcc -s -O2 -o ~/bin/ff-solar ff-solar.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static char chan;
static int param;

static void process(int pos) {
  int v=(buf[pos]<<8)|buf[pos+1];
  if(v>=param) v=65535-v;
  buf[pos]=v>>8;
  buf[pos+1]=v;
}

int main(int argc,char**argv) {
  int i;
  i=fread(buf,1,16,stdin);
  i=fwrite(buf,1,16,stdout);
  if(argc>1) {
    chan=0;
    for(i=0;argv[1][i];i++) {
      if(argv[1][i]=='r') chan|=1;
      if(argv[1][i]=='g') chan|=2;
      if(argv[1][i]=='b') chan|=4;
      if(argv[1][i]=='a') chan|=8;
    }
  } else {
    chan=7;
  }
  param=argc>2?strtol(argv[2],0,0):0x8000;
  while(fread(buf,1,8,stdin)) {
    if(chan&1) process(0);
    if(chan&2) process(2);
    if(chan&4) process(4);
    if(chan&8) process(6);
    i=fwrite(buf,1,8,stdout);
  }
  return 0;
}

