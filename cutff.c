#if 0
gcc -s -O2 -o ~/bin/cutff -Wno-unused-result cutff.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned char c[8];
} Color;

static Color pal[256];
static unsigned char head[6];

static void set_color(int n,const char*s) {
  unsigned char*p=pal[n].c;
  switch(strlen(s)) {
    case 1:
      if(*s=='-') for(n=0;n<256;n++) memset(pal[n].c,n,6);
      break;
    case 6:
      sscanf(s,"%2hhX%2hhX%2hhX",p+0,p+2,p+4);
      p[1]=p[0]; p[3]=p[2]; p[5]=p[4];
      break;
    case 8:
      sscanf(s,"%2hhX%2hhX%2hhX%2hhX",p+0,p+2,p+4,p+6);
      p[1]=p[0]; p[3]=p[2]; p[5]=p[4]; p[7]=p[6];
      break;
    case 12:
      sscanf(s,"%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX",p+0,p+1,p+2,p+3,p+4,p+5);
      break;
    case 16:
      sscanf(s,"%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX",p+0,p+1,p+2,p+3,p+4,p+5,p+6,p+7);
      break;
  }
}

static inline void do_scanline(void) {
  int a,b;
  fgetc(stdin); fgetc(stdin); // Ignore line record length
  for(;;) {
    a=fgetc(stdin);
    if(!(a&127)) break;
    if(a&128) {
      b=fgetc(stdin);
      a&=127;
      while(a--) fwrite(pal[b].c,1,8,stdout);
    } else {
      while(a--) fwrite(pal[fgetc(stdin)&255].c,1,8,stdout);
    }
  }
}

int main(int argc,char**argv) {
  int i;
  pal[0].c[6]=pal[0].c[7]=255;
  for(i=1;i<256;i++) memset(pal[i].c,255,8);
  for(i=0;i<argc-1 && i<256;i++) set_color(i,argv[i+1]);
  fread(head,1,6,stdin);
  fwrite("farbfeld",1,8,stdout);
  putchar(0); putchar(0); putchar(head[1]); putchar(head[0]);
  putchar(0); putchar(0); putchar(head[3]); putchar(head[2]);
  i=head[2]|(head[3]<<8);
  while(i--) do_scanline();
  return 0;
}
