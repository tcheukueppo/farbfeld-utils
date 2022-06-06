#if 0
gcc -s -O2 -o ~/bin/pbmff -Wno-unused-result pbmff.c
exit 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define fatal(...) do{ fprintf(stderr,__VA_ARGS__); fputc('\n',stderr); exit(1); }while(0)

static int width,height,maxval,form,page;

static int number(void) {
  int c,n;
  spaces: for(;;) {
    c=fgetc(stdin);
    if(c=='#') goto comment;
    if(c>='0' && c<='9') goto number;
    if(c<1 || c>32) fatal("Syntax error");
  }
  comment: for(;;) {
    c=fgetc(stdin);
    if(c==EOF || !c) fatal("Syntax error");
    if(c=='\r' || c=='\n') goto spaces;
  }
  number: for(n=c-'0';;) {
    c=fgetc(stdin);
    if(c==EOF || c<=32) return n;
    if(c<'0' || c>'9') fatal("Syntax error");
    n=10*n+c-'0';
  }
}

static inline void read_picture(void) {
  int x,y,z;
  if(fgetc(stdin)!='P') fatal("Invalid file format");
  form=fgetc(stdin);
  if(form<'1' || form>'6') fatal("Invalid file format");
  width=number();
  height=number();
  if(form!='1' && form!='4') maxval=number();
  if(!page) {
    fwrite("farbfeld",1,8,stdout);
    putchar(width>>24); putchar(width>>16); putchar(width>>8); putchar(width);
    putchar(height>>24); putchar(height>>16); putchar(height>>8); putchar(height);
  }
  switch(form) {
    case '1':
      for(y=0;y<height;y++) for(x=0;x<width;x++) {
        z=fgetc(stdin);
        if(z=='0' || z=='1') {
          z=z=='0'?255:0;
          if(!page) putchar(z),putchar(z),putchar(z),putchar(z),putchar(z),putchar(z),putchar(255),putchar(255);
        } else {
          if(z==EOF || z>32) fatal("Syntax error");
          --x;
        }
      }
      break;
    case '2':
      for(y=0;y<height;y++) for(x=0;x<width;x++) {
        z=(65535LL*number())/maxval;
        if(!page) {
          putchar(z>>8); putchar(z);
          putchar(z>>8); putchar(z);
          putchar(z>>8); putchar(z);
          putchar(255); putchar(255);
        }
      }
      break;
    case '3':
      for(y=0;y<height;y++) for(x=0;x<width<<2;x++) {
        z=3&~x?(65535LL*number())/maxval:65535;
        if(!page) putchar(z>>8),putchar(z);
      }
      break;
    case '4':
      for(y=0;y<height;y++) for(x=0;x<width;x++) {
        if(x&7) z<<=1; else z=fgetc(stdin);
        if(!page) {
          putchar(z&128?0:255); putchar(z&128?0:255);
          putchar(z&128?0:255); putchar(z&128?0:255);
          putchar(z&128?0:255); putchar(z&128?0:255);
          putchar(255); putchar(255);
        }
      }
      break;
    case '5':
      for(y=0;y<height;y++) for(x=0;x<width;x++) {
        z=fgetc(stdin);
        if(maxval&~255) z=(z<<8)|fgetc(stdin);
        z=(65535LL*z)/maxval;
        if(!page) {
          putchar(z>>8); putchar(z);
          putchar(z>>8); putchar(z);
          putchar(z>>8); putchar(z);
          putchar(255); putchar(255);
        }
      }
      break;
    case '6':
      for(y=0;y<height;y++) for(x=0;x<width<<2;x++) {
        if(3&~x) {
          z=fgetc(stdin);
          if(maxval&~255) z=(z<<8)|fgetc(stdin);
          z=(65535LL*z)/maxval;
        } else {
          z=65535;
        }
        if(!page) putchar(z>>8),putchar(z);
      }
      break;
  }
  if(!page) exit(0); else if(form<'4') page=0;
}

int main(int argc,char**argv) {
  page=argc>1?strtol(argv[1],0,0):0;
  do read_picture(); while(page-->0);
  fatal("Invalid page number");
}
