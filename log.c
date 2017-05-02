#include "server.h"

/*
 * log_response - Create a formatted log entry in proxy.log.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
int log_response(struct sockaddr_in *sockaddr, char *uri, ssize_t size) {
  int fd = open("proxy.log", O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
  if(fd < 0) {
    perror("LogFile error");
    return -1;
  }
  char* responseInfo = malloc(MAXLINE*3);
  if(!responseInfo){
    perror("Failed to allocate memory for response");
    return -1;
  }
  time_t now;
  char time_str[MAXLINE];
  char addr[INET_ADDRSTRLEN];
  unsigned long host;

  /* Get a formatted time string */
  now = time(NULL);
  strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

  if(!inet_ntop(AF_INET, (void*) &(sockaddr->sin_addr), addr, INET_ADDRSTRLEN)){
    perror("inet_ntop Error");
    return -1;
  }

  sprintf(responseInfo, "[%s] %s  %s  %ld\r\n",time_str,addr,uri,size);
  Rio_writen(fd, responseInfo, strlen(responseInfo));

  return 0;
}