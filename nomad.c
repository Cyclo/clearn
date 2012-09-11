#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define offsetof(st, m) \
  ((size_t) ( (char *)&((st *)0)->m - (char *)0 ))

struct nd_body{

  char *data;

};

struct nd_header{

  char filename[255];
  long offset;             // offset of file, relative to 0
  long length;             // length of file

  struct nd_header *next;  // next files header
  struct nd_body *body;
};

struct nd_file{

  char magic[2];
  int count;                // how many files in pack

  struct nd_header *headers;
};


typedef struct nd_file nd_file;
typedef struct nd_header nd_header;
typedef struct nd_body nd_body;

void die(char *s)
{
  fprintf(stderr,"%s\n",s);
  exit(1);
}

char *basename(char *path){

  char *p = strrchr(path,'/');
  if(!p){
    return path;
  }

  return p++;
}

void create_pack(nd_file *f)
{
  if(!f){
    die("No nd_file allocated");
  }

  memset(f,0,sizeof(nd_file));

  f->count = 0;
  f->headers = NULL;
  f->magic[0] = 'N';
  f->magic[1] = 'D';
}

nd_header *get_last_header(nd_file *f)
{
  if(!f){
    die("no nd_file");
  }

  nd_header *t = f->headers;

  while(t && (t->next != NULL)){
    t = t->next;
  }

  return t;
}

long get_file_size(char *path)
{
  long size;
  FILE *fp;

  fp = fopen(path,"rb");
  fseek(fp,0,SEEK_END);
  size = ftell(fp);
  fclose(fp);

  return size;
}

nd_body *alloc_body(long l)
{
  nd_body *b = malloc(sizeof(struct nd_body));
  
  if(!b){
    die("out of memory");
  }

  b->data = malloc(l);

  if(!b->data){
    die("out of memory");
  }

  return b;
}

int read_file(char *p,char *b,long l)
{
  if(!p && strlen(p) <= 0){
    return -1;
  }

  FILE *fp;
  fp = fopen(p,"rb");

  if(!fp){
    return -1;
  }

  int status;
  status = fread(b,l,1,fp);
  
  fclose(fp);
  return status;
}


void add_file(nd_file *f, char *path)
{
  if(!f){
    die("No nd_file");
  }

  nd_header *h = malloc(sizeof(nd_header));
  nd_header *last = get_last_header(f);

  if(!h){
    die("out of memory");
  }

  if(last){
    last->next = h;
  }else{
    f->headers = h;
  }

  f->count++;

  char *fname = basename(path);
  strcpy(h->filename,fname);

  h->offset = -1;
  h->length = get_file_size(path);
  h->next = NULL;
  h->body = alloc_body(h->length);

  if(!read_file(path,h->body->data,h->length)){
    die("couldn't read file body");
  }
}

void free_pack(nd_file *f)
{

  if(!f->headers){
    return;
  }

  nd_header *h = f->headers;
  nd_header *t = NULL;

  while(h != NULL){
    t = h->next;

    if(h->body){
      if(h->body->data){
        free(h->body->data);
      }
      free(h->body);
    }
    free(h);
    h = t;
  }
}

void calculate_offsets(nd_file *f)
{

  nd_header *h = f->headers;

  int base,i;
  i = 0;
  base = sizeof(nd_file) + (f->count*sizeof(struct nd_header));
  
  while(h != NULL){
    
    h->offset = base;
    base += h->length;

    h = h->next;
  }
}

void write_pack(nd_file *f, char *path)
{
  FILE *fp;
  fp = fopen(path,"wb");

  if(!fp){
    die("couldn't open file for writing");
  }

  calculate_offsets(f);

  // write the header
  // TODO: Learn to write part of structure
  fwrite(f,sizeof(nd_file),1,fp);

  // Loop headers and write
  nd_header *h = f->headers;

  while(h != NULL){

    fwrite(h,1,sizeof(nd_header),fp);
    h = h->next;
  }

  h = f->headers;

  while(h != NULL){

    fwrite(h->body->data,1,h->length,fp);
    h = h->next;
  }

  fclose(fp);
}

void read_pack(nd_file *f, char *path)
{

  FILE *fp;
  fp = fopen(path,"rb");
  if(!fp){
    die("couldn't open file for reading");
  }

  fread(f,1,sizeof(struct nd_file),fp);
  f->headers = NULL;

  if(strcmp(f->magic,"ND") == 0){

    printf("read_pack: magic confirmed: ND\n");
  }else{
    fprintf(stderr,"magic didn't match ND!=%s\n",f->magic);
    goto cleanup;
  }

  printf("read_pack: pack contains %d files\n",f->count);

  // Read the first header
  nd_header *h,*t;
  int tcount,cur,read;
  
  tcount = cur = read = 0;
  h = t = NULL;

  while(tcount < f->count){

    if(h) t = h;
    h = malloc(sizeof(nd_header));

    if(!h){
      fclose(fp);
      die("out of memory");
    } 

    if(!f->headers){
      f->headers = h;
    }else{
      t->next = h;
    }

    fread(h,1,sizeof(struct nd_header),fp);
    cur = ftell(fp);
    
    printf("read_pack: header[%d] %s offset:%d length: %d\n",tcount,h->filename,h->offset,h->length);

    h->body = alloc_body(h->length);
    fseek(fp,h->offset,SEEK_SET);
    
    read = fread(h->body->data,1,h->length,fp);
    //printf("%s\n",h->body->data);

    // if read != h->length
    
    fseek(fp,cur,SEEK_SET);

    // Store current cursor position
    // Seek to h->offset, read h->length
    // Seek back to cursor

    tcount++;
  }


cleanup:
  fclose(fp);
}


void dump_pack(nd_file *f)
{
  if(!f && !f->headers){
    die("dump_pack: no headers or nd_file");
  }

  nd_header *h = f->headers;

  printf("----HEAD DUMP----\n");
  while(h != NULL){
    printf("Filename:%s\nOffset:%d\nSize:%d\n----------\n",h->filename,h->offset,h->length);
    printf("Body: %s\n",h->body->data);
    h = h->next;
  }
}

int main()
{

  nd_file file;
  create_pack(&file);
  add_file(&file,"a.out");
  add_file(&file,"test.txt");
  write_pack(&file,"out.p");
  free_pack(&file);

  nd_file x = {0};
  read_pack(&x,"out.p");
  //dump_pack(&x);
  free_pack(&x);

  return 0;
}

