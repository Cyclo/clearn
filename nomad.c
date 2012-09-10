#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    die("Out of memory");
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
}

void free_pack(nd_file *f)
{

  if(!f->headers){
    return;
  }

  nd_header *h = f->headers;
  nd_header *t = NULL;

   do{ 
    //printf("Freeing %s\n",h->filename);  
    t = h->next;
    free(h);
    h = t;
  }while(h && (h->next != NULL));

}

int main()
{

  nd_file file;
  create_pack(&file);
  add_file(&file,"a.out");
  free_pack(&file);

  printf("%s\n",basename("/test/test/test.txt"));
  return 0;
}

