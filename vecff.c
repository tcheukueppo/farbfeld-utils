#if 0
gcc -s -O2 -o ~/bin/vecff -Wno-unused-result vecff.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int width,height,xcur,ycur,xin,yin;
static unsigned char color[16];
static char*pic;
static char*endpic;

static void pset(int x,int y) {
  if(x<0 || x>=width || y<0 || y>=height) return;
  pic[y*width+x]=1;
}

static void draw_line(int x2,int y2) {
  int x1=xcur;
  int y1=ycur;
  int dx=abs(x1-x2);
  int dy=abs(y1-y2);
  int x=x1;
  int y=y1;
  int x1i=dx>=dy?0:x2>=x1?1:-1;
  int y1i=dx<dy?0:y2>=y1?1:-1;
  int x2i=dx<dy?0:x2>=x1?1:-1;
  int y2i=dx>=dy?0:y2>=y1?1:-1;
  int n=dx>=dy?dx:dy;
  int den=n;
  int num=n>>1;
  int add=dx<dy?dx:dy;
  int i;
  for(i=0;i<=n;i++) {
    pset(x,y);
    num+=add;
    if(num>=den) {
      num-=den;
      x+=x1i;
      y+=y1i;
    }
    x+=x2i;
    y+=y2i;
  }
  xcur=x2;
  ycur=y2;
}

static void read_coord(void) {
  unsigned long long i;
  i=fgetc(stdin); i|=fgetc(stdin)<<8; xin=(i*width)/1000;
  i=fgetc(stdin); i|=fgetc(stdin)<<8; yin=(i*height)/1000;
}

int main(int argc,char**argv) {
  if(argc!=4) {
    fprintf(stderr,"Improper number of command line arguments\n");
    return 1;
  }
  width=strtol(argv[1],0,0);
  height=strtol(argv[2],0,0);
  switch(strlen(argv[3])) {
    case 6:
      sscanf(argv[3],"%2hhX%2hhX%2hhX",color+0,color+2,color+4);
      color[1]=color[0]; color[3]=color[2]; color[5]=color[4]; color[7]=color[6]=255;
      break;
    case 8:
      sscanf(argv[3],"%2hhX%2hhX%2hhX%2hhX",color+0,color+2,color+4,color+6);
      color[1]=color[0]; color[3]=color[2]; color[5]=color[4]; color[7]=color[6];
      break;
    case 12:
      sscanf(argv[3],"%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX",color+0,color+1,color+2,color+3,color+4,color+5);
      color[7]=color[6]=255;
      break;
    case 16:
      sscanf(argv[3],"%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX",color+0,color+1,color+2,color+3,color+4,color+5,color+6,color+7);
      break;
    default:
      fprintf(stderr,"Invalid color format\n");
      return 1;
  }
  pic=calloc(width,height);
  if(!pic) {
    fprintf(stderr,"Allocation failed\n");
    return 1;
  }
  endpic=pic+width*height;
  for(;;) {
    switch(fgetc(stdin)) {
      case 'L': read_coord(); draw_line(xin,yin); break;
      case 'P': read_coord(); xcur=xin; ycur=yin; break;
      case 'X': goto done;
      default: fprintf(stderr,"Unrecognized command in input\n"); break;
    }
  }
  done:
  fwrite("farbfeld",1,8,stdout);
  putchar(width>>24); putchar(width>>16); putchar(width>>8); putchar(width);
  putchar(height>>24); putchar(height>>16); putchar(height>>8); putchar(height);
  while(pic!=endpic) fwrite(color+(*pic++?0:8),1,8,stdout);
  return 0;
}
