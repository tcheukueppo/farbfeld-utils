#if 0
gcc -s -O2 -o ~/bin/ff-transopt -Wno-unused-result ff-transopt.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[8];
static int cutoff=0;

int main(int argc,char**argv) {
  if(argc>1) cutoff=strtol(argv[1],0,0);
  fread(buf,1,8,stdin); fwrite(buf,1,8,stdout);
  fread(buf,1,8,stdin); fwrite(buf,1,8,stdout);
  while(fread(buf,1,8,stdin)>0) {
    if((buf[6]<<8)+buf[7]<=cutoff) memset(buf,0,8);
    fwrite(buf,1,8,stdout);
  }
  return 0;
}
