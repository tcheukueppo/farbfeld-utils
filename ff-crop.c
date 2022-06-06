#if 0
gcc -s -O2 -o ~/bin/ff-crop -Wno-unused-result ff-crop.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[16];
static int width,height,xoffs,yoffs,xsize,ysize,active;
static unsigned char*row;
static unsigned char*actrow;

static void parse_color(unsigned char*out,const char*in) {
  switch(strlen(in)) {
    case 6:
      sscanf(in,"%02hhX%02hhX%02hhX",out+0,out+2,out+4);
      out[1]=out[0];
      out[3]=out[2];
      out[5]=out[4];
      out[6]=out[7]=255;
      break;
    case 8:
      sscanf(in,"%02hhX%02hhX%02hhX%02hhX",out+0,out+2,out+4,out+6);
      out[1]=out[0];
      out[3]=out[2];
      out[5]=out[4];
      out[7]=out[6];
      break;
    case 12:
      sscanf(in,"%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",out+0,out+1,out+2,out+3,out+4,out+5);
      out[6]=out[7]=255;
      break;
    case 16:
      sscanf(in,"%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",out+0,out+1,out+2,out+3,out+4,out+5,out+6,out+7);
      break;
    default:
      fprintf(stderr,"Invalid color format (%d)\n",(int)strlen(in));
      exit(1);
  }
}

int main(int argc,char**argv) {
  int i;
  if(argc<5) {
    fprintf(stderr,"Too few arguments\n");
    return 1;
  }
  xoffs=strtol(argv[1],0,0);
  yoffs=strtol(argv[2],0,0);
  xsize=strtol(argv[3],0,0);
  ysize=strtol(argv[4],0,0);
  fread(buf,1,16,stdin);
  width=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
  height=(buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15];
  row=calloc(8,width);
  if(!row) {
    fprintf(stderr,"Out of memory\n");
    return 1;
  }
  buf[8]=xsize>>24;
  buf[9]=xsize>>16;
  buf[10]=xsize>>8;
  buf[11]=xsize;
  buf[12]=ysize>>24;
  buf[13]=ysize>>16;
  buf[14]=ysize>>8;
  buf[15]=ysize;
  fwrite(buf,1,16,stdout);
  if(argc>5 && argv[5][0]) parse_color(buf,argv[5]); else memset(buf,0,8);
  while(yoffs<0) {
    yoffs++;
    i=xsize;
    while(i--) fwrite(buf,1,8,stdout);
  }
  while(yoffs-->0) {
    fread(row,8,width,stdin);
    height--;
  }
  active=xsize;
  if(xoffs<0) active+=xoffs;
  if(active>width-xoffs) active=width-xoffs;
  if(active>width) active=width;
  actrow=row+(xoffs<0?0:xoffs*8);
  while(height-->0 && ysize-->0) {
    fread(row,8,width,stdin);
    if(xoffs<0) for(i=xoffs;i;i++) fwrite(buf,1,8,stdout);
    if(xoffs<width) fwrite(actrow,8,active,stdout);
    for(i=active-(xoffs<0?xoffs:0);i<xsize;i++) fwrite(buf,1,8,stdout);
  }
  while(ysize-->0) {
    i=xsize;
    while(i--) fwrite(buf,1,8,stdout);
  }
  return 0;
}
