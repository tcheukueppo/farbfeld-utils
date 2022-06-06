#if 0
gcc -s -O2 -o ~/bin/ffbit -Wno-unused-result ffbit.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char head[16];
static int width,height,depth,notfound,transparent;
static char format;
static unsigned char curpix[8];
static struct { unsigned char d[8]; } palette[256];
static int rowpos,curbyte;
static int parameter;
static unsigned char*buf;
static const char*planemap;
static int cwidth,cheight;

static void parse_color(int i,const char*s) {
  unsigned char d[8];
  switch(strlen(s)) {
    case 6:
      sscanf(s,"%2hhx%2hhx%2hhx",d+0,d+2,d+4);
      d[1]=d[0];
      d[3]=d[2];
      d[5]=d[4];
      break;
    case 12:
      sscanf(s,"%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",d+0,d+1,d+2,d+3,d+4,d+5);
      break;
    default:
      fprintf(stderr,"Invalid color format (%d)\n",(int)strlen(s));
      exit(1);
  }
  memcpy(palette[i].d,d,8);
}

typedef void(*pixel_func_t)(int);
typedef int(*init_func_t)(void);

static void putpixel_B1(int x) {
  curbyte|=(x==1?128:0)>>(rowpos++&7);
  if(rowpos==width || !(rowpos&7)) {
    putchar(curbyte);
    curbyte=0;
    if(rowpos==width) rowpos=0;
  }
}

static void putpixel_P1(int x) {
  curbyte|=(x==1?128:0)>>(rowpos++&7);
  if(rowpos==width*height || !(rowpos&7)) {
    putchar(curbyte);
    curbyte=0;
  }
}

static void putpixel_W1(int x) {
  curbyte|=(x==1?128:0)>>(rowpos++&7);
  if(rowpos==width || !(rowpos&7)) {
    putchar(curbyte);
    curbyte=0;
    if(rowpos==width) {
      if((width-1)&8) putchar(0);
      rowpos=0;
    }
  }
}

static void putpixel_X1(int x) {
  putchar(x);
}

static void putpixel_b1(int x) {
  curbyte|=x<<(rowpos++&7);
  if(rowpos==width || !(rowpos&7)) {
    putchar(curbyte);
    curbyte=0;
    if(rowpos==width) rowpos=0;
  }
}

static void putpixel_p1(int x) {
  curbyte|=x<<(rowpos++&7);
  if(rowpos==width*height || !(rowpos&7)) {
    putchar(curbyte);
    curbyte=0;
  }
}

static void putpixel_w1(int x) {
  curbyte|=x<<(rowpos++&7);
  if(rowpos==width || !(rowpos&7)) {
    putchar(curbyte);
    curbyte=0;
    if(rowpos==width) {
      if((width-1)&8) putchar(0);
      rowpos=0;
    }
  }
}

static void putpixel_B2(int x) {
  curbyte|=(x<<6)>>((rowpos++&3)<<1);
  if(rowpos==width || !(rowpos&3)) {
    putchar(curbyte);
    curbyte=0;
    if(rowpos==width) rowpos=0;
  }
}

static int initpixel_F2(void) {
  if(!parameter) parameter=1;
  buf=calloc(2,parameter);
  return !buf;
}

static void putpixel_F2(int x) {
  buf[curbyte]|=x&1;
  buf[parameter+curbyte]|=(x&2)>>1;
  if(++rowpos==8) {
    rowpos=0;
    curbyte++;
    if(curbyte!=parameter) buf[curbyte]=buf[parameter+curbyte]=0;
  } else {
    buf[curbyte]<<=1;
    buf[parameter+curbyte]<<=1;
  }
  if(curbyte==parameter) {
    fwrite(buf,2,parameter,stdout);
    curbyte=rowpos=0;
    *buf=buf[parameter]=0;
  }
}

static void putpixel_P2(int x) {
  curbyte|=(x<<6)>>((rowpos++&3)<<1);
  if(rowpos==width*height || !(rowpos&3)) {
    putchar(curbyte);
    curbyte=0;
  }
}

static void putpixel_X2(int x) {
  putchar(x);
}

static void putpixel_b2(int x) {
  curbyte|=x<<((rowpos++&3)<<1);
  if(rowpos==width || !(rowpos&3)) {
    putchar(curbyte);
    curbyte=0;
    if(rowpos==width) rowpos=0;
  }
}

static int initpixel_f2(void) {
  if(!parameter) parameter=1;
  buf=calloc(2,parameter);
  return !buf;
}

static void putpixel_f2(int x) {
  buf[curbyte]|=(x&1)<<7;
  buf[parameter+curbyte]|=(x&2)<<6;
  if(++rowpos==8) {
    rowpos=0;
    curbyte++;
    if(curbyte!=parameter) buf[curbyte]=buf[parameter+curbyte]=0;
  } else {
    buf[curbyte]>>=1;
    buf[parameter+curbyte]>>=1;
  }
  if(curbyte==parameter) {
    fwrite(buf,2,parameter,stdout);
    curbyte=rowpos=0;
    *buf=buf[parameter]=0;
  }
}

static void putpixel_p2(int x) {
  curbyte|=x<<((rowpos++&3)<<1);
  if(rowpos==width*height || !(rowpos&3)) {
    putchar(curbyte);
    curbyte=0;
  }
}

static void putpixel_B4(int x) {
  curbyte|=(x<<4)>>((rowpos++&1)<<1);
  if(rowpos==width || !(rowpos&1)) {
    putchar(curbyte);
    curbyte=0;
    if(rowpos==width) rowpos=0;
  }
}

static void putpixel_P4(int x) {
  rowpos++;
  if(rowpos&1) curbyte=x<<4; else putchar(curbyte|x);
  if((rowpos&1) && rowpos==width*height) putchar(curbyte);
}

static int initpixel_S4(void) {
  if(!parameter) parameter=1;
  buf=calloc(4,parameter);
  parameter<<=1;
  return !buf;
}

static void putpixel_S4(int x) {
  buf[curbyte]|=x&1;
  buf[curbyte+1]|=(x&2)>>1;
  buf[parameter+curbyte]|=(x&4)>>2;
  buf[parameter+curbyte+1]|=(x&8)>>3;
  if(++rowpos==8) {
    rowpos=0;
    curbyte+=2;
    if(curbyte!=parameter) buf[curbyte]=buf[curbyte+1]=buf[parameter+curbyte]=buf[parameter+curbyte+1]=0;
  } else {
    buf[curbyte]<<=1;
    buf[curbyte+1]<<=1;
    buf[parameter+curbyte]<<=1;
    buf[parameter+curbyte+1]<<=1;
  }
  if(curbyte==parameter) {
    fwrite(buf,2,parameter,stdout);
    curbyte=rowpos=0;
    *buf=buf[1]=buf[parameter]=buf[parameter+1]=0;
  }
}

static void putpixel_X4(int x) {
  putchar(x);
}

static void putpixel_b4(int x) {
  curbyte|=x<<((rowpos++&1)<<2);
  if(rowpos==width || !(rowpos&1)) {
    putchar(curbyte);
    curbyte=0;
    if(rowpos==width) rowpos=0;
  }
}

static void putpixel_h4(int x) {
  // This one is not like the others
  if(!rowpos) palette[16]=palette[0];
  if(++rowpos==width) rowpos=0;
  if(x==notfound && ((x&~15) || memcmp(palette[x].d,curpix,6))) {
    if(palette[16].d[0]==curpix[0] && palette[16].d[2]==curpix[2]) x=(curpix[4]>>4)|0x10;
    if(palette[16].d[4]==curpix[4] && palette[16].d[2]==curpix[2]) x=(curpix[0]>>4)|0x20;
    if(palette[16].d[0]==curpix[0] && palette[16].d[4]==curpix[4]) x=(curpix[2]>>4)|0x30;
  }
  memcpy(palette[16].d,curpix,6);
  putchar(x);
}

static void putpixel_p4(int x) {
  rowpos++;
  if(rowpos&1) curbyte=x; else putchar(curbyte|(x<<4));
  if((rowpos&1) && rowpos==width*height) putchar(curbyte);
}

static int initpixel_s4(void) {
  if(!parameter) parameter=1;
  buf=calloc(4,parameter);
  parameter<<=1;
  return !buf;
}

static void putpixel_s4(int x) {
  buf[curbyte]|=(x&1)<<7;
  buf[curbyte+1]|=(x&2)<<6;
  buf[parameter+curbyte]|=(x&4)<<5;
  buf[parameter+curbyte+1]|=(x&8)<<4;
  if(++rowpos==8) {
    rowpos=0;
    curbyte+=2;
    if(curbyte!=parameter) buf[curbyte]=buf[curbyte+1]=buf[parameter+curbyte]=buf[parameter+curbyte+1]=0;
  } else {
    buf[curbyte]>>=1;
    buf[curbyte+1]>>=1;
    buf[parameter+curbyte]>>=1;
    buf[parameter+curbyte+1]>>=1;
  }
  if(curbyte==parameter) {
    fwrite(buf,2,parameter,stdout);
    curbyte=rowpos=0;
    *buf=buf[1]=buf[parameter]=buf[parameter+1]=0;
  }
}

static void putpixel_Z0(int x) {
  putchar(x);
}

static int initpixel_ZZ(void) {
  if(!cwidth || !cheight) return 1;
  buf=calloc(cwidth,cheight);
  parameter=cwidth*cheight;
  return !buf;
}

static void emit_bit(int b) {
  static unsigned char v=0;
  static int p=0;
  if(b) v|=format=='+'?128>>p:1<<p;
  if(++p==8) {
    putchar(v);
    p=0;
    v=0;
  }
}

static void putpixel_ZZ(int x) {
  const char*p;
  const char*q;
  const char*r;
  int y,z;
  buf[rowpos++]=x;
  if(rowpos!=parameter) return;
  rowpos=0;
  p=planemap;
  while(*p) {
    for(y=0;y<cheight;y++) {
      q=p;
      while(*q) {
        for(x=0;x<cwidth;x++) {
          r=q;
          z=buf[y*cwidth+x];
          while(*r) {
            if(*r>='0' && *r<'8') emit_bit(z&(1<<(*r&7))); else break;
            r++;
          }
        }
        if(*r==',') q=++r;
        if(*r==':' || *r==';' || !*r) break;
      }
    }
    p=r;
    if(*p) p++;
  }
}

static const pixel_func_t pixel_func[512]={
  [0]=putpixel_Z0,
  ['-']=putpixel_ZZ,
  ['+']=putpixel_ZZ,
  ['B'*1]=putpixel_B1,
  ['P'*1]=putpixel_P1,
  ['W'*1]=putpixel_W1,
  ['X'*1]=putpixel_X1,
  ['b'*1]=putpixel_b1,
  ['p'*1]=putpixel_p1,
  ['w'*1]=putpixel_w1,
  ['B'*2]=putpixel_B2,
  ['F'*2]=putpixel_F2,
  ['P'*2]=putpixel_P2,
  ['X'*2]=putpixel_X2,
  ['b'*2]=putpixel_b2,
  ['f'*2]=putpixel_f2,
  ['p'*2]=putpixel_p2,
  ['B'*4]=putpixel_B4,
  ['P'*4]=putpixel_P4,
  ['S'*4]=putpixel_S4,
  ['X'*4]=putpixel_X4,
  ['b'*4]=putpixel_b4,
  ['h'*4]=putpixel_h4,
  ['s'*4]=putpixel_s4,
  ['p'*4]=putpixel_p4,
};

static const init_func_t pixel_init[512]={
  ['-']=initpixel_ZZ,
  ['+']=initpixel_ZZ,
  ['F'*2]=initpixel_F2,
  ['f'*2]=initpixel_f2,
  ['S'*4]=initpixel_S4,
  ['s'*4]=initpixel_s4,
};

int main(int argc,char**argv) {
  int i,j,k;
  pixel_func_t put;
  if(argc!=6 && argc!=8 && argc!=20 && !(argc>4 && (argv[3][0]=='Z' || argv[3][0]=='-' || argv[3][0]=='+') && argc<261)) {
    fprintf(stderr,"Wrong number of arguments\n");
    return 1;
  }
  notfound=strtol(argv[1],0,0);
  transparent=strtol(argv[2],0,0);
  format=argv[3][0]&127;
  if(format=='-' || format=='+') {
    depth=1;
    planemap=argv[3]+1;
    cwidth=notfound;
    cheight=transparent;
    notfound=255;
    transparent=0;
  } else {
    parameter=strtol(argv[3]+1,0,0);
    depth=(format=='Z'?0:(argc==6?1:argc==8?2:4));
  }
  put=pixel_func[format*depth];
  if(!put) {
    fprintf(stderr,"Invalid format\n");
    return 1;
  }
  if(pixel_init[format*depth] && pixel_init[format*depth]()) {
    fprintf(stderr,"Failed to initialize\n");
    return 1;
  }
  for(i=4;i<argc;i++) parse_color(i-4,argv[i]);
  fread(head,1,16,stdin);
  if(memcmp(head,"farbfeld",8)) {
    fprintf(stderr,"Not farbfeld\n");
    return 1;
  }
  width=(head[8]<<24)|(head[9]<<16)|(head[10]<<8)|head[11];
  height=(head[12]<<24)|(head[13]<<16)|(head[14]<<8)|head[15];
  i=width*height;
  while(i--) {
    fread(curpix,1,8,stdin);
    if(curpix[7]<0x80) {
      put(transparent);
    } else {
      k=notfound;
      for(j=0;j<argc-4;j++) if(!memcmp(palette[j].d,curpix,6)) k=j;
      put(k);
    }
  }
  return 0;
}
