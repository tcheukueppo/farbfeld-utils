#if 0
gcc -s -O2 -o ~/bin/jpegff -Wno-unused-result jpegff.c
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_JPEG
#include "stb_image.h"

static unsigned char*pic;
static int width,height,i;

int main(int argc,char**argv) {
  pic=stbi_load_from_file(stdin,&width,&height,&i,4);
  if(!pic || !width || !height) {
    fprintf(stderr,"Error: %s\n",stbi_failure_reason()?:"Unknown error");
    return 1;
  }
  fwrite("farbfeld",1,8,stdout);
  putchar(width>>24);
  putchar(width>>16);
  putchar(width>>8);
  putchar(width>>0);
  putchar(height>>24);
  putchar(height>>16);
  putchar(height>>8);
  putchar(height>>0);
  i=width*height*4;
  while(i--) putchar(*pic),putchar(*pic++);
  return 0;
}
