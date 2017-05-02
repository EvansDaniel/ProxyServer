#include <time.h>
#include "csapp.h"
#include "log.h"

/*
 * log_response - Create a formatted log entry in proxy.log.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void log_response(struct sockaddr_in *sockaddr, char *uri, int size) {
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

  if(!inet_ntop(AF_INET, (void*) &(sockaddr->sin_addr), addr, INET_ADDRSTRLEN))
    perror("inet_ntop Error");

  sprintf(responseInfo, "[%s] %s  %s  %d\r\n",time_str,addr,uri,size);
  Rio_writen(fd, responseInfo, strlen(responseInfo));
}