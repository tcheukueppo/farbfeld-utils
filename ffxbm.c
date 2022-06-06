#if 0
gcc -s -O2 -o ~/bin/ffxbm -Wno-unused-result ffxbm.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static const char*name="picture";
static int width,height,hot_x,hot_y;
static int pos;

static void output(int x) {
  if(pos==12) {
    printf(",\n   ");
    pos=0;
  } else if(pos) {
    printf(", ");
  }
  printf("0x%02x",x);
  ++pos;
}

static void do_row(void) {
  int i,d;
  for(i=d=0;i<width;i++) {
    fread(buf,1,8,stdin);
    d|=(128&~*buf)>>(7&~i);
    if(!(7&~i)) output(d),d=0;
  }
  if(width&7) output(d);
}

int main(int argc,char**argv) {
  fread(buf,1,16,stdin);
  if(argc>1 && argv[1][0]) name=argv[1];
  if(argc>3 && argv[2][0]) hot_x=strtol(argv[2],0,0),hot_y=strtol(argv[3],0,0);
  if(memcmp(buf,"farbfeld",8)) {
    fprintf(stderr,"Not farbfeld\n");
    return 1;
  }
  width=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  height=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  printf("#define %s_width %d\n#define %s_height %d\n",name,width,name,height);
  if(argc>3 && argv[2][0]) printf("#define %s_hot_x %d\n#define %s_hot_y %d\n",name,hot_x,name,hot_y);
  printf("static unsigned char %s_bits[] = {\n   ",name);
  while(height--) do_row();
  printf("};\n");
  return 0;
}
