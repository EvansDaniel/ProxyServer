/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the 
 *     GET method to serve static and dynamic content.
 */
#include "csapp.h"
#include "parser_headers.h"

void doit(int fd);

char* read_requesthdrs(rio_t *rp, char* headers);

char* parse_path(char* uri);

int parse_uri(char *uri, char *filename, char *cgiargs);

void serve_static(int fd, char *filename, int filesize);

void get_filetype(char *filename, char *filetype);

void serve_dynamic(int fd, char *filename, char *cgiargs);

void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);

size_t find_path_start(char* uri,size_t start);

void read_response_headers();

int main(int argc, char **argv) {
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
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
    Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
                port, MAXLINE, 0);
    printf("%s\n","Client accepted");
    doit(connfd);
    Close(connfd);                                          //line:netp:tiny:doit                                           //line:netp:tiny:close
  }
}
/* $end tinymain */

void remove_all_chars(char* str, char c) {
  char *pr = str, *pw = str;
  while (*pr) {
    *pw = *pr++;
    pw += (*pw != c);
  }
  *pw = '\0';
}

/*void write_bs(int fd) {
  char* lol = "yoyo";
  //** Parse URI from GET request * /
  rio_writen(fd,lol,strlen(lol));
}*/

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd) {
  printf("%s\n","Inside do it ");
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  rio_t rio;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  if (!Rio_readlineb(&rio, buf, MAXLINE))  //line:netp:doit:readrequest
    return;

  sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest
  if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
    clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
    return;
  }
  // retrieve the host and the path
  char* path = parse_path(uri);
  if(path == NULL) {
    fprintf(stderr,"Invalid uri given: %s",uri);
    return;
  }
  char* headers = malloc(MAXLINE);
  if(headers == NULL) {
    fprintf(stderr,"Failed to malloc space for headers");
    return;
  }
  sprintf(headers,"%s %s %s",method, path, "HTTP/1.0\r\n");
  char* host_p = read_requesthdrs(&rio,headers);
  printf("%s",headers);
  char* port_p = "80";
  // this fails why???????????
  /*

   char* host = "hackerrank.com";
   char* port = "80";
   printf("%d\n",strcmp(host,host_p));
   printf("%d\n",strcmp(port_p,port));
   printf("%s %s\n",host,host_p);
   int client_fd = open_clientfd(host_p, port_p);

   */

  // this stuff works perfectly. Why does the www sub domain matter??
  // connect to the remote server
  char* www_host = malloc(strlen(host_p)+4);
  memcpy(www_host,"www.",5);
  //strcat(www_host,host_p);
  printf("Host: %s\n",host_p);
  char* host = "registrar.sewanee.edu";
  /*char* test_host = malloc(strlen(host));
  memcpy(test_host,host,strlen(host));*/
  printf("%s\n","Lol");
  for(int i = 0; i < strlen(host_p)+10; i++) {
    printf("%d ",(int)*(host_p + i));
  }
  printf("%s\n","Making connection to server");
  int client_fd = open_clientfd(host, port_p);
  if(client_fd == -1) {
    perror("client fd problem: ");
  }
  rio_writen(client_fd,headers,strlen(headers));
  printf("%s\n","Headers:");
  printf("%s\n",headers);
  read_response_write_headers(client_fd,fd);

  //shutdown(client_fd, SHUT_RDWR);
  //Close(client_fd);
  shutdown(fd, SHUT_RDWR);
  //write_bs(fd);
  //exit(1);
}
/* $end doit */

/*
 * read_requesthdrs - read HTTP request headers
 */
/* $begin read_requesthdrs */
char* read_requesthdrs(rio_t *rp,char* headers) {
  char buf[MAXLINE];
  Rio_readlineb(rp, buf, MAXLINE);
  sprintf(headers,"%s%s",headers,buf);
  sprintf(headers,"%s%s",headers,"\r\n");
  char *host;
  char *temp_buf = malloc(sizeof buf);
  memcpy(temp_buf,buf,MAXLINE);
  strtok(temp_buf," ");
  host = strtok(NULL," ");
  while (strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    //printf("%s", buf);
    sprintf(headers,"%s%s",headers,buf);
  }
  free(temp_buf);
  // remove the \r\n from the host string
  remove_all_chars(host,'\r');
  remove_all_chars(host,'\n');
  strcat(host,"\0");
  return host;
}
/* $end read_requesthdrs */

/*
 * serve_static - copy a file back to the client 
 */
/* $begin serve_static */
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
