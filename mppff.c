#if 0
gcc -s -O2 -o ~/bin/mppff -Wno-unused-result mppff.c
exit
#endif

/*
  This program is based on "mpp2bmp" software by Zerkman / Sector One.
  This version is public domain, as permitted by WTFPL.

  Original license information:
   * This program is free software. It comes without any warranty, to
   * the extent permitted by applicable law. You can redistribute it
   * and/or modify it under the terms of the Do What The Fuck You Want
   * To Public License, Version 2, as published by Sam Hocevar. See
   * http://sam.zoy.org/wtfpl/COPYING for more details.
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef union {
  uint32_t rgb;
  struct { uint8_t a, r, g, b; } cp;
} Pixel;

typedef struct {
  int width;
  int height;
  Pixel *array;
  void *ptr;
} PixBuf;

static void mem_error(const void *ptr, const char *name, const char* file, int line) {
  if (!ptr) {
    fprintf(stderr, "%s:%d:allocation for pointer `%s' failed\n",
            file, line, name);
    exit(1);
  }
}

#define MEM_ERROR(ptr) mem_error(ptr, #ptr, __FILE__, __LINE__)

static PixBuf *pixbuf_new(int width, int height) {
  PixBuf *pb = malloc(sizeof(PixBuf));
  MEM_ERROR(pb);
  pb->ptr = malloc(width * height * sizeof(Pixel) + 15);
  MEM_ERROR(pb->ptr);
  intptr_t adr = (intptr_t)pb->ptr;
  adr = (adr+15)&-16;
  pb->array = (Pixel*)adr;
  pb->width = width;
  pb->height = height;
  return pb;
}

typedef struct {
  int id;
  int ncolors;
  int nfixed;
  int border0;
  int x0;
  int (*xinc)(int);
  int xdelta;
  int width;
  int height;
} Mode;

static int xinc0(int c) { return ((c==15)?88:((c==31)?12:((c==37)?100:4))); }
static int xinc1(int c) { return (((c)&1)?16:4); }
static int xinc2(int c) { return 8; }
static int xinc3(int c) { return ((c==15)?112:((c==31)?12:((c==37)?100:4))); }

static const Mode modes[4] = {
  { 0, 54, 0, 1, 33, xinc0, 148, 320, 199 },
  { 1, 48, 0, 1,  9, xinc1, 160, 320, 199 },
  { 2, 56, 0, 1,  4, xinc2, 128, 320, 199 },
  { 3, 54, 6, 0, 69, xinc3, 160, 416, 273 },
};

static int read32(const void *ptr) {
  const unsigned char *p = ptr;
  return (p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
}

static PixBuf *decode_mpp(const Mode *mode, int ste, int extra) {
  int bits = 3 * (3 + ste + extra);
  int palette_size = (mode->ncolors-mode->nfixed)*mode->height;
  int packed_palette_size = ((bits*(mode->ncolors-mode->nfixed-mode->border0*2)*mode->height+15)/8)&-2;
  int extra_palette_bytes = palette_size*sizeof(Pixel) - packed_palette_size;
  int image_size = mode->width/2*mode->height;
  Pixel *pal = malloc(palette_size*sizeof(Pixel));
  unsigned char *img = malloc(image_size);
  unsigned char *p = ((unsigned char*)pal)+extra_palette_bytes;
  int bitbuf = 0, bbnbits = 0;
  int i;

  MEM_ERROR(pal);
  MEM_ERROR(img);
  fread(p, packed_palette_size, 1, stdin);
  fread(img, image_size, 1, stdin);

  for (i = 0; i < palette_size; i++) {
    int x = i % mode->ncolors;
    if (mode->border0 && (x == 0 || x == ((mode->ncolors-1) & -16))) {
      pal[i].rgb = 0;
      continue;
    }
    while (bbnbits < bits) {
      bitbuf = (bitbuf << 8) | *p++;
      bbnbits += 8;
    }
    bbnbits -= bits;
    unsigned short c = (bitbuf >> bbnbits) & ((1<<bits)-1);
    Pixel p;
    p.cp.a = 0;
    switch (bits) {
    case 9:
      p.cp.r = (c>>6);
      p.cp.g = (c>>3)&7;
      p.cp.b = c&7;
      break;
    case 12:
    case 15:
      p.cp.r = ((c>>7)&0xe) | ((c>>11)&1);
      p.cp.g = ((c>>3)&0xe) | ((c>>7)&1);
      p.cp.b = ((c<<1)&0xe) | ((c>>3)&1);
      if (bits == 15) {
        p.cp.r = (p.cp.r << 1) | ((c>>14)&1);
        p.cp.g = (p.cp.g << 1) | ((c>>13)&1);
        p.cp.b = (p.cp.b << 1) | ((c>>12)&1);
      }
      break;
    default:
      abort();
    }
    p.cp.r <<= 8 - (bits/3);
    p.cp.g <<= 8 - (bits/3);
    p.cp.b <<= 8 - (bits/3);
    pal[i] = p;
  }

  PixBuf *pix = pixbuf_new(mode->width, mode->height);
  memset(pix->array, 0, mode->width * mode->height * sizeof(Pixel));
  int k=0, l=0;
  int x, y;
  unsigned short b0=0, b1=0, b2=0, b3=0;
  Pixel palette[16];
  memset(palette, 0, sizeof(Pixel));
  for (y=0; y<mode->height; ++y) {
    Pixel *ppal = pal + y * (mode->ncolors-mode->nfixed);
    int nextx = mode->x0;
    int nextc = 0;
    for (i=mode->nfixed; i<16; ++i)
      palette[i] = *ppal++;
    for (x=0; x<mode->width; ++x) {
      if (x==nextx) {
        palette[nextc&0xf] = *ppal++;
        nextx += mode->xinc(nextc);
        ++nextc;
      }
      if ((x&0xf) == 0) {
        b0 = (img[k+0]<<8) | (img[k+1]);
        b1 = (img[k+2]<<8) | (img[k+3]);
        b2 = (img[k+4]<<8) | (img[k+5]);
        b3 = (img[k+6]<<8) | (img[k+7]);
        k += 8;
      }
      i = ((b3>>12)&8) | ((b2>>13)&4) | ((b1>>14)&2) | ((b0>>15)&1);
      pix->array[l++] = palette[i];
      b0 <<= 1;
      b1 <<= 1;
      b2 <<= 1;
      b3 <<= 1;
    }
  }
  free(pal);
  free(img);
  return pix;
}

int main(int argc, char **argv)
{
  unsigned char header[12];

  fread(header, 12, 1, stdin);
  const Mode *mode = &modes[header[3]];
  int doubl = (header[4]&4) >> 2;
  int extra = (header[4]&2) >> 1;
  int ste = header[4]&1;
  int skip = read32(header+8);

  while(skip--) fgetc(stdin);
  PixBuf *pix = decode_mpp(mode, ste, extra);
  if (doubl) {
    PixBuf *pix2 = decode_mpp(mode, ste, extra);
    int k;
    for (k = 0; k < mode->width*mode->height; ++k) {
      Pixel a = pix->array[k], b = pix2->array[k];
      a.cp.r = ((int)a.cp.r + b.cp.r)/2;
      a.cp.g = ((int)a.cp.g + b.cp.g)/2;
      a.cp.b = ((int)a.cp.b + b.cp.b)/2;
      pix->array[k] = a;
    }
  }

  fwrite("farbfeld",1,8,stdout);
  putchar(mode->width>>24); putchar(mode->width>>16); putchar(mode->width>>8); putchar(mode->width);
  putchar(mode->height>>24); putchar(mode->height>>16); putchar(mode->height>>8); putchar(mode->height);
  int i;
  for(i=0;i<mode->width*mode->height;i++) {
    Pixel a=pix->array[i];
    putchar(a.cp.r); putchar(a.cp.r);
    putchar(a.cp.g); putchar(a.cp.g);
    putchar(a.cp.b); putchar(a.cp.b);
    putchar(255); putchar(255);
  }

  return 0;
}

