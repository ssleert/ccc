#ifndef __RAM_H__
#define __RAM_H__

#include <stddef.h>

#include "error.h"

#define MemInfoFile "/proc/meminfo"

typedef struct {
  size_t Val;
  Error Err;
} MemSizeError;

MemSizeError MemGetTotal(void);
MemSizeError MemGetFree(void);

#endif
