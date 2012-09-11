#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define offsetof(st, m) \
       ((size_t) ( (char *)&((st *)0)->m - (char *)0 ))

struct nd_header{
  
  char filename[255];
  long offset;             // offset of file, relative to 0
  long length;             // length of file
  
  struct nd_header *next;  // next files header
};

struct nd_file{

  char magic[2];
  int count;                // how many files in pack

  struct nd_header *headers;
};


typedef struct nd_file nd_file;
typedef struct nd_header nd_header;

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

long get_next_offset(nd_file *f)
{

  if(!f){
    die("No nd_file");
  }
  
  nd_header *t = get_last_header(f);
  
  if(!t){
    return sizeof(nd_file);
  }

  return (t->offset + t->length);
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

  char *fname = basename(path);
  strcpy(h->filename,fname);
  
  h->offset = get_next_offset(f);
  h->length = get_file_size(path);
  h->next = NULL;

  f->count++;
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
    free(t);
    h = t;
  }
}

void write_pack(nd_file *f, char *path)
{
  FILE *fp;
  fp = fopen(path,"wb");

  if(!fp){
    die("couldn't open file for writing");
  }
  
  // write the header
  // TODO: Learn to write part of structure
  fwrite(f,sizeof(nd_file),1,fp);
  
  // Loop headers and write
  nd_header *h = f->headers;

  while(h != NULL){
  
    fwrite(h,1,sizeof(nd_header),fp);
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
  int tcount = 0;
  nd_header *h,*t;
  h = t = NULL;

  while(tcount < f->count){
    if(h){
      t = h;
    }
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
  dump_pack(&x);
  free_pack(&x);

  return 0;
}

