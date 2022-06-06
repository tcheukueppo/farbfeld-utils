#if 0
gcc -s -O2 -o ~/bin/zxff -Wno-unused-result zxff.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char mem[0x3000];
static int mode;

static inline void outpix(int a,int p) {
  int i;
  int b=a&0x40?0xFF:0xD7;
  if(!p) a>>=3;
  putchar(i=a&2?b:0); putchar(i);
  putchar(i=a&4?b:0); putchar(i);
  putchar(i=a&1?b:0); putchar(i);
  putchar(255); putchar(255);
}

static void out_picture_0(unsigned char*pix,unsigned char*att) {
  int x,y,z;
  for(y=0;y<8;y++) {
    for(z=0;z<8;z++) {
      for(x=0;x<32;x++) {
        outpix(att[x],pix[x+(z<<8)]&0x80);
        outpix(att[x],pix[x+(z<<8)]&0x40);
        outpix(att[x],pix[x+(z<<8)]&0x20);
        outpix(att[x],pix[x+(z<<8)]&0x10);
        outpix(att[x],pix[x+(z<<8)]&0x08);
        outpix(att[x],pix[x+(z<<8)]&0x04);
        outpix(att[x],pix[x+(z<<8)]&0x02);
        outpix(att[x],pix[x+(z<<8)]&0x01);
      }
    }
    pix+=0x20;
    att+=0x20;
  }
}

static void out_picture_2(unsigned char*pix) {
  int x,y,z;
  for(y=0;y<8;y++) {
    for(z=0;z<8;z++) {
      for(x=0;x<32;x++) {
        outpix(pix[x+(z<<8)+0x1800],pix[x+(z<<8)]&0x80);
        outpix(pix[x+(z<<8)+0x1800],pix[x+(z<<8)]&0x40);
        outpix(pix[x+(z<<8)+0x1800],pix[x+(z<<8)]&0x20);
        outpix(pix[x+(z<<8)+0x1800],pix[x+(z<<8)]&0x10);
        outpix(pix[x+(z<<8)+0x1800],pix[x+(z<<8)]&0x08);
        outpix(pix[x+(z<<8)+0x1800],pix[x+(z<<8)]&0x04);
        outpix(pix[x+(z<<8)+0x1800],pix[x+(z<<8)]&0x02);
        outpix(pix[x+(z<<8)+0x1800],pix[x+(z<<8)]&0x01);
      }
    }
    pix+=0x20;
  }
}

static void out_picture_6(unsigned char*pix) {
  int x,y,z;
  for(y=0;y<8;y++) {
    for(z=0;z<8;z++) {
      for(x=0;x<32;x++) {
        outpix(mode,pix[x+(z<<8)]&0x80);
        outpix(mode,pix[x+(z<<8)]&0x40);
        outpix(mode,pix[x+(z<<8)]&0x20);
        outpix(mode,pix[x+(z<<8)]&0x10);
        outpix(mode,pix[x+(z<<8)]&0x08);
        outpix(mode,pix[x+(z<<8)]&0x04);
        outpix(mode,pix[x+(z<<8)]&0x02);
        outpix(mode,pix[x+(z<<8)]&0x01);
        outpix(mode,pix[x+(z<<8)+0x1800]&0x80);
        outpix(mode,pix[x+(z<<8)+0x1800]&0x40);
        outpix(mode,pix[x+(z<<8)+0x1800]&0x20);
        outpix(mode,pix[x+(z<<8)+0x1800]&0x10);
        outpix(mode,pix[x+(z<<8)+0x1800]&0x08);
        outpix(mode,pix[x+(z<<8)+0x1800]&0x04);
        outpix(mode,pix[x+(z<<8)+0x1800]&0x02);
        outpix(mode,pix[x+(z<<8)+0x1800]&0x01);
      }
    }
    pix+=0x20;
  }
}

int main(int argc,char**argv) {
  fread(mem,1,0x1B00,stdin);
  if(fread(mem+0x1B00,1,0x1500,stdin)>0) {
    mode=fgetc(stdin);
    if(mode==EOF) mode=2;
  } else {
    mode=0;
  }
  switch(mode&7) {
    case 0:
      fwrite("farbfeld\0\0\x01\x00\0\0\x00\xC0",1,16,stdout);
      out_picture_0(mem+0x0000,mem+0x1800);
      out_picture_0(mem+0x0800,mem+0x1900);
      out_picture_0(mem+0x1000,mem+0x1A00);
      return 0;
    case 2:
      fwrite("farbfeld\0\0\x01\x00\0\0\x00\xC0",1,16,stdout);
      out_picture_2(mem+0x0000);
      out_picture_2(mem+0x0800);
      out_picture_2(mem+0x1000);
      return 0;
    case 6:
      fwrite("farbfeld\0\0\x02\x00\0\0\x00\xC0",1,16,stdout);
      mode=(((mode>>3)&7)*9)^0170;
      out_picture_6(mem+0x0000);
      out_picture_6(mem+0x0800);
      out_picture_6(mem+0x1000);
      return 0;
    default:
      fprintf(stderr,"Currently not implemented\n");
      return 1;
  }
}
