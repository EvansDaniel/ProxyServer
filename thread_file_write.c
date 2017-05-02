#include "csapp.h"

volatile int my_count = 0;
sem_t mutex;

void* thread_write_to_file(void *vargp) {
  CRITICAL_CODE_SECTION_BEGIN(&mutex);
  fprintf(vargp,"%d\n",my_count++);
  CRITICAL_CODE_SECTION_END(&mutex);
  return NULL;
}


int main() {
  Sem_init(&mutex,0,1);
  FILE *file = fopen("thread_write.txt","w+");
  if(file == NULL) {
    perror("fopen");
    exit(1);
  }
  pthread_t tid[10];
  printf("Before Thread\n");
  for(int i = 0; i < 10; i++) {
    if(pthread_create(&tid[i], NULL, thread_write_to_file, file) != 0) {
      perror("pthread_create");
    }
    pthread_detach(pthread_self());
  }

  /*for(int i = 0; i < 10; i++) {
    if(pthread_join(tid[i],NULL)) {
      perror("pthread_join");
    }
  }*/
  // move file pointer to start of file, (could use rewind(FILE*) but who likes helper functions?
  if(fseek(file,0,SEEK_SET) < 0) {
    perror("fseek");
  }
  // print out the entire file
  int c;
  do {
    c = fgetc(file);
    if(c == EOF) {
      break;
    }
    printf("%c",(char)c);
  } while(1);

  printf("The count is: %d\n",my_count);
  printf("After Thread\n");
  pthread_exit(0);
}