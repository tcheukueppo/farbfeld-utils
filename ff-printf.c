#if 0
gcc -s -O2 -o ~/bin/ff-printf -Wno-unused-result ff-printf.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static int count;

int main(int argc,char**argv) {
  int i;
  char*form=argc>1?argv[1]:"%d %d %d %d\n";
  fread(buf,1,16,stdin);
  i=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  count=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  count*=i;
  if(argc>2 && argv[2][0]=='+') {
    while(count--) {
      fread(buf,1,8,stdin);
      printf(form,buf[0],buf[2],buf[4],buf[6]);
    }
  } else {
    while(count--) {
      fread(buf,1,8,stdin);
      printf(form,(buf[0]<<8)|buf[1],(buf[2]<<8)|buf[3],(buf[4]<<8)|buf[5],(buf[6]<<8)|buf[7]);
    }
  }
  return 0;
}
