#if 0
gcc -s -O2 -o ~/bin/xbmff -Wno-unused-result xbmff.c
exit 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int width,height,form;
static const unsigned char back[8]={255,255,255,255,255,255,255,255};
static const unsigned char fore[8]={0,0,0,0,0,0,255,255};
static int tok,tokv;
static char buf[256];

static int gettoken(void) {
  static int nxt=0;
  char*s;
  again:
  if(feof(stdin)) {
    eof:
    fprintf(stderr,"Unexpected end of file\n");
    exit(1);
  }
  while(nxt!=EOF && nxt<=' ') nxt=fgetc(stdin);
  if(nxt=='/') {
    nxt=fgetc(stdin);
    if(nxt=='*') {
      for(;;) {
        nxt=fgetc(stdin);
        if(nxt==EOF) goto eof;
        if(nxt=='*') {
          while(nxt=='*') nxt=fgetc(stdin);
          if(nxt=='/') {
            nxt=0;
            goto again;
          }
        }
      }
    } else {
      nxt='/';
      goto unex;
    }
  } else if(nxt=='#') {
    if(!fscanf(stdin,"define %255s %i ",buf,&tokv)) {
      fprintf(stderr,"Invalid directive\n");
      exit(1);
    }
    if(s=strrchr(buf,'_')) s++; else s=buf;
    if(!strcmp(s,"width")) width=tokv;
    if(!strcmp(s,"height")) height=tokv;
    nxt=0;
    goto again;
  } else if(nxt=='-' || nxt=='+' || (nxt>='0' && nxt<='9')) {
    ungetc(nxt,stdin);
    if(!fscanf(stdin,"%i",&tokv)) goto unex;
    nxt=0;
    return tok=0;
  } else if(nxt=='{' || nxt=='}' || nxt==';' || nxt=='=' || nxt=='[' || nxt==']' || nxt==',') {
    tok=nxt;
    return nxt=0;
  } else if(fscanf(stdin,"%255[A-Za-z0-9_$]",buf+1)) {
    *buf=nxt;
    nxt=0;
    tok=1;
    if(!strcmp(buf,"static")) return tokv=1;
    if(!strcmp(buf,"unsigned")) return tokv=2;
    if(!strcmp(buf,"char")) return tokv=3;
    if(!strcmp(buf,"short")) return tokv=4;
    if(s=strrchr(buf,'_')) s++; else s=buf;
    if(!strcmp(s,"bits")) return tokv=5;
    fprintf(stderr,"Unexpected word '%s'\n",buf);
    exit(1);
  } else {
    unex:
    fprintf(stderr,"Unexpected character '%c'\n",nxt);
    exit(1);
  }
}

static void badtoken(void) {
  fprintf(stderr,"Invalid X bitmap file\n");
  exit(1);
}

static void scanline(void) {
  int i,j;
  for(i=0;i<width;i+=form) {
    gettoken();
    if(tok) badtoken();
    for(j=0;j<form && i+j<width;j++) fwrite((tokv>>j)&1?fore:back,1,8,stdout);
    gettoken();
    if(tok=='}' && (height || i+form<width)) badtoken();
    if(tok!=',' && tok!='}') badtoken();
  }
}

int main(int argc,char**argv) {
  gettoken();
  if(tok!=1) badtoken();
  if(tokv==1) gettoken();
  if(tok!=1) badtoken();
  if(tokv==2) gettoken();
  if(tok!=1) badtoken();
  if(tokv==3) form=8; else if(tokv==4) form=16; else badtoken();
  gettoken();
  if(tok!=1 || tokv!=5) badtoken();
  gettoken();
  if(tok!='[') badtoken();
  gettoken();
  if(tok==0) gettoken();
  if(tok!=']') badtoken();
  gettoken();
  if(tok!='=') badtoken();
  gettoken();
  if(tok!='{') badtoken();
  if(width<=0 || height<=0) {
    fprintf(stderr,"Invalid width/height\n");
    return 1;
  }
  fwrite("farbfeld",1,8,stdout);
  putchar(width>>24); putchar(width>>16); putchar(width>>8); putchar(width);
  putchar(height>>24); putchar(height>>16); putchar(height>>8); putchar(height);
  while(height--) scanline();
  return 0;
}
