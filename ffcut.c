#if 0
gcc -s -O2 -o ~/bin/ffcut -Wno-unused-result ffcut.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned char c[8];
} Color;

static Color pal[256];
static unsigned char buf[65536];
static unsigned char line[65536];
static int width,inpos,outpos;

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

static inline void do_run(void) {
  if(inpos==width-1) {
    // Only one pixel remains on the current scanline
    buf[outpos++]=129;
    buf[outpos++]=line[inpos++];
  } else if(line[inpos]==line[inpos+1]) {
    // Homogeneous run
    int i=2;
    while(inpos+i<width && line[inpos+i]==line[inpos] && i<127) i++;
    buf[outpos++]=i|128;
    buf[outpos++]=line[inpos];
    inpos+=i;
  } else {
    // Heterogeneous run
    int i=0;
    int p=outpos++;
    while(inpos+i<width && i<127) {
      if(i>2 && line[inpos+i]==line[inpos+i-1] && line[inpos+i]==line[inpos+i-2]) {
        i-=2;
        outpos-=2;
        break;
      } else if(i==126 && inpos+127<width && line[inpos+i]==line[inpos+i+1]) {
        break;
      }
      buf[outpos++]=line[inpos+i++];
    }
    buf[p]=i;
    inpos+=i;
  }
}

static inline void do_scanline(void) {
  int i,j;
  for(i=0;i<width;i++) {
    fread(buf,1,8,stdin);
    for(j=0;j<256;j++) if(!memcmp(buf,pal[j].c,8)) break;
    line[i]=j;
  }
  inpos=outpos=0;
  while(inpos<width) do_run();
  buf[outpos++]=0;
  putchar(outpos); putchar(outpos>>8);
  fwrite(buf,1,outpos,stdout);
}

int main(int argc,char**argv) {
  int i;
  pal[0].c[6]=pal[0].c[7]=255;
  for(i=1;i<256;i++) memset(pal[i].c,255,8);
  for(i=0;i<argc-1 && i<256;i++) set_color(i,argv[i+1]);
  fread(buf,1,8,stdin);
  if(memcmp("farbfeld",buf,8)) {
    fprintf(stderr,"Not farbfeld\n");
    return 1;
  }
  fread(buf,1,8,stdin);
  if(buf[0] || buf[1] || buf[4] || buf[5] || buf[2]>253) {
    fprintf(stderr,"Picture is too big\n");
    return 1;
  }
  putchar(buf[3]); putchar(buf[2]);
  putchar(buf[7]); putchar(buf[6]);
  putchar(0); putchar(0);
  width=buf[3]|(buf[2]<<8);
  i=buf[7]|(buf[6]<<8);
  while(i--) do_scanline();
  return 0;
}
