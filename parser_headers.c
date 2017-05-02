#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

int read_response_write_headers(int clientfd,int proxy_clientfd) {

  rio_t response_rio;
  char buf[MAXLINE];
  int bytes;

  printf("%s\n","Reading response:");
  Rio_readinitb(&response_rio,clientfd);
  Rio_readlineb(&response_rio,buf,MAXLINE);
  printf("%s\n", buf);

  // Read response headers and print to stdout
  memset(buf,0,MAXLINE);
  while(strcmp(buf,"\r\n")) {
    Rio_readlineb(&response_rio,buf,MAXLINE);
    printf("%s",buf);
  }

  // Write the binary code to proxy client
  while(strcmp(buf,"</html>")) {
    Rio_readlineb(&response_rio,buf,MAXLINE);
    bytes += strlen(buf);
    Rio_writen(proxy_clientfd,buf,strlen(buf));
  }
  Close(file_d);
  return bytes;
}

/*
 * log_response - Create a formatted log entry in proxy.log. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void log_response(struct sockaddr_in *sockaddr, char *uri, int size){
  int fd = open("proxy.log", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
  if(fd < 0) {
    perror("LogFile error");
    return;
  }
  char* responseInfo = malloc(MAXLINE*3);   
  time_t now;
  char time_str[MAXLINE];
  char addr[INET_ADDRSTRLEN];
  unsigned long host;

  /* Get a formatted time string */
  now = time(NULL);
  strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

  if(!inet_ntop(AF_INET, sockaddr->sin_addr, addr, INET_ADDRSTRLEN))
    perror("inet_ntop Error");

  sprintf(responseInfo, "[%s] %s  %s  %d\r\n",time_str,addr,uri,size);
  Rio_writen(fd, responseInfo, strlen(responseInfo));
}

void view_string(char* string) {
  printf("Viewing string %s\n",string);
  for(int i = 0; i < strlen(string); i++) {
    printf("%d ",(int)*(string + i));
  }
  printf("%s\n","End viewing string");
}