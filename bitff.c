#if 0
gcc -s -O2 -o ~/bin/bitff -Wno-unused-result bitff.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int width;
static int height;
static char format;
static int depth;
static struct { unsigned char d[8]; } palette[256];
static int rowpos,curbyte;
static int parameter;
static unsigned char*buf;

static void parse_color(int i,const char*s) {
  unsigned char d[8];
  switch(strlen(s)) {
    case 6:
      sscanf(s,"%2hhx%2hhx%2hhx",d+0,d+2,d+4);
      d[1]=d[0];
      d[3]=d[2];
      d[5]=d[4];
      d[7]=d[6]=255;
      break;
    case 8:
      sscanf(s,"%2hhx%2hhx%2hhx%2hhx",d+0,d+2,d+4,d+6);
      d[1]=d[0];
      d[3]=d[2];
      d[5]=d[4];
      d[7]=d[6];
      break;
    case 12:
      sscanf(s,"%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",d+0,d+1,d+2,d+3,d+4,d+5);
      d[7]=d[6]=255;
      break;
    case 16:
      sscanf(s,"%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",d+0,d+1,d+2,d+3,d+4,d+5,d+6,d+7);
      break;
    default:
      fprintf(stderr,"Invalid color format (%d)\n",(int)strlen(s));
      exit(1);
  }
  memcpy(palette[i].d,d,8);
}

typedef int(*pixel_func_t)(void);

static int getpixel_B1(void) {
  if(!(rowpos&7)) curbyte=fgetc(stdin);
  if(++rowpos==width) rowpos=0;
  return (curbyte<<=1)&256?1:0;
}

static int getpixel_P1(void) {
  if((rowpos=(rowpos+1)&7)==1) curbyte=fgetc(stdin);
  return (curbyte<<=1)&256?1:0;
}

static int getpixel_W1(void) {
  if(!(rowpos&7)) curbyte=fgetc(stdin);
  if(++rowpos==width) {
    rowpos=0;
    if((width&15) && (width&15)<=8) fgetc(stdin);
  }
  return (curbyte<<=1)&256?1:0;
}

static int getpixel_X1(void) {
  return fgetc(stdin)&1;
}

static int getpixel_b1(void) {
  if(!(rowpos&7)) curbyte=fgetc(stdin)<<1;
  if(++rowpos==width) rowpos=0;
  return (curbyte>>=1)&1;
}

static int getpixel_p1(void) {
  if((rowpos=(rowpos+1)&7)==1) curbyte=fgetc(stdin)<<1;
  return (curbyte>>=1)&1;
}

static int getpixel_w1(void) {
  if(!(rowpos&7)) curbyte=fgetc(stdin)<<1;
  if(++rowpos==width) {
    rowpos=0;
    if((width&15) && (width&15)<=8) fgetc(stdin);
  }
  return (curbyte>>=1)&1;
}

static int getpixel_B2(void) {
  if(!(rowpos&3)) curbyte=fgetc(stdin);
  if(++rowpos==width) rowpos=0;
  return ((curbyte<<=2)>>8)&3;
}

static int initpixel_F2(void) {
  if(!parameter) parameter=1;
  buf=calloc(2,curbyte=parameter);
  return !buf;
}

static int getpixel_F2(void) {
  int i;
  if(curbyte==parameter) {
    curbyte=rowpos=0;
    fread(buf,2,parameter,stdin);
  }
  i=buf[curbyte]&128?1:0;
  i|=buf[parameter+curbyte]&128?2:0;
  buf[curbyte]<<=1;
  buf[parameter+curbyte]<<=1;
  if(++rowpos==8) {
    rowpos=0;
    curbyte++;
  }
  return i;
}

static int getpixel_P2(void) {
  if((rowpos=(rowpos+1)&3)==1) curbyte=fgetc(stdin);
  return ((curbyte<<=2)>>8)&3;
}

static int getpixel_X2(void) {
  return fgetc(stdin)&3;
}

static int getpixel_b2(void) {
  if(!(rowpos&3)) curbyte=fgetc(stdin)<<2;
  if(++rowpos==width) rowpos=0;
  return (curbyte>>=2)&3;
}

static int initpixel_f2(void) {
  if(!parameter) parameter=1;
  buf=calloc(2,curbyte=parameter);
  return !buf;
}

static int getpixel_f2(void) {
  int i;
  if(curbyte==parameter) {
    curbyte=rowpos=0;
    fread(buf,2,parameter,stdin);
  }
  i=buf[curbyte]&1;
  i|=(buf[parameter+curbyte]&1)<<1;
  buf[curbyte]>>=1;
  buf[parameter+curbyte]>>=1;
  if(++rowpos==8) {
    rowpos=0;
    curbyte++;
  }
  return i;
}

static int getpixel_p2(void) {
  if((rowpos=(rowpos+1)&3)==1) curbyte=fgetc(stdin)<<2;
  return (curbyte>>=2)&3;
}

static int getpixel_B4(void) {
  if(!(rowpos&1)) curbyte=fgetc(stdin);
  if(++rowpos==width) rowpos=0;
  return ((curbyte<<=4)>>8)&15;
}

static int getpixel_P4(void) {
  if(rowpos^=1) curbyte=fgetc(stdin);
  return ((curbyte<<=4)>>8)&15;
}

static int initpixel_S4(void) {
  if(!parameter) parameter=1;
  buf=calloc(4,parameter);
  curbyte=parameter<<=1;
  return !buf;
}

static int getpixel_S4(void) {
  int i;
  if(curbyte==parameter) {
    curbyte=rowpos=0;
    fread(buf,4,parameter,stdin);
  }
  i=buf[curbyte]&128?1:0;
  i|=buf[curbyte+1]&128?2:0;
  i|=buf[parameter+curbyte]&128?4:0;
  i|=buf[parameter+curbyte+1]&128?8:0;
  buf[curbyte]<<=1;
  buf[curbyte+1]<<=1;
  buf[parameter+curbyte]<<=1;
  buf[parameter+curbyte+1]<<=1;
  if(++rowpos==8) {
    rowpos=0;
    curbyte+=2;
  }
  return i;
}

static int getpixel_X4(void) {
  return fgetc(stdin)&15;
}

static int getpixel_b4(void) {
  if(!(rowpos&1)) curbyte=fgetc(stdin)<<4;
  if(++rowpos==width) rowpos=0;
  return (curbyte>>=4)&15;
}

static int getpixel_h4(void) {
  // This one is not like the others
  int c=fgetc(stdin)&0x3F;
  if(!rowpos) palette[16]=palette[0];
  if(++rowpos==width) rowpos=0;
  switch(c&0x30) {
    case 0x00: palette[16]=palette[c]; return c;
    case 0x10: palette[16].d[4]=palette[16].d[5]=(c&15)*17; return 16;
    case 0x20: palette[16].d[0]=palette[16].d[1]=(c&15)*17; return 16;
    case 0x30: palette[16].d[2]=palette[16].d[3]=(c&15)*17; return 16;
  }
}

static int getpixel_p4(void) {
  if(rowpos^=1) curbyte=fgetc(stdin)<<4;
  return (curbyte>>=4)&15;
}

static int initpixel_s4(void) {
  if(!parameter) parameter=1;
  buf=calloc(4,parameter);
  curbyte=parameter<<=1;
  return !buf;
}

static int getpixel_s4(void) {
  int i;
  if(curbyte==parameter) {
    curbyte=rowpos=0;
    fread(buf,4,parameter,stdin);
  }
  i=buf[curbyte]&1;
  i|=(buf[curbyte+1]&1)<<1;
  i|=(buf[parameter+curbyte]&1)<<2;
  i|=(buf[parameter+curbyte+1]&1)<<3;
  buf[curbyte]>>=1;
  buf[curbyte+1]>>=1;
  buf[parameter+curbyte]>>=1;
  buf[parameter+curbyte+1]>>=1;
  if(++rowpos==8) {
    rowpos=0;
    curbyte+=2;
  }
  return i;
}

static int getpixel_Z0(void) {
  return fgetc(stdin)&255;
}

static const pixel_func_t pixel_func[512]={
  [0]=getpixel_Z0,
  ['B'*1]=getpixel_B1,
  ['P'*1]=getpixel_P1,
  ['W'*1]=getpixel_W1,
  ['X'*1]=getpixel_X1,
  ['b'*1]=getpixel_b1,
  ['p'*1]=getpixel_p1,
  ['w'*1]=getpixel_w1,
  ['B'*2]=getpixel_B2,
  ['F'*2]=getpixel_F2,
  ['P'*2]=getpixel_P2,
  ['X'*2]=getpixel_X2,
  ['b'*2]=getpixel_b2,
  ['f'*2]=getpixel_f2,
  ['p'*2]=getpixel_p2,
  ['B'*4]=getpixel_B4,
  ['P'*4]=getpixel_P4,
  ['S'*4]=getpixel_S4,
  ['X'*4]=getpixel_X4,
  ['b'*4]=getpixel_b4,
  ['h'*4]=getpixel_h4,
  ['p'*4]=getpixel_p4,
  ['s'*4]=getpixel_s4,
};

static const pixel_func_t pixel_init[512]={
  ['F'*2]=initpixel_F2,
  ['f'*2]=initpixel_f2,
  ['S'*4]=initpixel_S4,
  ['s'*4]=initpixel_s4,
};

int main(int argc,char**argv) {
  int i;
  pixel_func_t get;
  if(argc!=6 && argc!=8 && argc!=20 && !(argc>4 && argv[3][0]=='Z' && argc<261)) {
    fprintf(stderr,"Wrong number of arguments\n");
    return 1;
  }
  width=strtol(argv[1],0,0);
  height=strtol(argv[2],0,0);
  format=argv[3][0]&127;
  parameter=strtol(argv[3]+1,0,0);
  depth=(format=='Z'?0:(argc==6?1:argc==8?2:4));
  get=pixel_func[format*depth];
  if(!get) {
    fprintf(stderr,"Invalid format\n");
    return 1;
  }
  if(pixel_init[format*depth] && pixel_init[format*depth]()) {
    fprintf(stderr,"Failed to initialize\n");
    return 1;
  }
  for(i=4;i<argc;i++) parse_color(i-4,argv[i]);
  fwrite("farbfeld",1,8,stdout);
  putchar(width>>24);
  putchar(width>>16);
  putchar(width>>8);
  putchar(width>>0);
  putchar(height>>24);
  putchar(height>>16);
  putchar(height>>8);
  putchar(height>>0);
  i=width*height;
  while(i--) fwrite(palette[get()].d,1,8,stdout);
  return 0;
}
