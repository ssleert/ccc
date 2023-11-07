#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "cleaner.h"
#include "log.h"

static inline char GetCharForLevel(CLEANER_LEVEL Level) {
  assert(Level < 1 && Level > CLEANER_LEVEL_LEN);

  return Level + '0';
}

Error CleanerDropCaches(const char FileName[], CLEANER_LEVEL Level) {
  assert(Level < 1 && Level > CLEANER_LEVEL_LEN);

  Error Err = ErrorNo();
  int DropFile = 0;

  {
    errno = 0;
    DropFile = open(FileName, O_WRONLY);
    if (DropFile == 0) {
      Err = ErrorNew("Cant open %s file for writing: %s", FileName,
                     strerror(errno));
      goto Cleanup;
    }

    char Line[2] = {GetCharForLevel(Level), '\n'};
    size_t Written = write(DropFile, Line, sizeof(Line));
    if (Written == 0) {
      Err = ErrorNew("Zero bytes written to %s file. Check for file access",
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
