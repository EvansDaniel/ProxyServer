#include "csapp.h"
#include "helpers.h"

void remove_all_chars(char* str, char c) {
  char *pr = str, *pw = str;
  while (*pr) {
    *pw = *pr++;
    pw += (*pw != c);
  }
  *pw = '\0';
}

int min(int a, int b) {
  return a < b ? a : b;
}

int max(int a, int b) {
  return a > b ? a : b;
}

void write_bs(int fd) {
  char* lol = "yoyo";
  //** Parse URI from GET request */
  rio_writen(fd,lol,strlen(lol));
}