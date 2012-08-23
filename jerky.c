#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

void parse(char *);
enum state {PROGRAM,IDENT,NUM,FUNC,DECL,OP,DELIM};
enum tokens{TIDENT,TCONST,TASSIGN,TDELIM};

struct token{

  char s_val[255];
  int i_val;
  int type;
  int curs;
  
  struct token *next;
};

int main()
{

  char str[255] = "Hello = 10\n func(20,hello);";
  parse(str);

  return 0;
}

void add_list(struct token *root,struct token *next)
{
  struct token *tmp;
  
  if(root->next == NULL)
    root->next = next;
  
  tmp = root->next;
  while(tmp->next != NULL)
    tmp = tmp->next;

  tmp->next = next;
}

void dump_list(struct token *root)
{
  struct token *tmp = root;
  do{

    if(tmp->type == TIDENT){
      printf("Ident: %s\n",tmp->s_val);
    }else if(tmp->type == TCONST){
      printf("Constant: %d\n",tmp->i_val);
    }else if(tmp->type == TASSIGN){
      printf("Assignment\n");
    }else if(tmp->type == TDELIM){
      printf("Func Delim\n");
    }else{

    }
  }while((tmp = tmp->next) != NULL);

}

void free_list(struct token *root)
{
  struct token *tmp = root;
  if(root == NULL) return;

  if(root->next == NULL){
    
    free(root);
    return;
  }

  while((tmp = tmp->next) != NULL)
    free(tmp);
  
  if(root)
    free(root);
}

void parse(char *s)
{
  int state = PROGRAM;
  char c,*p;

  struct token *t_ident;
  struct token *t_num;
  struct token *t_op;
  struct token *root;

  root = malloc(sizeof(struct token));
  root->next = NULL;
  root->type = -1;
  
  for(p = s; p != s+strlen(s); p++){
    c = *p;
    
    if(c == ' ') continue;
    if(c == '\n') continue;
      
    if(c == '='){
      
      t_op = malloc(sizeof(struct token));
      add_list(root,t_op);
      t_op->next = NULL;
      t_op->type = TASSIGN;
      
      state = OP;

    }else if(c == ','){ 
      
      t_op = malloc(sizeof(struct token));
      add_list(root,t_op);
      t_op->next = NULL;
      t_op->type = TDELIM;

      state = TDELIM;

    }if(isalpha(c)){
      
      if(state != IDENT){
        
        t_ident = malloc(sizeof(struct token));
        add_list(root,t_ident);
        t_ident->type = TIDENT;
        t_ident->next = NULL;
        t_ident->curs = 0;
      }
      
      t_ident->s_val[t_ident->curs++] = c;
      t_ident->s_val[t_ident->curs+1] = '\0';

      state = IDENT;
         
    }else if(isdigit(c)){
      
      if(state == IDENT) {

        t_ident->s_val[t_ident->curs++] = c;
        t_ident->s_val[t_ident->curs+1] = '\0';

        continue;
      }
      
      if(state != NUM){
      
        t_num = malloc(sizeof(struct token));
        add_list(root,t_num);
        t_num->type = TCONST;
        t_num->next = NULL;
        t_num->i_val = 0;
        state = NUM;
      }

      t_num->i_val *= 10;
      t_num->i_val += c - '0';

    }else{
     
      state = PROGRAM;
    }
    
  }

  dump_list(root);
  free_list(root);

}
