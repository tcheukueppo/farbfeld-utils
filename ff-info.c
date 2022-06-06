#if 0
gcc -s -O2 -o ~/bin/ff-info -Wno-unused-result ff-info.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];

#define width (buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11]
#define height (buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15]

int main(int argc,char**argv) {
  fread(buf,1,16,stdin);
  if(argc>1 && argv[1][0]=='w') printf("%d\n",width);
  else if(argc>1 && argv[1][0]=='h') printf("%d\n",height);
  else printf("%d %d\n",width,height);
  return 0;
}
