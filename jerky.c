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
  
  if(root->next == NULL){
    root->next = next;
    return;  
  }
  
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

struct token *create_token(struct token *root, int type)
{
  struct token *tmp = malloc(sizeof(struct token));

  if(!tmp){
    fprintf(stderr,"Out of memory!\n");
    exit(0);
  }
  
  tmp->next = NULL;
  tmp->type = type;
  tmp->i_val = 0;
  tmp->curs = 0;

  if(root)
    add_list(root,tmp);
  
  return tmp;
}

void parse(char *s)
{
  int state = PROGRAM;
  char c,*p;

  struct token *t_ident;
  struct token *t_num;
  struct token *t_op;
  struct token *root;

  root = create_token(NULL,-1);
  
  for(p = s; p != s+strlen(s); p++){
    c = *p;
    
    if(c == ' ') continue;
    if(c == '\n') continue;

    if(c == '='){

      t_op = create_token(root,TASSIGN);
      state = OP;

    }else if(c == ','){ 
      
      t_op = create_token(root,TDELIM);
      state = TDELIM;

    }if(isalpha(c)){
      
      if(state != IDENT){
        
        t_ident = create_token(root,TIDENT);
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
        
        t_num = create_token(root,TCONST);
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
