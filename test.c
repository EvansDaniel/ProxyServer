#include "parser_headers.h"
#include "stdio.h"
#include "csapp.h"
#include "helpers.h"

int main() {
  char* uri = "http://ums.adtech.de/mapuser?providerid=1053;userid=2l_mzt1T55PCX-LA3wf7xI5Ut5XCBeSSiQfP117i";
  char* host = parse_host(uri);
  printf("%s\n",host);
  view_string(host,strlen(host)+2);
}