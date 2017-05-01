#include <string.h>
#include <malloc.h>
#include <netdb.h>
#include <zconf.h>
#include <errno.h>
#include "parser_headers.h"
#include "csapp.h"

char* parse_path(char* uri) {
  // get temp pointer to uri 
  size_t uri_len = strlen(uri);
  char* path = malloc(uri_len);
  if(path == NULL) {
    return NULL;
  }
  memset(path,0,strlen(path));

  if(!strncmp("http://",uri,7)) {
    // http protocol
    char* path_start = strchr(uri + 7,'/');
    // if there is no path, concatenate a / and null-terminator
    if(path_start == NULL) {
      strcat(path,"/\0");
      return path;
    }
    // make a copy of the path
    size_t path_len = strlen(path_start) + 1;
    memcpy(path,path_start,path_len);
  } else if(strncmp("https://",uri,8)) {
    // https protocol
    char* path_start = strchr(uri + 8,'/');
    // if there is no path, concatenate a / and null-terminator
    if(path_start == NULL) {
      strcat(path,"/\0");
      return path;
    }
    // make a copy of the path
    size_t path_len = strlen(path_start) + 1;
    memcpy(path,path_start,path_len);
  } else {
    return NULL;
  }
  strcat(path,"\0");
  return path;
}

void read_response_write_headers(int clientfd,int proxy_clientfd) {
  printf("%s\n","Reading response:");
  FILE *file_d = open("");
  if(file_d == NULL) {
    perror("file_d: ");
  }
  rio_t response_rio;
  char buf[MAXLINE];
  Rio_readinitb(&response_rio,clientfd);
  Rio_readlineb(&response_rio,buf,MAXLINE);
  //Rio_writen(file_d,buf,MAXLINE);
  fprintf(file_d,"%s",buf);
  printf("%s\n",buf);
  // read the headers
  memset(buf,0,MAXLINE);
  while(strcmp(buf,"\r\n")) {
    Rio_readlineb(&response_rio,buf,MAXLINE);
    fprintf(file_d,"%s",buf);
    fprintf(file_d,"%s",buf);
    //Rio_writen(file_d,buf,MAXLINE);
    printf("%s",buf);
    rio_writen(proxy_clientfd,buf,strlen(buf));
  }
  Rio_readlineb(&response_rio,buf,MAXLINE);
  printf("%s\n",buf);
  //Rio_writen(file_d,buf,MAXLINE);
  Rio_readlineb(&response_rio,buf,MAXLINE);
  //Rio_writen(file_d,buf,MAXLINE);
  while(strcmp(buf,"</html>")) {
    Rio_readlineb(&response_rio,buf,MAXLINE);
    printf("%s",buf);
    rio_writen(proxy_clientfd,buf,strlen(buf));
  }
}

void view_string(char* string) {
  printf("Viewing string %s\n",string);
  for(int i = 0; i < strlen(string); i++) {
    printf("%d ",(int)*(string + i));
  }
  printf("%s\n","End viewing string");
}