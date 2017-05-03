/**
 * Team Members: Daniel Evans, Adrian Preston, and Blaise Iradukunda
 */
#include "server.h"

sem_t log_mutex;
sem_t uri_mutex;
char* global_uri;

char* get_client_ip(int clientfd) {
  struct sockaddr_in addr;
  socklen_t addr_size = sizeof(struct sockaddr_in);
  int res = getsockname(clientfd, (struct sockaddr *)&addr, &addr_size);
  if(res < 0) {
    perror("getpeername");
    return NULL;
  }
  char *clientip = malloc(INET_ADDRSTRLEN);
  const char* temp = inet_ntop(AF_INET, (void*)&(&addr)->sin_addr,clientip,INET_ADDRSTRLEN);
  memcpy(clientip, temp,INET_ADDRSTRLEN);
  return clientip;
}

/**
 * Contains the main server loop for the program. Accepts
 * connections received by clients and creates "child threads"
 * to handle the request, write the respective response (if any), and
 * log the request to the file "proxy.log"
 * @param argc Must always be 2
 * @param argv the first arg is expected to be a port number within the valid
 * port number range [0 to 65536]
 * @return returns 0 iff a valid, unused port is given as argument
 */
int main(int argc, char **argv) {
  sem_init(&log_mutex,0,1);
  sem_init(&uri_mutex,0,1);
  global_uri = malloc(MAXLINE);
  memset(global_uri,0,MAXLINE);
  int listenfd, *connfd;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  // if not given a port, exit
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  // open a listening socket connection on the given port
  // can exit on failure for invalid port number or if
  // given a used port
  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    // malloc new space for each connfd, required for correctly handling
    // accept connections when threading multiple requests
    // otherwise, each thread is given the same/overwritten connfd pointer
    connfd = malloc(sizeof(int));
    // accept the connection
    *connfd = accept(listenfd, (SA *) &clientaddr, &clientlen);
    //printf("Client IP %s\n",get_client_ip(*connfd));
    if(connfd < 0) {
      perror("accept - Unable to accept connection");
    } else {
      printf("%s\n", "Client accepted");
      pthread_t thread;
      if(pthread_create(&thread, NULL, process_request, (void *) connfd) < 0) {
        perror("pthread_create failed - Unable to service request");
        continue;
      }
    }
  }
  free(global_uri);
  // call pthread_exit to wait for other threads to finish their work
  pthread_exit(NULL);
  return 0;
}

/**
 *
 * @param connfd
 * @return
 */
void* process_request(void* connfd) {
  // detach the thread so we aren't required to join to reclaim memory
  pthread_detach(pthread_self());
  int fd = *(int*)connfd;
  printf("%s\n","Processing request...");
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  // good practice to memset all memory
  memset(buf,0,MAXLINE);
  memset(method,0,MAXLINE);
  memset(uri,0,MAXLINE);
  memset(version,0,MAXLINE);
  rio_t rio;
  // catches rio function return values
  ssize_t catcher = 0;

  // read first line of request (should be of type METHOD URI VERSION,
  // if a valid request
  rio_readinitb(&rio, fd);
  catcher = rio_readlineb(&rio, buf, MAXLINE);

  if(catcher < 0){     // Bad read from fd
    perror("Failed to read request line");
    cleanup(-1,NULL,NULL,NULL,connfd);
    return NULL;
  }

  // Parse out the fields of request line
  // Check that we were asked to perform a GET request
  sscanf(buf, "%s %s %s", method, uri, version);
  // TODO: try to optimize the requests by ignoring requests that have already been performed by this client??


  CRITICAL_CODE_SECTION_BEGIN(&uri_mutex);
  if(!strncmp(uri,global_uri,strlen(uri))) {
    cleanup(-1,NULL,NULL,NULL,connfd);
    printf("%s\n","Oh shit");
    CRITICAL_CODE_SECTION_END(&uri_mutex);
    return NULL;
  }
  CRITICAL_CODE_SECTION_END(&uri_mutex);
  memcpy(global_uri,uri,strlen(uri));


  if (strcasecmp(method, "GET")) {
    clienterror(fd, method, "501", "Not Implemented", "Server.c does not implement this method");
    cleanup(-1,NULL,NULL,NULL,connfd);
    return NULL;
  }
  // Space for the headers to be forwarded to the peer server
  char* headers = malloc(MAXLINE);
  if(headers == NULL) {
    perror("Failed to allocate headers");
    cleanup(-1,NULL,NULL,NULL,connfd);
    return NULL;
  }
  // good practice
  memset(headers,0,MAXLINE);

  // retrieve the path from the uri, check validity and cleanup if bad
  char* path = parse_path(uri);
  if(path == NULL) {
    // TODO: make this a client_error() call?
    fprintf(stderr,"Invalid uri given: %s",uri);
    cleanup(-1,headers,path,NULL,connfd);
    return NULL;
  }

  // add the request header with HTTP/1.0
  sprintf(headers,"%s %s %s\r\n",method,path,"HTTP/1.0");
  if(read_request_headers(&rio,headers) == -1) {
    cleanup(-1,headers,path,NULL,connfd);
    return NULL;
  }

  // retrieve the host from the uri
  char* host_p = parse_host(uri);
  if(host_p == NULL) {
    fprintf(stderr,"%s\n","parse_host failed to alloc memory");
    cleanup(-1,headers,path,host_p,connfd);
  }
  // we only handle http:// access protocol
  // One of the reasons parse_host returns NULL is if the uri has an access protocol
  // other than "http://", so we know that if we got to this point
  // the uri contained access protocol "http://" and that we can make a request on port 80
  char* port_p = "80";
  printf("%s\n","Making connection to server");

  // open a connection to the peer server
  int peer_serverfd = open_clientfd(host_p, port_p);
  if(peer_serverfd < 0) {
    perror("open_clientfd() error");
    cleanup(peer_serverfd,headers,path,host_p,connfd);
    return NULL;
  }

  // Forward/Write headers saved earlier to the peer server
  if(rio_writen(peer_serverfd,headers,strlen(headers)) == -1){
    perror("rio_writen() error");
    cleanup(peer_serverfd,headers,path,host_p,connfd);
    return NULL;
  }

  // read the response and write it the client, save the number of bytes written
  // for the log file later
  ssize_t numBytes = read_response_write_client(peer_serverfd,fd);
  if(numBytes < 0){
    perror("R_R_W_C FAILED");
    cleanup(peer_serverfd,headers,path,host_p,connfd);
    return NULL;
  }

  // Obtain server IP for log entry
  struct sockaddr_in socket_address;
  socklen_t socklen = sizeof(socket_address);

  if(getsockname(peer_serverfd,(SA *) &socket_address, &socklen) == -1) {
    perror("getsockname() error");
    cleanup(peer_serverfd,headers,path,host_p,connfd);
    return NULL;
  }

  // Writing to the file is a critical code section
  // Without the mutex, multiple threads can write to the file at the same time
  if(CRITICAL_CODE_SECTION_BEGIN(&log_mutex) < 0) {
    perror("CRITICAL_CODE_SECTION_BEGIN - Failed to lock log mutex");
  }
  if (log_response(&socket_address, uri, numBytes) == -1)
    perror("Failed to log response");
  if(CRITICAL_CODE_SECTION_END(&log_mutex) < 0) {
    perror("CRITICAL_CODE_SECTION_BEGIN - Failed to unlock log mutex");
  }

  // The request has been fulfilled so clean up the resources
  // connfd cannot be closed in the main server loop b/c then our
  // connection to the client will immediately close after calling
  // pthread_create with the process_request and connfd as arguments
  printf("%s\n","End Request");
  cleanup(peer_serverfd,headers,path,host_p,connfd);
  return NULL;
}

/**
 *
 * @param peer_serverfd the descriptor representing the connection between
 * the proxy and the server the client is asking us to forward the request to
 * @param headers the headers to be forwarded to the peer server
 * @param path the path parsed out of the uri
 * @param host_p the host parsed from the uri
 * @param connfd the descriptor representing the connection between
 * the proxy and the client
 */
void cleanup(int peer_serverfd,char* headers,char* path,char* host_p,void* connfd) {
  if(peer_serverfd != -1)
    Close(peer_serverfd);
  free(headers);
  free(path);
  free(host_p);
  Close(*(int*)connfd);
  free(connfd);
}

/**
 *
 * @param rp the rio data structure representing our connection with the
 * clent
 * @param headers the user-allocated headers to ultimately write to the peer server
 * @return 0 on success and -1 on failed reads of the descriptor bound to rp
 */
int read_request_headers(rio_t *rp,char* headers) {
  char header[MAXLINE];
  if(rio_readlineb(rp, header, MAXLINE) == -1){
    perror("rio_readlineb() error");
    return -1;
  }
  add_header(header,headers);
  char* proxy_conn = "Proxy-Connection:";
  char* conn = "Connection";
  char* user_agent_header = "User-Agent:";
  char* user_agent = "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:53.0) Gecko/20100101 Firefox/53.0";
  add_header("Connection: close\r\n",headers);
  add_header("Proxy-Connection: close\r\n",headers);
  int has_user_agent = 0;
  while (strcmp(header, "\r\n")) {
    if(rio_readlineb(rp, header, MAXLINE) == -1){
      perror("rio_readlineb() error");
      return -1;
    }
    if(!strncmp(proxy_conn,header,strlen(proxy_conn))
       || !strncmp(conn,header,strlen(conn))) {
      continue;
    }
    if(!strncmp(user_agent_header,header,strlen(user_agent_header))) {
      has_user_agent = 1;
    }
    add_header(header,headers);
  }
  // add the hard_coded user agent string (Mozilla Firfox) string
  // if a user agent isn't given in the client's request
  if(!has_user_agent) {
    add_header(user_agent,headers);
  }
  return 0;
}

/**
 * Helper function to show fatal errors that the proxy
 * cannot handle
 * @param fd the descriptor to write the error message to
 * @param cause Reason why error occured
 * @param errnum Response code error
 * @param shortmsg Short version of why fatal error occurred
 * @param longmsg Long version of why fatal error occurred
 */
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg) {
  char buf[MAXLINE], body[MAXBUF];
  char temp[MAXLINE];
  memset(temp,0,MAXLINE);
  memset(body,0,MAXLINE);
  memset(buf,0,MAXLINE);
  /* Build the HTTP response body */
  strcat(body,"<html><title>Tiny Error</title>");
  strcat(body,"<body bgcolor=""ffffff"">\r\n");
  sprintf(temp, "%s: %s\r\n", errnum, shortmsg);
  strcat(body,temp);
  sprintf(temp, "<p>%s: %s\r\n", longmsg, cause);
  strcat(body,temp);
  strcat(body,"<hr><em>The Tiny Web server</em>\r\n");

  /* Print the HTTP response */
  sprintf(temp, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  strcat(buf,temp);
  if(rio_writen(fd, buf, strlen(buf)) == -1){
    perror("rio_writen() error");
    return;
  }
  sprintf(buf, "Content-type: text/html\r\n");
  if(rio_writen(fd, buf, strlen(buf)) == -1){
    perror("rio_writen() error");
    return;
  }
  sprintf(buf, "Content-length: %d\r\n\r\n", (int) strlen(body));
  if(rio_writen(fd, buf, strlen(buf)) == -1) {
    perror("rio_writen() error");
    return;
  }
  if(rio_writen(fd, body, strlen(body)) == -1) {
    perror("rio_writen() error");
    return;
  }
}
