#if 0
gcc -s -O2 -o ~/bin/ff-sql -Wno-unused-result ff-sql.c sqlite3.o -lm -ldl -lpthread
exit
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"

static int width,height;
static unsigned char*pic;
static unsigned char head[16];
static sqlite3*db;

static void binary_func(sqlite3_context*cxt,int argc,sqlite3_value**argv) {
  double(*f)(double,double)=sqlite3_user_data(cxt);
  double x=sqlite3_value_double(argv[0]);
  double y=sqlite3_value_double(argv[1]);
  sqlite3_result_double(cxt,f(x,y));
}

static void const_func(sqlite3_context*cxt,int argc,sqlite3_value**argv) {
  int*x=sqlite3_user_data(cxt);
  sqlite3_result_int(cxt,*x);
}

static void unary_func(sqlite3_context*cxt,int argc,sqlite3_value**argv) {
  double(*f)(double)=sqlite3_user_data(cxt);
  double x=sqlite3_value_double(*argv);
  sqlite3_result_double(cxt,f(x));
}

typedef struct {
  sqlite3_vtab v;
  char k;
} Vt;

typedef struct {
  sqlite3_vtab_cursor v;
  int x,y;
  char k;
} Cur;

static int xConnect(sqlite3*db1,void*aux,int argc,const char*const*argv,sqlite3_vtab**v0,char**err) {
  Vt*v=sqlite3_malloc(sizeof(Vt));
  if(!v) return SQLITE_NOMEM;
  v->k=1&~argv[2][0];
  sqlite3_declare_vtab(db1,"CREATE TABLE T(X,Y,R,G,B,A)");
  *v0=(void*)v;
  return SQLITE_OK;
}

static int xBestIndex(sqlite3_vtab*v0,sqlite3_index_info*inf) {
  return SQLITE_OK;
}

static int xDisconnect(sqlite3_vtab*v) {
  sqlite3_free(v);
  return SQLITE_OK;
}

static int xOpen(sqlite3_vtab*v0,sqlite3_vtab_cursor**c0) {
  Vt*v=(void*)v0;
  Cur*c=sqlite3_malloc(sizeof(Cur));
  if(!c) return SQLITE_NOMEM;
  c->k=v->k;
  *c0=(void*)c;
  return SQLITE_OK;
}

static int xClose(sqlite3_vtab_cursor*v) {
  sqlite3_free(v);
  return SQLITE_OK;
}

static int xFilter(sqlite3_vtab_cursor*c0,int idxNum,const char*idxStr,int argc,sqlite3_value**argv) {
  Cur*c=(void*)c0;
  c->x=c->y=0;
  return SQLITE_OK;
}

static int xNext(sqlite3_vtab_cursor*c0) {
  Cur*c=(void*)c0;
  if(++c->x==width) c->x=0,++c->y;
  return SQLITE_OK;
}

static int xEof(sqlite3_vtab_cursor*c0) {
  Cur*c=(void*)c0;
  return (c->y==height);
}

static int xColumn(sqlite3_vtab_cursor*c0,sqlite3_context*cxt,int n) {
  Cur*c=(void*)c0;
  int x;
  switch(n) {
    case 0: sqlite3_result_int(cxt,c->x); break;
    case 1: sqlite3_result_int(cxt,c->y); break;
    default:
      x=pic[(c->x+c->y*width)*8LL+1]+(pic[(c->x+c->y*width)*8LL]<<8);
      if(c->k) sqlite3_result_double(cxt,x/65535.0); else sqlite3_result_int(cxt,x);
  }
  return SQLITE_OK;
}

static int xRowid(sqlite3_vtab_cursor*c0,sqlite3_int64*p) {
  Cur*c=(void*)c0;
  *p=c->x+c->y*width;
  return SQLITE_OK;
}

static int xUpdate(sqlite3_vtab*v0,int argc,sqlite3_value**argv,sqlite3_int64*rowid) {
  Vt*v=(void*)v0;
  int x,y,i;
  sqlite3_int64 q;
  unsigned char*p;
  if(argc!=8) return SQLITE_CONSTRAINT_VTAB;
  x=sqlite3_value_int(argv[2]);
  y=sqlite3_value_int(argv[3]);
  if(x<0 || y<0 || x>=width || y>=height) return SQLITE_CONSTRAINT_VTAB;
  if(rowid) *rowid=x+y*width;
  p=pic+(x+y*width)*8LL;
  for(i=0;i<4;i++) {
    if(v->k) {
      q=sqlite3_value_double(argv[i+4])*65535.0;
    } else {
      q=sqlite3_value_int64(argv[i+4]);
    }
    if(q<0) q=0;
    if(q>65535) q=65535;
    p[0]=q>>8;
    p[1]=q;
    p+=2;
  }
  return SQLITE_OK;
}

static const sqlite3_module the_module={
  .iVersion=1,
  .xConnect=xConnect,
  .xDisconnect=xDisconnect,
  .xBestIndex=xBestIndex,
  .xOpen=xOpen,
  .xClose=xClose,
  .xFilter=xFilter,
  .xNext=xNext,
  .xEof=xEof,
  .xColumn=xColumn,
  .xRowid=xRowid,
  .xUpdate=xUpdate,
};

int main(int argc,char**argv) {
  if(argc<2) {
    fprintf(stderr,"Too few arguments\n");
    return 1;
  }
  sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
  sqlite3_config(SQLITE_CONFIG_MEMSTATUS,(int)0);
  if(sqlite3_initialize()) {
    fprintf(stderr,"Error initializing SQLite\n");
    return 1;
  }
  fread(head,1,16,stdin);
  width=(head[8]<<24)|(head[9]<<16)|(head[10]<<8)|head[11];
  height=(head[12]<<24)|(head[13]<<16)|(head[14]<<8)|head[15];
  pic=malloc((width*height)<<3);
  if(!pic) {
    fprintf(stderr,"Allocation failed\n");
    return 1;
  }
  fread(pic,width,height<<3,stdin);
  if(sqlite3_open(":memory:",&db)) {
    fprintf(stderr,"Error opening database\n");
    return 1;
  }
  sqlite3_create_function(db,"ATAN2",1,SQLITE_UTF8|SQLITE_DETERMINISTIC,atan2,binary_func,0,0);
  sqlite3_create_function(db,"COS",1,SQLITE_UTF8|SQLITE_DETERMINISTIC,cos,unary_func,0,0);
  sqlite3_create_function(db,"EXP",1,SQLITE_UTF8|SQLITE_DETERMINISTIC,exp,unary_func,0,0);
  sqlite3_create_function(db,"FMOD",1,SQLITE_UTF8|SQLITE_DETERMINISTIC,fmod,binary_func,0,0);
  sqlite3_create_function(db,"H",0,SQLITE_UTF8|SQLITE_DETERMINISTIC,&height,const_func,0,0);
  sqlite3_create_function(db,"LOG",1,SQLITE_UTF8|SQLITE_DETERMINISTIC,log,unary_func,0,0);
  sqlite3_create_function(db,"POW",1,SQLITE_UTF8|SQLITE_DETERMINISTIC,pow,binary_func,0,0);
  sqlite3_create_function(db,"SIN",1,SQLITE_UTF8|SQLITE_DETERMINISTIC,sin,unary_func,0,0);
  sqlite3_create_function(db,"SQRT",1,SQLITE_UTF8|SQLITE_DETERMINISTIC,sqrt,unary_func,0,0);
  sqlite3_create_function(db,"TAN",1,SQLITE_UTF8|SQLITE_DETERMINISTIC,tan,unary_func,0,0);
  sqlite3_create_function(db,"W",0,SQLITE_UTF8|SQLITE_DETERMINISTIC,&width,const_func,0,0);
  sqlite3_create_module(db,"F",&the_module,0);
  sqlite3_create_module(db,"G",&the_module,0);
  if(sqlite3_exec(db,argv[1],0,0,0)) {
    fprintf(stderr,"%s\n",sqlite3_errmsg(db));
    return 1;
  }
  fwrite(head,1,16,stdout);
  fwrite(pic,width,height<<3,stdout);
  return 0;
}
