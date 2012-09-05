#include <stdio.h>
#include <string.h>
#include <stdlib.h>


struct bank{

  int   size;
  int   cur;
  char *data;
  int   enc;
  int   gz;

};

typedef struct bank bank;

void bank_error(char *msg)
{
  fprintf(stderr,"Bank Error:%s\n",msg);
}

void init_bank(bank *b,int s)
{
  memset(b,0,sizeof(bank));

  if(!b){
    bank_error("failed to allocate bank");
    return;
  }

  b->cur = 0;
  b->enc = 0;
  b->gz  = 0;
  b->size = s;

  b->data = malloc(b->size);

  if(!b->data){
    bank_error("failed to alloc initial buffer");
    return;
  }
  
  memset(b->data,0,b->size);  
}

void delete_bank(bank *b)
{
  if(!b){
    return;
  }

  if(b->data){
    free(b->data);
  }
}

void poke_int(bank *b,int i)
{

  if(!b){
    bank_error("no bank on poke_int");
    return;
  }

  *(&b->data[b->cur]) = i;
  b->cur += sizeof(int);
}

int peek_int(bank *b,int offset)
{
  
  if(!b){
    bank_error("no bank on peek_int");
    return;
  }

  b->cur = offset*sizeof(int);
  return (int)b->data[b->cur];
}


void poke_string(bank *b,char *s)
{

  if(!b){
    bank_error("no bank on poke_string");
    return;
  }

  if(!s){
    bank_error("no string on poke_string");
    return;
  }

  int len = strlen(s);
  char *p = &b->data[b->cur];
  b->cur += len;

  memcpy(p,s,len);
  b->data[b->cur] = '\0';
}

int main()
{
  
  bank *b = malloc(sizeof(bank)); 
  init_bank(b,100);

  poke_int(b,97);
  poke_int(b,100);

  printf("%d\n",(int)*b->data);
  printf("%d\n",peek_int(b,1));

  poke_string(b,"Hello World!");

  printf("%s\n",&(b->data[4]));
  delete_bank(b);
  
  /* I need to learn why when passing bank *b; to a function, then inside the function calling malloc at
   * that pointer, the memory wasn't being allocated */

  return 0;
}

