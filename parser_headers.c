#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser_headers.h"
#include "csapp.h"
#include "helpers.h"

char* parse_header_value(char* header) {
  if(header == NULL) {
    return NULL;
  }
  char* header_value = malloc(MAXLINE);
  int i = 0;
  while(*(header + i) != ' ') {
    i++;
  }
  memcpy(header_value,header + i + 1, strlen(header+i+1));
  return header_value;
}

char* parse_path(char* uri) {
  printf("Uri is cool %s\n",uri);
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
  printf("My Path: %s\n",path);
  return path;
}


int read_response_write_client(int clientfd, int proxy_clientfd) {

  printf("%s\n","Reading response:");
  char* content_length = "Content-Length:";
  rio_t response_rio;
  char buf[MAXLINE];
  size_t content_len = 0;

  /*ssize_t rio_error;
  rio_readinitb(&response_rio,clientfd);
  rio_error = rio_readlineb(&response_rio,buf,MAXLINE);
  if(rio_error < 0 ) {
    printf("%s\n","Test 0");
    //return -1;
  }*/

  char server_reply[MAXLINE];
  ssize_t recv_bytes = 0;
  if(write(clientfd,"H",1) < 0) {
    perror("write");
  }
  while((recv_bytes = recv(clientfd, server_reply , MAXLINE , 0)) > 0)
  {
    //puts(server_reply);
    if(rio_writen(proxy_clientfd,server_reply,recv_bytes) < 0) {
      perror("rio_writen");
    }
  }
  printf("%s\n","here");
  if(recv_bytes < 0) {
    perror("recv");
  }
  printf("Recv bytes %d\n",recv_bytes);

  /*// Read response headers and print to stdout
  memset(buf,0,MAXLINE);
  while(strcmp(buf,"\r\n")) {
    rio_error = rio_readlineb(&response_rio, buf, MAXLINE);
    if (rio_error < 0) {
      printf("%s\n", "Test 1");
      break;
    }
    if (strstr(buf, "Content-Length: ")) {
      printf("length:%s\n", parse_header_value(buf));
      content_len = (size_t) atoi(parse_header_value(buf));
    }
    printf("%s",buf);
  }
  // Write the html code to proxy client
  char* html = "</html>";
  char* previous = malloc(MAXLINE);
  int i = 0;

  if(content_len) {
    char entity[content_len];
    rio_error = rio_readnb(&response_rio,entity,content_len);
    if(rio_error < 0) {
      perror("rio_readnb");
    }
    rio_error = rio_writen(proxy_clientfd,entity,content_len);
    if(rio_error < 0) {
      return 0;
    }
  } else {
    while(write(clientfd,"Hello",strlen("Hello")) != -1) {
      rio_error = rio_readlineb(&response_rio,buf,MAXLINE);
      if(rio_error < 0) {
        printf("%s\n","Test 2");
        break;
      }
      rio_error = rio_writen(proxy_clientfd,buf,strlen(buf));
      if(rio_error < 0) {
        break;
      }
    }
  }*/
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

char* new_parse_host(char* uri) {
  size_t uri_len = strlen(uri);
  char* t_host  = malloc(uri_len);
  if(t_host == NULL) {
    perror("malloc");
    return NULL;
  }
  int offset = 0;
  if(!strncmp(uri,"http://",7)) {
    offset += 7;
  }
  int temp_offset = offset;
  while(*(uri+offset) != '/') {
    offset++;
  }
  memcpy(t_host,uri+temp_offset,(offset-temp_offset));
  remove_all_chars(t_host,' ');
  view_string(t_host);
  return t_host;
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
  printf("%s\n",host);
  remove_all_chars(host,'\r');
  remove_all_chars(host,'\n');
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