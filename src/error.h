#ifndef _ERROR_H_
#define _ERROR_H_

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// i really tooooo lazy to google murmurhash
static inline size_t djb2Hash(const char Str[]) {
  size_t hash = 5381;
  const char *ch = Str;

  for (; *ch != '\0'; ++ch) {
    hash = ((hash << 5) + hash) + *ch;
  }
  return hash;
}

#define __ERROR_H_BUFSIZE 4096

typedef struct {
  char What[__ERROR_H_BUFSIZE];
  size_t Hash;
} Error;

static inline Error ErrorNo(void) { return (Error){.What = "", .Hash = 0}; }

static inline Error ErrorNew(const char What[], ...) {
  assert(What != NULL);
  assert(strnlen(What, __ERROR_H_BUFSIZE) < __ERROR_H_BUFSIZE);

  va_list Args;
  va_start(Args, What);

  Error Err;

  vsnprintf(Err.What, __ERROR_H_BUFSIZE - 1, What, Args);
  Err.What[__ERROR_H_BUFSIZE - 1] = '\0';

  Err.Hash = djb2Hash(What);

  va_end(Args);
  return Err;
}

static inline bool ErrorIs(Error *Err) {
  assert(Err != NULL);
  return !(Err->Hash == 0);
}

static inline bool ErrorEquals(Error *ErrFirst, Error *ErrSecond) {
  assert(ErrFirst != NULL);
  assert(ErrSecond != NULL);

  return ErrFirst->Hash == ErrSecond->Hash;
}

static inline char *ErrorWhat(Error *Err) {
  assert(Err != NULL);
  return Err->What;
}

#endif /* _ERROR_H_ */
