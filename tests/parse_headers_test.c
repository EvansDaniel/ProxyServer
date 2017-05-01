#include "../parser_headers.h"
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
int main() {

  char* m = "hello world";
  char* m_x = malloc(90);
  sprintf(m_x,"Hello world: %s",m);
  printf("%s\n",m_x);
  /*char* str = "http://example.com/helloworld";
  char* path = parse_path(str);

  printf("%s\n",path);*/
}