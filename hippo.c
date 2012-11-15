// Hippo, a simple http header parser
// Probably broken only written to improve skills
//
// eddie.b eblundell@gmail.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAX_READ 1000
#define HEAD_END "\r\n\r\n"

const char GET[] = "HEAD / HTTP/1.0 \r\n\r\n";


typedef struct _http_value{
  
  char fieldname[255];
  char fieldval[255];
  struct _http_value *next;

} http_value;

typedef struct{

  int fieldcount;
  int length;
  http_value *root;

} http_header;

void ltrim(char *s)
{
  int i,j;
  i = j = 0;

  while(isspace(s[i++]))
    ;

  i--;

  while((s[j++] = s[i++]) != '\0')
    ;

  s[j] = '\0';
}

int parse_http_header(const char *raw, int len, http_header *header)
{
  int i,inright,j,endcount;
  i = j  = inright = endcount = 0;

  header->length = len;
  header->root = NULL;
  header->fieldcount = 0;
  
  http_value *tmp = NULL;
  http_value *current = NULL;
  
  char *buff = malloc(256);

  while(i < len){

    if(raw[i] == ':' && !inright){
      inright = 1;
      i++;

      // Copy into field name
      if(current){
        buff[j] = '\0';
        ltrim(buff);
        strcpy(current->fieldname,buff);
        j = 0;
      }
    }

    if(raw[i] == '\n'){


      header->fieldcount ++;

      // Store right side before moving on
      if(current){
        buff[j] = '\0';
        ltrim(buff);
        strcpy(current->fieldval,buff);
        j = 0;
      }

      tmp = malloc(sizeof(http_value));
      if(current)
        current->next = tmp;
      current = tmp;

      if(!header->root)
        header->root = current;
     

      // Were in left mode now
      inright = 0;
      i++;
    }

    buff[j++] = raw[i];

    i++;
  }

  if(buff)
    free(buff);

  return 0;
}

void free_header(http_header *header)
{
  if(!header)
    return;

  http_value *root = header->root;
  http_value *tmp;

  while(root != NULL){
    tmp = root;
    root = root->next;
    if(root)
      free(tmp);
  }
}

int econnect(const char *addr, int port)
{

  int msocket;
  struct sockaddr_in dest;

  msocket = socket(AF_INET, SOCK_STREAM, 0);
  memset(&dest,0,sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_addr.s_addr = inet_addr(addr);
  dest.sin_port = htons(port);

  connect(msocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
  
  return msocket;
}

int main()
{

  char buffer[MAX_READ+1];
  int len,msocket;

  msocket = econnect("204.62.15.126",80);

  // Send request
  send(msocket, GET,strlen(GET),0);
  
  len = recv(msocket,buffer, MAX_READ, 0);
  buffer[len] = '\0';
  
  http_header header;
  parse_http_header(buffer,len,&header);

  http_value *tmp;
  tmp = header.root;
  
  printf("Field Count:%d\n",header.fieldcount);
  
  do{
    printf("LEFT## %s RIGHT## %s\n",tmp->fieldname,tmp->fieldval);
  }while((tmp = tmp->next));


  free_header(&header);

  close(msocket);

  return 0;
}
