#if 0
gcc -s -O2 -o ~/bin/ff-apclut -Wno-unused-result ff-apclut.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static unsigned char clut[134217728];

int main(int argc,char**argv) {
  FILE*fp;
  long k;
  if(argc<2) {
    fprintf(stderr,"Too few arguments\n");
    return 1;
  }
  fp=fopen(argv[1],"r");
  if(!fp) {
    perror(argv[1]);
    return 1;
  }
  fread(buf,1,16,fp);
  fread(clut,1,134217728,fp);
  fread(buf,1,16,stdin);
  fwrite(buf,1,16,stdout);
  while(fread(buf,1,8,stdin)>0) {
    k=(((long)buf[0])<<3)|(((long)buf[2])<<11)|(((long)buf[4])<<19);
    fwrite(clut+k,1,8,stdout);
  }
  return 0;
}

