#if 0
gcc -s -O2 -o ~/bin/ff-false -Wno-unused-result ff-false.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned short r;
  unsigned short g;
  unsigned short b;
  unsigned short a;
} Color;

static int width;
static int height;
static Color shade[256];

static Color parse_color(const char*s) {
  Color c;
  switch(strlen(s)) {
    case 2:
      sscanf(s,"%2hx",&c.r);
      c.r*=0x101;
      c.a=c.g=c.b=c.r;
      break;
    case 4:
      sscanf(s,"%4hx",&c.r);
      c.a=c.g=c.b=c.r;
      break;
    case 6:
      sscanf(s,"%2hx%2hx%2hx",&c.r,&c.g,&c.b);
      c.r*=0x101;
      c.g*=0x101;
      c.b*=0x101;
      c.a=0xFFFF;
      break;
    case 8:
      sscanf(s,"%2hx%2hx%2hx%2hx",&c.r,&c.g,&c.b,&c.a);
      c.r*=0x101;
      c.g*=0x101;
      c.b*=0x101;
      c.a*=0x101;
      break;
    case 12:
      sscanf(s,"%4hx%4hx%4hx",&c.r,&c.g,&c.b);
      c.a=0xFFFF;
      break;
    case 16:
      sscanf(s,"%4hx%4hx%4hx%4hx",&c.r,&c.g,&c.b,&c.a);
      break;
  }
  return c;
}

static void calc_gradient(Color s1,Color s2,int i1,int i2) {
  Color c;
  int i;
  double g;
  for(i=i1;i<=i2;i++) {
    g=(i-i1)/(double)(i2-i1);
    if(s1.r==s2.r) c.r=s1.r; else c.r=g*s1.r+(1.0-g)*s2.r;
    if(s1.g==s2.g) c.g=s1.g; else c.g=g*s1.g+(1.0-g)*s2.g;
    if(s1.b==s2.b) c.b=s1.b; else c.b=g*s1.b+(1.0-g)*s2.b;
    if(s1.a==s2.a) c.a=s1.a; else c.a=g*s1.a+(1.0-g)*s2.a;
    if(c.r>shade[i].r) shade[i].r=c.r;
    if(c.g>shade[i].g) shade[i].g=c.g;
    if(c.b>shade[i].b) shade[i].b=c.b;
    if(c.a>shade[i].a) shade[i].a=c.a;
  }
}

int main(int argc,char**argv) {
  int i;
  if((argc-1)%4) {
    fprintf(stderr,"Wrong number of arguments\n");
    return 1;
  }
  for(i=1;i<argc;i+=4) calc_gradient(parse_color(argv[i+1]),parse_color(argv[i]),strtol(argv[i+2],0,0)&255,strtol(argv[i+3],0,0)&255);
  for(i=0;i<16;i++) putchar(fgetc(stdin));
  for(;;) {
    i=fgetc(stdin);
    if(i==EOF) return 0;
    putchar(shade[i].r>>8);
    putchar(shade[i].r);
    fgetc(stdin);
    i=fgetc(stdin);
    if(i==EOF) return 0;
    putchar(shade[i].g>>8);
    putchar(shade[i].g);
    fgetc(stdin);
    i=fgetc(stdin);
    if(i==EOF) return 0;
    putchar(shade[i].b>>8);
    putchar(shade[i].b);
    fgetc(stdin);
    i=fgetc(stdin);
    if(i==EOF) return 0;
    putchar(shade[i].a>>8);
    putchar(shade[i].a);
    fgetc(stdin);
  }
}
