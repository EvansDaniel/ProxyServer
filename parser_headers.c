#include "server.h"
#include <poll.h>

/**
 * This function isn't used in the proxy.
 * Given a string of the form header: value,
 * the function will return extract value
 * @param header string of the form "header: value"
 * @return "value" in the header string
 */
char *parse_header_value(char *header) {
  if (header == NULL) {
    return NULL;
  }
  int i = 0;
  // look for the space separating the : and the value
  while (*(header + i) != ' ') {
    i++;
  }
  // malloc the space for the header value
  size_t header_value_len = strlen(header + i + 1);
  char *header_value = malloc(header_value_len);
  // copy from the one after the space, to the end of the string
  memcpy(header_value, header + i + 1, strlen(header + i + 1));
  return header_value;
}

/**
 *
 * @param uri of the form "http://subdomain.domain.top_level_domain/path/to/GET
 * @return returns /path/to/GET
 */
char *parse_path(char *uri) {
  // get temp pointer to uri
  if(uri == NULL) {
    return NULL;
  }
  char* path;
  if (!strncmp("http://", uri, 7)) {
    // http protocol
    char *path_start = strchr(uri + 7, '/');
    // if there is no path, concatenate a / and null-terminator
    if (path_start == NULL) {
      size_t uri_len = strlen(uri);
      // calloc 2 for the "/" and the null-terminator
      path = calloc(2,sizeof(char));
      if (path == NULL) {
        return NULL;
      }
      strcat(path, "/");
      return path;
    }
    // make a copy of the path
    size_t path_len = strlen(path_start) + 1;
    size_t uri_len = strlen(uri);
    path = calloc(uri_len,sizeof(char));
    if (path == NULL) {
      return NULL;
    }
    memcpy(path, path_start, path_len);
  } else { // access protocol other than "http://"
    return NULL;
  }
  return path;
}

/**
 * Reads the response from the peer server and writes the response to
 * proxy_clientfd file descriptor
 * @param clientfd proxy is acting as the client, so clientfd represents our
 * connection to the peer server
 * @param proxy_clientfd represents our connection to the proxy's client
 * @return the number of bytes written to proxy_clientfd
 */
ssize_t read_response_write_client(int clientfd, int proxy_clientfd) {

  printf("%s\n", "Reading response:");
  char server_reply[MAXLINE];
  ssize_t recv_bytes = 0;

  ssize_t numBytes = 0;
  // Retrieve the info from the peer server
  // This will wait at most 1 second before returning either EAGAIN or
  // EWOULDBLOCK because of the setsockopt call in open_clientfd
  // Basically we don't want recv to block because it slows down the
  // the loading of web pages in the browser. As long as the proxy_clientfd
  // remains open, the browser will continue to try to load the page
  // This works well for non-browser clients as well
  while((recv_bytes = recv(clientfd, server_reply , MAXLINE , 0)) > 0)
  {
    // write to the proxy's client
    if(rio_writen(proxy_clientfd,server_reply,recv_bytes) < 0) {
      perror("rio_writen() error");
      return -1;
    }
    // sum up the bytes written as returned from recv()
    numBytes += recv_bytes;
  }
  if(recv_bytes == 0) {
    fprintf(stdout,"Timeout or EOF encountered\n");
  }
  if(recv_bytes < 0){
    perror("recv() error");
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
  strcat(headers,header);
}

/**
 * Extracts the host "subdomain.domain.top_level_domain"
 * from a uri given as "http://subdomain.domain.top_level_domain/path/to/GET
 * @param uri the uri the client is connecting to
 * @return the host as it is parsed from the uri
 */
char *parse_host(char *uri) {
  // malloc the length of the uri
  size_t uri_len = strlen(uri);
  char *t_host = malloc(uri_len);
  if (t_host == NULL) {
    perror("malloc");
    return NULL;
  }
  // best practice
  memset(t_host, 0, uri_len);
  int offset = 0;
  // check if uri starts with http access protocol
  if (!strncmp(uri, "http://", 7)) {
    offset += 7;
  } else {
    fprintf(stderr,"%s","Invalid uri given");
    return NULL;
  }
  // find the next "/" after the access protocol
  int temp_offset = offset;
  while (*(uri + offset) != '/') {
    offset++;
  }
  // memcpy from the end of the access protocol to the first "/"
  memcpy(t_host, uri + temp_offset, (offset - temp_offset));
  // remove any spaces
  remove_all_chars(t_host, ' ');
  return t_host;
}
/**
 * Prints the ascii integer values of the characters
 * of the given string to the desired len
 * @param string printed string
 * @param len amount of string to print
 */
void view_string(char *string, int len) {
  printf("Viewing string %s\n", string);
  for (int i = 0; i < len; i++) {
    printf("%d ", (int) *(string + i));
  }
  printf("%s\n", "End viewing string");
}