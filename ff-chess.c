#if 0
gcc -s -O2 -o ~/bin/ff-chess ff-chess.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int xsize,ysize,tsize,x,y,xx,yy;
static unsigned char color[16];

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
  if(argc!=6) {
    fprintf(stderr,"Too %s arguments\n",argc<6?"few":"many");
    return 1;
  }
  xsize=strtol(argv[1],0,0);
  ysize=strtol(argv[2],0,0);
  tsize=strtol(argv[3],0,0);
  parse_color(color,argv[4]);
  parse_color(color+8,argv[5]);
  fwrite("farbfeld",1,8,stdout);
  putchar((xsize*tsize)>>24);
  putchar((xsize*tsize)>>16);
  putchar((xsize*tsize)>>8);
  putchar((xsize*tsize)>>0);
  putchar((ysize*tsize)>>24);
  putchar((ysize*tsize)>>16);
  putchar((ysize*tsize)>>8);
  putchar((ysize*tsize)>>0);
  for(y=0;y<ysize;y++) for(yy=0;yy<tsize;yy++) for(x=0;x<xsize;x++) for(xx=0;xx<tsize;xx++) fwrite(color+(((x+y)&1)<<3),1,8,stdout);
  return 0;
}

