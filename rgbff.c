#if 0
gcc -s -O2 -o ~/bin/rgbff -Wno-unused-result rgbff.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long long*mem;
static unsigned char buf[8];
static int width,height;
static int opt[128];

static inline int getchannel(unsigned long long z,int b,int s) {
  unsigned long long m=((1ULL<<b)-1)<<s;
  unsigned short i=(z&m)>>s;
  switch(b) {
    case 0: return 0xFFFF;
    case 1: return i?0xFFFF:0;
    case 2: return 0x5555*i;
    case 3: return (0x9249*i)>>2;
    case 4: return 0x1111*i;
    case 5: return (0x8421*i)>>4;
    case 6: return (0x1041*i)>>2;
    case 7: return (0x4081*i)>>5;
    case 8: return i|(i<<8);
    case 9 ... 15: return (i>>(b-8))|(i<<(16-b));
    case 16: return i;
    default: return (z&m)>>(s+b-16);
  }
}

static inline void getpixel(void) {
  int i;
  unsigned long long z=0;
  fread(buf,1,opt['p'],stdin);
  if(opt['e']) {
    for(i=0;i<opt['p'];i++) z=(z<<8)|buf[i];
  } else {
    for(i=opt['p']-1;i>=0;i--) z=(z<<8)|buf[i];
  }
  i=getchannel(z,opt['R'],opt['r']); buf[0]=i>>8; buf[1]=i;
  i=getchannel(z,opt['G'],opt['g']); buf[2]=i>>8; buf[3]=i;
  i=getchannel(z,opt['B'],opt['b']); buf[4]=i>>8; buf[5]=i;
  i=getchannel(z,opt['A'],opt['a']); buf[6]=i>>8; buf[7]=i;
  if(opt['m'] && i) {
    z=(65535LL*((buf[0]<<8)|buf[1]))/i; z=z>65535?65535:z; buf[0]=z>>8; buf[1]=z;
    z=(65535LL*((buf[2]<<8)|buf[3]))/i; z=z>65535?65535:z; buf[2]=z>>8; buf[3]=z;
    z=(65535LL*((buf[4]<<8)|buf[5]))/i; z=z>65535?65535:z; buf[4]=z>>8; buf[5]=z;
  }
}

static void read_planes(int n,int m) {
  int p=0;
  int b,i,k,s;
  for(k=0;k<m;k++) {
    for(i=0;i<n;i++) {
      s=p+(opt['e']?(opt['p']-1)<<3:0);
      for(b=0;b<opt['p'];b++) {
        mem[i]&=~(255ULL<<s);
        mem[i]|=((unsigned long long)fgetc(stdin))<<s;
        if(opt['e']) s-=8; else s+=8;
      }
    }
    p+=opt['p']<<3;
    for(i=0;i<opt['k'];i++) fgetc(stdin);
    if(feof(stdin)) break;
  }
}

static void getpixel_plane(unsigned long long z) {
  int i;
  i=getchannel(z,opt['R'],opt['r']); buf[0]=i>>8; buf[1]=i;
  i=getchannel(z,opt['G'],opt['g']); buf[2]=i>>8; buf[3]=i;
  i=getchannel(z,opt['B'],opt['b']); buf[4]=i>>8; buf[5]=i;
  i=getchannel(z,opt['A'],opt['a']); buf[6]=i>>8; buf[7]=i;
  if(opt['m'] && i) {
    z=(65535LL*((buf[0]<<8)|buf[1]))/i; z=z>65535?65535:z; buf[0]=z>>8; buf[1]=z;
    z=(65535LL*((buf[2]<<8)|buf[3]))/i; z=z>65535?65535:z; buf[2]=z>>8; buf[3]=z;
    z=(65535LL*((buf[4]<<8)|buf[5]))/i; z=z>65535?65535:z; buf[4]=z>>8; buf[5]=z;
  }
}

int main(int argc,char**argv) {
  int i,j;
  if(argc<3) {
    fprintf(stderr,"Too few arguments\n");
    return 1;
  }
  width=strtol(argv[1],0,0);
  height=strtol(argv[2],0,0);
  opt['e']=1; opt['p']=3; opt['s']=0; opt['S']=0;
  opt['r']=16; opt['g']=8; opt['b']=0; opt['a']=0;
  opt['R']=opt['G']=opt['B']=8; opt['A']=0;
  opt['m']=0; opt['i']=0; opt['k']=0;
  for(i=3;i<argc;i++) if(argv[i][0]) opt[argv[i][0]&127]=strtol(argv[i]+1,0,0);
  if(opt['p']<1 || opt['p']>8) {
    fprintf(stderr,"Invalid number of bytes per pixel\n");
    return 1;
  }
  fwrite("farbfeld",1,8,stdout);
  putchar(width>>24); putchar(width>>16); putchar(width>>8); putchar(width);
  putchar(height>>24); putchar(height>>16); putchar(height>>8); putchar(height);
  if(opt['S']) opt['s']=(opt['S']-(width*opt['p'])%opt['S'])%opt['S'];
  if(!opt['i']) {
    for(i=0;i<height;i++) {
      for(j=0;j<width;j++) {
        getpixel();
        fwrite(buf,1,8,stdout);
      }
      for(j=0;j<opt['s'];j++) fgetc(stdin);
    }
  } else if(opt['i']>0) {
    mem=calloc(sizeof(long long),width);
    if(!mem) {
      fprintf(stderr,"Allocation failed\n");
      return 1;
    }
    for(i=0;i<height;i++) {
      read_planes(width,opt['i']);
      for(j=0;j<opt['s'];j++) fgetc(stdin);
      for(j=0;j<width;j++) {
        getpixel_plane(mem[j]);
        fwrite(buf,1,8,stdout);
      }
    }
  } else if(opt['i']==-1) {
    mem=calloc(sizeof(long long),width*height);
    if(!mem) {
      fprintf(stderr,"Allocation failed\n");
      return 1;
    }
    read_planes(width*height,8);
    for(i=0;i<height;i++) {
      for(j=0;j<width;i++) {
        getpixel_plane(mem[j]);
        fwrite(buf,1,8,stdout);
      }
    }
  } else {
    fprintf(stderr,"Invalid interlace mode %d\n",opt['i']);
    return 1;
  }
  return 0;
}
