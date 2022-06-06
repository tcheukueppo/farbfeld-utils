#if 0
gcc -s -O2 -o ~/bin/ff-psycho ff-psycho.c -lm
exit
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int w,h;

#define PUSH(z) (stack[sp=(sp+1)&31]=(z))
#define POP(z) (z=stack[sp],sp=(sp-1)&31)
static int calc(const char*s,int xi,int yi) {
  static double stack[32];
  double a,b,c,d,e;
  int sp=1;
  double xf=stack[0]=(2.0*xi)/w-1.0;
  double yf=stack[1]=(2.0*(h-yi))/h-1.0;
  while(*s) switch(*s++) {
    case '0': PUSH(xf); break;
    case '1': PUSH(yf); break;
    case '2': POP(a); POP(b); PUSH((a+b)/2.0); break;
    case '3': POP(a); POP(b); POP(c); PUSH((a+b+c)/3.0); break;
    case '4': POP(a); POP(b); POP(c); POP(d); PUSH((a+b+c+d)/4.0); break;
    case '5': POP(a); POP(b); POP(c); POP(d); POP(e); PUSH((a+b+c+d+e)/5.0); break;
    case '6': POP(a); PUSH(sin(M_PI*a)); break;
    case '7': POP(a); PUSH(cos(M_PI*a)); break;
    case '8': POP(a); POP(b); PUSH(a*b); break;
    case '9': POP(a); POP(b); PUSH(0.5*hypot(a,b)); break;
  }
  POP(a);
  return a>=1.0?65535:a<=-1.0?0:(a+1.0)*32767.5;
}

int main(int argc,char**argv) {
  static int x,y,z;
  if(argc!=6) {
    fprintf(stderr,"Improper number of command line arguments\n");
    return 1;
  }
  w=strtol(argv[1],0,0);
  h=strtol(argv[2],0,0);
  fwrite("farbfeld",1,8,stdout);
  putchar(w>>24); putchar(w>>16); putchar(w>>8); putchar(w);
  putchar(h>>24); putchar(h>>16); putchar(h>>8); putchar(h);
  for(y=0;y<h;y++) for(x=0;x<w;x++) {
    z=calc(argv[3],x,y); putchar(z>>8); putchar(z);
    z=calc(argv[4],x,y); putchar(z>>8); putchar(z);
    z=calc(argv[5],x,y); putchar(z>>8); putchar(z);
    putchar(255); putchar(255);
  }
  return 0;
}
