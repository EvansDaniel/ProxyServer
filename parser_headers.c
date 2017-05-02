#include "server.h"
#include <poll.h>

char *parse_header_value(char *header) {
  if (header == NULL) {
    return NULL;
  }
  char *header_value = malloc(MAXLINE);
  int i = 0;
  while (*(header + i) != ' ') {
    i++;
  }
  memcpy(header_value, header + i + 1, strlen(header + i + 1));
  return header_value;
}

char *parse_path(char *uri) {
  printf("Uri is cool %s\n", uri);
  // get temp pointer to uri 
  size_t uri_len = strlen(uri);
  char *path = malloc(uri_len);
  if (path == NULL) {
    return NULL;
  }
  memset(path, 0, strlen(path));

  if (!strncmp("http://", uri, 7)) {
    // http protocol
    char *path_start = strchr(uri + 7, '/');
    // if there is no path, concatenate a / and null-terminator
    if (path_start == NULL) {
      strcat(path, "/\0");
      return path;
    }
    // make a copy of the path
    size_t path_len = strlen(path_start) + 1;
    memcpy(path, path_start, path_len);
  } else if (strncmp("https://", uri, 8)) {
    // https protocol
    char *path_start = strchr(uri + 8, '/');
    // if there is no path, concatenate a / and null-terminator
    if (path_start == NULL) {
      strcat(path, "/\0");
      return path;
    }
    // make a copy of the path
    size_t path_len = strlen(path_start) + 1;
    memcpy(path, path_start, path_len);
  } else {
    return NULL;
  }
  strcat(path, "\0");
  printf("My Path: %s\n", path);
  return path;
}


ssize_t read_response_write_client(int clientfd, int proxy_clientfd) {

  int timeout = 3;
  printf("\n%s\n", "Reading response...");

  char server_reply[MAXLINE];
  ssize_t recv_bytes = 0;

  ssize_t numBytes = 0;
  while((recv_bytes = recv(clientfd, server_reply , MAXLINE , 0)) > 0)
  {
    //puts(server_reply);
    if(rio_writen(proxy_clientfd,server_reply,recv_bytes) < 0) {
      perror("rio_writen() error");
      shutdown(proxy_clientfd, SHUT_RDWR);
      return -1;
    }
    numBytes += recv_bytes;
  }

  if(recv_bytes < 0){
    perror("recv() error");
    shutdown(clientfd, SHUT_RDWR);
    return -1;
  }  
  return numBytes;
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
 * @param uri the uri the client is connecting to
 * @return the host as it is parsed from the uri
 */
char *parse_host(char *uri) {
  size_t uri_len = strlen(uri);
  char *t_host = calloc(uri_len, sizeof(char));
  memset(t_host, 0, uri_len);
  if (t_host == NULL) {
    perror("malloc");
    return NULL;
  }
  int offset = 0;
  if (!strncmp(uri, "http://", 7)) {
    offset += 7;
  }
  int temp_offset = offset;
  while (*(uri + offset) != '/') {
    offset++;
  }
  memcpy(t_host, uri + temp_offset, (offset - temp_offset));
  remove_all_chars(t_host, ' ');
  return t_host;
}

void view_string(char *string, int len) {
  printf("Viewing string %s\n", string);
  for (int i = 0; i < len; i++) {
    printf("%d ", (int) *(string + i));
  }
  printf("%s\n", "End viewing string");
}