#if 0
gcc -s -O2 -o ~/bin/ff-channel -Wno-unused-result ff-channel.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[64];
static FILE*file[4];

int main(int argc,char**argv) {
  int i;
  if(argc<5 && argc!=3) {
    fprintf(stderr,"Incorrect number of arguments\n");
    return 1;
  }
  for(i=0;i<4;i++) {
    if(argc==3 && i==2) break;
    file[i]=fopen(argv[i+1],"r");
    if(!file[i]) {
      perror(argv[i+1]);
      return 1;
    }
  }
  fread(buf+0,1,16,file[0]);
  fread(buf+16,1,16,file[1]);
  if(argc==3) {
    memcpy(buf+32,buf,32);
  } else {
    fread(buf+32,1,16,file[2]);
    fread(buf+48,1,16,file[3]);
  }
  if(memcmp(buf,buf+16,16) || memcmp(buf,buf+32,16) || memcmp(buf,buf+48,16)) {
    fprintf(stderr,"Header mismatch\n");
    return 1;
  }
  fwrite(buf,1,16,stdout);
  if(argc==3) {
    while(fread(buf+0,1,8,file[0]) && fread(buf+8,1,8,file[1])) {
      fwrite(buf+0,1,6,stdout);
      fwrite(buf+14,1,2,stdout);
    }
  } else {
    while(fread(buf+0,1,8,file[0]) && fread(buf+16,1,8,file[1]) && fread(buf+32,1,8,file[2]) && fread(buf+48,1,8,file[3])) {
      fwrite(buf+0,1,2,stdout);
      fwrite(buf+18,1,2,stdout);
      fwrite(buf+36,1,2,stdout);
      fwrite(buf+54,1,2,stdout);
    }
  }
  return 0;
}
