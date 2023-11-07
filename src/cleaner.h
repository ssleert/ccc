#ifndef __CLEANER_H__
#define __CLEANER_H__

#include "error.h"

typedef enum {
  CLEANER_LEVEL_Low = 1,
  CLEANER_LEVEL_Middle,
  CLEANER_LEVEL_High,
} CLEANER_LEVEL;
#define CLEANER_LEVEL_LEN 3

Error CleanerDropCaches(const char FileName[], CLEANER_LEVEL level);

#endif
