#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "ccc.h"
#include "mem.h"

// get line from file without heap allocation
// returns Len if buffer fully used
// and -1 on EOF
static inline ssize_t GetLine(char Str[], size_t Len, FILE *Stream) {
  assert(Str != NULL);
  assert(Stream != NULL);

  ssize_t Idx = 0;
  while (true) {
    if (feof(Stream)) {
      Idx = -1;
      break;
    }
    if ((size_t)Idx == Len) {
      break;
    }
    char ch = fgetc(Stream);
    if (ch == '\n') {
      break;
    }
    Str[Idx] = ch;
    Idx++;
  }

  Str[Len - 1] = '\0';
  return Idx;
}

// we cant get error here
static inline size_t GetSizeFromMemStr(const char Str[]) {
  assert(Str != NULL);

  char Integer[BUFSIZE] = {0};
  size_t IntegerIdx = 0;

  for (size_t i = 0; Str[i] != '\0'; ++i) {
    if (isdigit(Str[i])) {
      Integer[IntegerIdx] = Str[i];
      IntegerIdx++;
    }
  }

  char *endptr = NULL;
  size_t Result = strtoumax(Integer, &endptr, 10);

  return Result;
}

MemSizeError MemGetTotal(void) {
  MemSizeError Result = {0};

  FILE *MemFile = NULL;
  {

    errno = 0;
    MemFile = fopen(MemInfoFile, "r");
    if (MemFile == NULL) {
      Result.Err =
          ErrorNew("Error with %s file open: %s", MemInfoFile, strerror(errno));
      goto Cleanup;
    }

    char Line[BUFSIZE] = {0};
    ssize_t Readed = GetLine(Line, sizeof(Line), MemFile);
    if (Readed == -1) {
      Result.Err =
          ErrorNew("Error with total line reading from %s file", MemInfoFile);
      goto Cleanup;
    }

    Result.Val = GetSizeFromMemStr(Line);
  }
Cleanup:
  if (MemFile != NULL) {
    fclose(MemFile);
  }
  return Result;
}

MemSizeError MemGetFree(void) {
  MemSizeError Result = {0};

  FILE *MemFile = NULL;
  {

    errno = 0;
    MemFile = fopen(MemInfoFile, "r");
    if (MemFile == NULL) {
      Result.Err =
          ErrorNew("Error with %s file open: %s", MemInfoFile, strerror(errno));
      goto Cleanup;
    }

    char Line[BUFSIZE] = {0};
    // skip total mem line
    (void)GetLine(Line, sizeof(Line), MemFile);

    ssize_t Readed = GetLine(Line, sizeof(Line), MemFile);
    if (Readed == -1) {
      Result.Err =
          ErrorNew("Error with free line reading from %s file", MemInfoFile);
      goto Cleanup;
    }

    Result.Val = GetSizeFromMemStr(Line);
  }
Cleanup:
  if (MemFile != NULL) {
    fclose(MemFile);
  }
  return Result;
}
