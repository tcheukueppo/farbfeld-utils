#if 0
gcc -s -O2 -o ~/bin/ff-tartan -Wno-unused-result ff-tartan.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned short d[4];
  unsigned short c;
} Color;

static char mix[64];
static char nmix;
static int width,height;
static Color col[64];
static char ncol;

static void parse_color(unsigned short*o,const char*s) {
  switch(strlen(s)) {
    case 6:
      sscanf(s,"%2hX%2hX%2hX",o+0,o+1,o+2);
      o[0]*=0x101;
      o[1]*=0x101;
      o[2]*=0x101;
      o[3]=0xFFFF;
      break;
    case 8:
      sscanf(s,"%2hX%2hX%2hX%2hX",o+0,o+1,o+2,o+3);
      o[0]*=0x101;
      o[1]*=0x101;
      o[2]*=0x101;
      o[3]*=0x101;
      break;
    case 12:
      sscanf(s,"%4hX%4hX%4hX",o+0,o+1,o+2);
      o[3]=0xFFFF;
      break;
    case 16:
      sscanf(s,"%4hX%4hX%4hX%4hX",o+0,o+1,o+2,o+3);
      break;
    default:
      fprintf(stderr,"Improper color format: %s\n",s);
      exit(1);
  }
}

static inline void do_channel(int c,int x,int y,int m) {
  m=((8-m)*col[x].d[c]+m*col[y].d[c])>>3;
  putchar(m>>8); putchar(m);
}

int main(int argc,char**argv) {
  int i,x,xp,xq,y,yp,yq;
  char*s;
  if(argc<5 || argc>68) {
    fprintf(stderr,"Too %s arguments\n",argc<5?"few":"many");
    return 1;
  }
  width=strtol(argv[1],0,10);
  height=strtol(argv[2],0,10);
  if(!argv[3][0] || strlen(argv[3])>64) {
    fprintf(stderr,"Invalid mix list\n");
    return 1;
  }
  nmix=strlen(argv[3]);
  for(i=0;i<nmix;i++) mix[i]=argv[3][i]-'0';
  ncol=argc-4;
  for(i=0;i<ncol;i++) {
    col[i].c=strtol(argv[i+4],&s,10);
    if(!col[i].c || *s!='x') {
      fprintf(stderr,"Syntax error: %s\n",argv[i+4]);
      return 1;
    }
    parse_color(col[i].d,s+1);
  }
  fwrite("farbfeld",1,8,stdout);
  putchar(width>>24); putchar(width>>16); putchar(width>>8); putchar(width);
  putchar(height>>24); putchar(height>>16); putchar(height>>8); putchar(height);
  for(y=yp=yq=0;y<height;y++) {
    for(x=xp=xq=0;x<width;x++) {
      do_channel(0,xp,yp,mix[(x+y)%nmix]);
      do_channel(1,xp,yp,mix[(x+y)%nmix]);
      do_channel(2,xp,yp,mix[(x+y)%nmix]);
      do_channel(3,xp,yp,mix[(x+y)%nmix]);
      if(xq++==col[xp].c) xp=(xp+1)%ncol,xq=0;
    }
    if(yq++==col[yp].c) yp=(yp+1)%ncol,yq=0;
  }
  return 0;
}
