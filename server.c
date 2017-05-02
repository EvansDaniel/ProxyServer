/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the 
 *     GET method to serve static and dynamic content.
 */
#include "server.h"

int num_requests;

void print_requests() {
  printf("%d\n",num_requests);
}
int main(int argc, char **argv) {

  atexit(print_requests);
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  num_requests = 0;
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
    Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
                port, MAXLINE, 0);
    printf("%s\n","Client accepted");
    process_request(connfd);
    shutdown(connfd, SHUT_RDWR);
    Close(connfd);
  }

}

/*
 * process_request - handle one HTTP request/response transaction
 */
/* $begin process_request */
void process_request(int fd) {
  num_requests++;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  rio_t rio;
  int catcher = 0;

  /* Read request line and headers */
  rio_readinitb(&rio, fd);
  catcher = rio_readlineb(&rio, buf, MAXLINE);

  if(!catcher || catcher == -1){     // Bad read from fd
    perror("Failed to read request line");
    return;
  }

  // Scan in request info and check method for GET
  sscanf(buf, "%s %s %s", method, uri, version);
  if (strcasecmp(method, "GET")) {
    clienterror(fd, method, "501", "Not Implemented", "Sever.c does not implement this method");
    return;
  }
  char* headers = malloc(MAXLINE);
  if(headers == NULL) {
    perror("Failed to allocate headers");
    return;
  }
  memset(headers,0,MAXLINE);


  // retrieve the host and the path
  char* path = parse_path(uri);
  if(path == NULL) {
    fprintf(stderr,"Invalid uri given: %s",uri);
    return;
  }

  add_header(buf,headers);
  if(read_request_headers(&rio,headers) == -1)
    return;

  char* host_p = parse_host(uri);
  char* port_p = "80";
  printf("%s\n","Making connection to server");
  int client_fd = open_clientfd(host_p, port_p);
  if(client_fd < 0) {
    perror("open_clientfd() error");
    return;
  }
  if(rio_writen(client_fd,headers,strlen(headers)) == -1){
    perror("rio_writen() error");
    shutdown(client_fd, SHUT_RDWR);
    return;
  }

  ssize_t numBytes = read_response_write_client(client_fd,fd);
  if(numBytes < 0){
    perror("R_R_W_C FAILED");
    return;
  }

  // Obtain server IP for log entry
  struct sockaddr_in socket_address;
  socklen_t socklen = sizeof(socket_address);
  if(getsockname(client_fd,(SA *) &socket_address, &socklen) == -1){
    perror("getsockname() error");
    shutdown(client_fd, SHUT_RDWR);
    return;
  }

  // Light error handling b/c many handled internally
  if(log_response(&socket_address, uri, numBytes) == -1)
    perror("Failed to log response");


  shutdown(client_fd, SHUT_RDWR);
  Close(client_fd);
  free(headers);
  free(header_value);
  free(path);
  free(host_p);
  printf("%s\n","End Request");
}

int read_request_headers(rio_t *rp,char* headers) {
  char buf[MAXLINE];
  if(rio_readlineb(rp, buf, MAXLINE) == -1){
    perror("rio_readlineb() error");
    return -1;
  }
  add_header(buf,headers);
  //add_header("\r\n",headers);
  char* proxy_conn = "Proxy-Connection:";
  char* conn = "Connection";
  add_header("Connection: close\r\n",headers);
  add_header("Proxy-Connection: close\r\n",headers);
  while (strcmp(buf, "\r\n")) {
    if(rio_readlineb(rp, buf, MAXLINE) == -1){
      perror("rio_readlineb() error");
      return -1;
    }
    if(!strncmp(proxy_conn,buf,strlen(proxy_conn))
       || !strncmp(conn,buf,strlen(conn))) {
      continue;
    }
    add_header(buf,headers);
  }
  return 0;
}
/* $end read_requestheaders */



/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg) {
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  if(rio_writen(fd, buf, strlen(buf)) = -1){
    perror("rio_writen() error");
    return;
  }
  sprintf(buf, "Content-type: text/html\r\n");
  if(rio_writen(fd, buf, strlen(buf)) = -1){
    perror("rio_writen() error");
    return;
  }
  sprintf(buf, "Content-length: %d\r\n\r\n", (int) strlen(body));
  if(rio_writen(fd, buf, strlen(buf)) = -1){
    perror("rio_writen() error");
    return;
  if(rio_writen(fd, buf, strlen(buf)) = -1){
    perror("rio_writen() error");
    return;
  }
  return;
}
/* $end clienterror */
