#include "server.h"

/**
 * log_response - Create a formatted log entry in proxy.log.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 *
 * @param sockaddr contains the address ip address the request was received from
 * @param uri the uri the client asked us to forward the request to
 * @param size the size of the response received from the peer server
 * @return 0 on successful write to proxy.log, -1 on a failed write
 * If -1 is returned, then the function WILL NOT write to proxy.log
 */
int log_response(struct sockaddr_in *sockaddr, char *uri, ssize_t size) {

  // open a descriptor to the file
  // TODO: pass this to the function, it is inefficient to reopen the descriptor for each thread
  int fd = open("proxy.log", O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
  if(fd < 0) {
    perror("open() error");
    return -1;
  }
  // malloc space for a formatted log string
  char* response_log_string = malloc(MAXLINE*3);
  if(!response_log_string){
    perror("Failed to allocate memory for response");
    return -1;
  }
  time_t now;
  char time_str[MAXLINE];
  char addr[INET_ADDRSTRLEN];

  /* Get a formatted time string */
  now = time(NULL);
  strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

  // convert the sockaddr struct to a dotted ip address
  if(!inet_ntop(AF_INET, (void*) &(sockaddr->sin_addr), addr, INET_ADDRSTRLEN)){
    perror("inet_ntop() error");
    return -1;
  }

  // save the formatted string to response_info and try to write it to
  // proxy.log
  sprintf(response_log_string, "[%s] %s  %s  %ld\r\n",time_str,addr,uri,size);
  if(rio_writen(fd, response_log_string, strlen(response_log_string)) == -1) {
    perror("rio_writen() error");
    Close(fd);
    return -1;
  }

  // close file descriptor and free log string
  Close(fd);
  free(response_log_string);
  return 0;
}