/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the 
 *     GET method to serve static and dynamic content.
 */
#include "csapp.h"
#include "parser_headers.h"
#include "log.h"
#include "server.h"
#include "helpers.h"

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
    process_request(connfd);
    Close(connfd);
  }
}

void process_request(int fd) {
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
  char* host_p = read_request_headers(&rio,headers);

  int client_fd = open_clientfd(host_p,"80");
  if(client_fd < 0) {
    fprintf(stderr,"Error forwarding the request");
    exit(2);
  }

  // write request headers to server
  rio_writen(client_fd,headers,strlen(headers));
  int response_size = read_response_write_client(client_fd,fd);
  log_request("browserIP",uri,response_size);
  write_bs(fd);
}


char* read_request_headers(rio_t *rp,char* headers) {
  char header[MAXLINE];
  Rio_readlineb(rp, header, MAXLINE);
  add_header(header,headers);
  add_header("\r\n",headers);
  // add the connection header
  char* conn = "Connection:";
  char* proxy_conn = "Proxy-Connection:";
  add_header("Connection: close\r\n",headers);
  add_header("Proxy-Connection: close\r\n",headers);

  char* host = parse_host(header);
  // all headers should be added manually before this point
  while (strcmp(header, "\r\n")) {
    Rio_readlineb(rp, header, MAXLINE);
    // skip the connection/proxy-connection header,
    // we will always close connection
    if(!strncmp(header,conn,strlen(conn)) ||
       !strncmp(header,proxy_conn,strlen(proxy_conn)))
    {
      continue;
    }
    printf("%s", header);
    add_header(header,headers);
  }

  // remove the \r\n from the host string
  remove_all_chars(host,'\r');
  remove_all_chars(host,'\n');
  strcat(host,"");
  return host;
}

/*
void serve_static(int fd, char *filename, int filesize) {
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  *//* Send response headers to client *//*
  //get_filetype(filename, filetype);       //line:netp:servestatic:getfiletype
  sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));       //line:netp:servestatic:endserve
  printf("Response headers:\n");
  printf("%s", buf);

  *//* Send response body to client *//*
  srcfd = Open(filename, O_RDONLY, 0);    //line:netp:servestatic:open
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);//line:netp:servestatic:mmap
  Close(srcfd);                           //line:netp:servestatic:close
  Rio_writen(fd, srcp, filesize);         //line:netp:servestatic:write
  Munmap(srcp, filesize);                 //line:netp:servestatic:munmap
}*/


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
