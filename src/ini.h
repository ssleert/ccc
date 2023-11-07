// Super simple stack-only ini parser
// for embedded systems in C
// 2023 Simon Ryabinkov <smnrbkv@proton.me>

#ifndef __INI_H__
#define __INI_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define __INI_H_BUFSIZE 4096

// func that was called on all values in .ini file
typedef int32_t(IniCallBack)(const char Section[], const char Key[],
                             const char Val[]);

typedef enum {
  INI_LEXEME_Section = 1,
  INI_LEXEME_Key,
  INI_LEXEME_Value,
  INI_LEXEME_Comment
} INI_LEXEME;
#define INI_LEXEME_LEN 4

static inline bool IniCheckValue(const char Section[], const char Key[],
                                 const char NeededSection[],
                                 const char NeededKey[]) {
  return (strcmp(Section, NeededSection) == 0 && strcmp(Key, NeededKey) == 0);
}

// Stream - opened file stream
// Func - return non-zero to stop, error is passed back through function
// return - 0 on success
int32_t IniLoad(FILE *Stream, IniCallBack *Func);

#endif
