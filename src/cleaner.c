#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "cleaner.h"

static inline char GetCharForLevel(CLEANER_LEVEL Level) {
  assert(Level >= 1 && Level <= CLEANER_LEVEL_LEN);

  return Level + '0';
}

Error CleanerDropCaches(const char FileName[], CLEANER_LEVEL Level) {
  assert(FileName != NULL);
  assert(Level >= 1 && Level <= CLEANER_LEVEL_LEN);

  Error Err = ErrorNo();
  int32_t DropFile = 0;
  {
    errno = 0;
    DropFile = open(FileName, O_WRONLY);
    if (DropFile == -1) {
      Err = ErrorNew("Cant open %s file for writing: %s", FileName,
                     strerror(errno));
      goto Cleanup;
    }

    char Line[] = {GetCharForLevel(Level), '\n'};
    ssize_t Written = write(DropFile, Line, sizeof(Line));
    if (Written == -1) {
      Err = ErrorNew("Error on writting to %s file. Check for file access",
                     FileName);
      goto Cleanup;
    }
  }
Cleanup:
  if (DropFile != 0) {
    close(DropFile);
  }
  return Err;
}
