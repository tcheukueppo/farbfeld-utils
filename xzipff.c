#if 0
gcc -s -O2 -o ~/bin/xzipff -Wno-unused-result xzipff.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct { unsigned char d[8]; } palette[2];
static int width,height,gcount,goff,ischrset,pos,selnum,inn;
static unsigned char name[13];
static unsigned char selected[13];
static unsigned char head[32];
static unsigned char*sizes;
static unsigned long skip;

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

int main(int argc,char**argv) {
  fread(head,1,32,stdin);
  gcount=(head[0]<<8)|head[1];
  goff=(head[2]<<8)|head[3];
  if(head[4] && head[5]) {
    ischrset=1;
    width=head[4];
    height=head[5];
  }
  if(argc<3 || (argc==4 && argv[3][0]=='n')) {
    fprintf(stderr,"Too few arguments\n");
    return 1;
  }
  parse_color(0,argv[1]);
  parse_color(1,argv[2]);
  if(argc>3) {
    selnum=strtol(argv[3],0,0);
    if(argv[3][0]=='a') selnum=-1;
    if(argv[3][0]=='n') selnum=-2,snprintf(selected,13,"%s            ",argv[4]);
  }
  if(selnum!=-1 && selnum!=-2 && (selnum<goff || selnum>=goff+gcount)) {
    fprintf(stderr,"Picture number out of range (valid range is %d-%d)\n",goff,goff+gcount-1);
    return 1;
  }
  if(ischrset) {
    if(selnum==-2) {
      fprintf(stderr,"Cannot use this selector with this file\n");
      return 1;
    } else if(selnum==-1) {
      goff*=height;
      height*=gcount;
      height+=goff;
    } else {
      skip=(selnum-goff)*((width+15)>>4)*height;
    }
  } else {
    if(selnum==-1) {
      fprintf(stderr,"Cannot use this selector with this file\n");
      return 1;
    }
    sizes=malloc(gcount<<2);
    if(!sizes) {
      fprintf(stderr,"Failed to allocate memory for sizes of %d pictures\n",gcount);
      return 1;
    }
    fread(sizes,4,gcount,stdin);
    while(gcount) {
      width=(sizes[0]<<8)|sizes[1];
      height=(sizes[2]<<8)|sizes[3];
      fread(name,1,12,stdin);
      if(selnum==goff || (selnum==-2 && !memcmp(name,selected,12))) break;
      skip+=((width+15)>>4)*height;
      sizes+=4;
      goff++;
      gcount--;
    }
    if(!gcount) {
      fprintf(stderr,"Picture not found\n");
      return 1;
    }
    skip+=6*(gcount-1);
  }
  while(skip--) fread(name,2,1,stdin);
  fwrite("farbfeld",1,8,stdout);
  putchar(width>>24);
  putchar(width>>16);
  putchar(width>>8);
  putchar(width);
  putchar(height>>24);
  putchar(height>>16);
  putchar(height>>8);
  putchar(height);
  if(selnum==-1) while(goff-- && height--) for(pos=0;pos<width;pos++) fwrite(palette[0].d,1,8,stdout);
  while(height--) {
    pos=width;
    while(pos) {
      fread(name,2,1,stdin);
      selnum=(name[0]<<8)|name[1];
      for(inn=0;inn<16;inn++) if(pos) pos--,fwrite(palette[selnum&(0x8000>>inn)?1:0].d,1,8,stdout);
    }
  }
  return 0;
}
