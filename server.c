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
    Close(connfd);
  }

}

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void process_request(int fd) {
  num_requests++;
  printf("%s\n","Inside do it ");
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  rio_t rio;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  if (!Rio_readlineb(&rio, buf, MAXLINE))
    return;
  sscanf(buf, "%s %s %s", method, uri, version);
  if (strcasecmp(method, "GET")) {
    clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
    return;
  }
  char* headers = malloc(MAXLINE);
  memset(headers,0,MAXLINE);
  if(headers == NULL) {
    perror("malloc");
  }
  // retrieve the host and the path
  char* path = parse_path(uri);
  if(path == NULL) {
    fprintf(stderr,"Invalid uri given: %s",uri);
    return;
  }
  add_header(buf,headers);
  read_request_headers(&rio,headers);
  char* host_p = parse_host(uri);
  char* port_p = "80";
  printf("%s\n","Making connection to server");
  int client_fd = open_clientfd(host_p, port_p);
  if(client_fd < 0) {
    printf("%s\n","ClientFD FAILED");
    return;
  }
  rio_writen(client_fd,headers,strlen(headers));

  ssize_t numBytes = read_response_write_client(client_fd,fd);
  struct sockaddr_in socket_address;
  socklen_t socklen = sizeof(socket_address);

  if(getsockname(client_fd,(SA *) &socket_address, &socklen) == -1)
    perror("getsockname() error");
  if (log_response(&socket_address, uri, numBytes) == -1)
    perror("Failed to log response");


  shutdown(fd, SHUT_RDWR);
  printf("%s\n","End request");
}


void read_request_headers(rio_t *rp,char* headers) {
  char buf[MAXLINE];
  Rio_readlineb(rp, buf, MAXLINE);
  add_header(buf,headers);
  //add_header("\r\n",headers);
  char* proxy_conn = "Proxy-Connection:";
  char* conn = "Connection";
  add_header("Connection: close\r\n",headers);
  add_header("Proxy-Connection: close\r\n",headers);
  while (strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    if(!strncmp(proxy_conn,buf,strlen(proxy_conn))
       || !strncmp(conn,buf,strlen(conn))) {
      continue;
    }
    add_header(buf,headers);
  }
}
/* $end read_requesthdrs */

void serve_static(int fd, char *filename, int filesize) {
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);       //line:netp:servestatic:getfiletype
  sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));       //line:netp:servestatic:endserve
  printf("Response headers:\n");
  printf("%s", buf);

  /* Send response body to client */
  srcfd = Open(filename, O_RDONLY, 0);    //line:netp:servestatic:open
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);//line:netp:servestatic:mmap
  Close(srcfd);                           //line:netp:servestatic:close
  Rio_writen(fd, srcp, filesize);         //line:netp:servestatic:write
  Munmap(srcp, filesize);                 //line:netp:servestatic:munmap
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) {
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else
    strcpy(filetype, "text/plain");
}
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs) {
  char buf[MAXLINE], *emptylist[] = {NULL};

  /* Return first part of HTTP response */
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0) { /* Child */ //line:netp:servedynamic:fork
    /* Real server would set all CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1); //line:netp:servedynamic:setenv
    Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */ //line:netp:servedynamic:dup2
    Execve(filename, emptylist, environ); /* Run CGI program */ //line:netp:servedynamic:execve
  }
  Wait(NULL); /* Parent waits for and reaps child */ //line:netp:servedynamic:wait
}
/* $end serve_dynamic */

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
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int) strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */
