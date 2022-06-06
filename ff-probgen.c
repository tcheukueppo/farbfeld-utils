#if 0
gcc -s -O2 -o ~/bin/ff-probgen -Wno-unused-result ff-probgen.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int opt[128];
} Config;

static int width,height;
static unsigned short*row;
static Config config[5];

#define fatal(...) do{ fprintf(stderr,__VA_ARGS__); fputc('\n',stderr); exit(1); }while(0)
#define wsize config->opt['w']

static void load_config(int n,char*s) {
  int c,v;
  config[n].opt['c']=0;
  config[n].opt['d']=4;
  config[n].opt['h']=0;
  config[n].opt['j']=0;
  config[n].opt['k']=0;
  config[n].opt['m']=0;
  config[n].opt['r']=0;
  while(*s) {
    c=*s++;
    v=strtol(s,&s,10);
    config[n].opt[c&127]=v;
  }
}

static void process(void) {
  int x,i,v,c;
  int minv=65535;
  int maxv=0;
  int total=0;
  for(i=0;i<wsize;i++) {
    if(row[i]<minv) minv=row[i];
    if(row[i]>maxv) maxv=row[i];
    total+=row[i];
  }
  for(x=0;x<width;x++) {
    if(row[x]<=minv) c=1;
    else if(row[x]>=maxv) c=4;
    else if(row[x]<total/(2*wsize)) c=2;
    else c=3;
    v=row[(x+wsize)%width];
    total+=v-row[x];
    minv=65535;
    maxv=0;
    for(i=0;i<wsize;i++) {
      if(row[(i+x+wsize)%width]<minv) minv=row[(i+x+wsize)%width];
      if(row[(i+x+wsize)%width]>maxv) maxv=row[(i+x+wsize)%width];
    }
    v=(4*row[x])/config[c].opt['d']+random()%(config[c].opt['k']+1-config[c].opt['j'])+config[c].opt['j'];
    if(config[c].opt['m']) v=((config[c].opt['m']*total)/wsize+v*4)/(config[c].opt['m']+4);
    if(config[c].opt['r'] && (random()&255)<config[c].opt['r']) v=0;
    if(v<0) v=config[c].opt['c']&1?v&65535:0;
    if(v>65535) v=config[c].opt['c']&2?v&65535:65535;
    putchar(v>>8); putchar(v);
    putchar(v>>8); putchar(v);
    putchar(v>>8); putchar(v);
    putchar(255); putchar(255);
    row[x]=v;
  }
}

int main(int argc,char**argv) {
  if(argc!=7 && argc!=8) fatal("Incorrect number of arguments");
  config->opt['s']=42;
  config->opt['w']=5;
  width=strtol(argv[1],0,0);
  height=strtol(argv[2],0,0);
  load_config(0,argv[3]);
  if(wsize>=width) fatal("Invalid window size");
  srandom(config->opt['s']);
  row=calloc(sizeof(unsigned short),width);
  if(!row) fatal("Allocation failed");
  while(random()&5) row[random()%width]++;
  load_config(1,argv[4]);
  load_config(2,argv[5]);
  if(argc==7) {
    config[4]=config[3];
    load_config(4,argv[6]);
  } else {
    load_config(3,argv[6]);
    load_config(4,argv[7]);
  }
  fwrite("farbfeld",1,8,stdout);
  putchar(width>>24); putchar(width>>16); putchar(width>>8); putchar(width);
  putchar(height>>24); putchar(height>>16); putchar(height>>8); putchar(height);
  while(height--) process();
  return 0;
}
