#if 0
gcc -s -O2 -o ~/bin/ffpbm -Wno-unused-result ffpbm.c
exit
#endif
/*
  Convert farbfeld to PBM/PGM/PPM
  This and all other programs at http://zzo38computer.org/prog/farbfeld.zip are public domain, except LodePNG.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char format='4';
static int depth=16;
static unsigned char buf[8];
static int width;
static int pos=0;

int main(int argc,char**argv) {
  int i;
  if(argc>1) format=argv[1][0];
  if(argc>2) depth=strtol(argv[2],0,0);
  fread(buf,1,8,stdin);
  if(memcmp("farbfeld",buf,8)) {
    fprintf(stderr,"Not farbfeld\n");
    return 1;
  }
  if(depth<1 || depth>16 || format<'1' || format>'7') {
    fprintf(stderr,"Invalid output format (%c,%d)\n",format,depth);
    return 1;
  }
  putchar('P');
  putchar(format);
  if(format!='7') {
    i=fgetc(stdin)<<24;
    i|=fgetc(stdin)<<16;
    i|=fgetc(stdin)<<8;
    i|=fgetc(stdin);
    printf("\n%d",width=i);
    i=fgetc(stdin)<<24;
    i|=fgetc(stdin)<<16;
    i|=fgetc(stdin)<<8;
    i|=fgetc(stdin);
    printf("\n%d\n",i);
    if(format!='1' && format!='4') printf("%d\n",(1<<depth)-1);
  }
  --depth;
  switch(format) {
    case '1':
      while(fread(buf,8,1,stdin)) {
        putchar('1'^(*buf>>7));
        putchar('\n');
      }
      break;
    case '2':
      // This program outputs linear PGM/PPM.
      // To gamma correct it, use pnmgamma or ff-bright
      while(fread(buf,8,1,stdin)) {
        printf("%d\n",((buf[0]<<8)|buf[1])>>(15-depth));
      }
      break;
    case '3':
      while(fread(buf,8,1,stdin)) {
        printf("%d %d %d\n",((buf[0]<<8)|buf[1])>>(15-depth),((buf[2]<<8)|buf[3])>>(15-depth),((buf[4]<<8)|buf[5])>>(15-depth));
      }
      break;
    case '4':
      while(fread(buf,8,1,stdin)) {
        if(!(pos&7)) i=255;
        i^=(*buf>>7)<<(7&~pos);
        if((pos&7)==7 || pos==width-1) putchar(i);
        pos=(pos+1)%width;
      }
      break;
    case '5':
      while(fread(buf,8,1,stdin)) {
        putchar(*buf>>(7&~depth));
        if(depth&8) putchar(((buf[0]<<8)|buf[1])>>(15-depth));
      }
      break;
    case '6':
      while(fread(buf,8,1,stdin)) {
        putchar(buf[0]>>(7&~depth));
        if(depth&8) putchar(((buf[0]<<8)|buf[1])>>(15-depth));
        putchar(buf[2]>>(7&~depth));
        if(depth&8) putchar(((buf[2]<<8)|buf[3])>>(15-depth));
        putchar(buf[4]>>(7&~depth));
        if(depth&8) putchar(((buf[4]<<8)|buf[5])>>(15-depth));
      }
      break;
    case '7':
      i=fgetc(stdin)<<24;
      i|=fgetc(stdin)<<16;
      i|=fgetc(stdin)<<8;
      i|=fgetc(stdin);
      printf("\nWIDTH %d\n",i);
      i=fgetc(stdin)<<24;
      i|=fgetc(stdin)<<16;
      i|=fgetc(stdin)<<8;
      i|=fgetc(stdin);
      printf("HEIGHT %d\n",i);
      printf("MAXVAL %d\nDEPTH 4\nTUPLTYPE RGB_ALPHA\nENDHDR\n",(2<<depth)-1);
      while(fread(buf,8,1,stdin)) {
        putchar(buf[0]>>(7&~depth));
        if(depth&8) putchar(((buf[0]<<8)|buf[1])>>(15-depth));
        putchar(buf[2]>>(7&~depth));
        if(depth&8) putchar(((buf[2]<<8)|buf[3])>>(15-depth));
        putchar(buf[4]>>(7&~depth));
        if(depth&8) putchar(((buf[4]<<8)|buf[5])>>(15-depth));
        putchar(buf[6]>>(7&~depth));
        if(depth&8) putchar(((buf[6]<<8)|buf[7])>>(15-depth));
      }
      break;
  }
  return 0;
}
