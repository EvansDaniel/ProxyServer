#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <zconf.h>
#include <errno.h>
#include <malloc.h>

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


int read_response_write_client(int clientfd, int proxy_clientfd) {
  printf("%s\n","Reading response:");

  FILE* file_d = fopen("file.txt","w+");
  if(file_d == NULL) {
    perror("file_d: ");
  }


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

  fprintf(file_d,"%s",buf);

  // read the headers
  memset(buf,0,MAXLINE);
  while(strcmp(buf,"\r\n")) {
    Rio_readlineb(&response_rio,buf,MAXLINE);

    fprintf(file_d,"%s",buf);

    printf("%s",buf);
    //rio_writen(proxy_clientfd,buf,strlen(buf));
  }
  while(strcmp(buf,"\r\n")) {
    Rio_readlineb(&response_rio,buf,MAXLINE);
    printf("%s",buf);
    //rio_writen(proxy_clientfd,buf,strlen(buf));
  }
  fflush(file_d);
  return 0;
}

/**
 * Will append the header to the headers
 * @param header the header to add
 * @param headers  the headers that will ultimately be sent to
 * the other server
 */
void add_header(char* header, char* headers) {
  sprintf(headers,"%s%s",headers,header);
}

/**
 *
 * @param host_header of the form "Host: host_domain"
 * @return the parsed host
 */
char* parse_host(char* host_header) {
  char *t_host;
  // tmp malloc for not corrupting buf to parse the host line
  char *temp_buf = malloc(strlen(host_header));
  memcpy(temp_buf,host_header,strlen(host_header));
  strtok(temp_buf," ");
  // the parsed host
  t_host = strtok(NULL," ");
  char* host = malloc(MAXLINE);
  memcpy(host,t_host,MAXLINE);
  free(temp_buf);
  return host;
}

int count_response_bytes() {
  return -1;;
}

/**
 * Trys connecting to the host on the www sub domain
 * @param host_p The host to connect to
 * @return
 */
/*int try_prepending_www(char *host_p) {
  char* www = "www.";
  sprintf(host_p,"%s%s",www,host_p);
  int client_fd = open_clientfd(host_p)
}*/

void view_string(char* string) {
  printf("Viewing string %s\n",string);
  for(int i = 0; i < strlen(string); i++) {
    printf("%d ",(int)*(string + i));
  }
  printf("%s\n","End viewing string");
}