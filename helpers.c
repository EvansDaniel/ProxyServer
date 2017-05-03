#include "csapp.h"
#include "helpers.h"


/**
 * Remove all c from str
 * @param str string to remove c from
 * @param c character to remove from str
 */
void remove_all_chars(char* str, char c) {
  char *pr = str, *pw = str;
  while (*pr) {
    *pw = *pr++;
    pw += (*pw != c);
  }
  *pw = '\0';
}

// Alias functions that make the crtical sectiosn more apparent
/**
 *
 * @param sem The address of the mutex to lock
 * @return 0 on success, -1 on failure
 */
int CRITICAL_CODE_SECTION_BEGIN(sem_t *sem) {
  return sem_wait(sem);
}
/**
 *
 * @param sem The address of the mutext to unlock
 * @return 0 on success, -1 on failure
 */
int CRITICAL_CODE_SECTION_END(sem_t *sem) {
  return sem_post(sem);
}