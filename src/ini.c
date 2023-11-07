#include "ini.h"

#include <assert.h>
#include <string.h>

// return pointer to truncated string
static inline char *StrTruncate(char s[]) {
  if (s[0] == '\0') {
    return s;
  }
  char *Str = s;
  for (; *Str == ' '; ++Str)
    ;
  char *StrEnd = Str + strlen(Str) - 1;
  for (; *StrEnd == ' ' && StrEnd > Str; --StrEnd)
    ;
  StrEnd[1] = '\0';
  return Str;
}

int32_t IniLoad(FILE *Stream, IniCallBack *Func) {
  assert(Stream != NULL && "file stream is NULL");
  assert(Func != NULL && "callback func is NULL");

  char Section[__INI_H_BUFSIZE + 1];
  size_t SectionIdx = 0;
  char Key[__INI_H_BUFSIZE + 1];
  size_t KeyIdx = 0;
  char Value[__INI_H_BUFSIZE + 1];
  size_t ValueIdx = 0;

  INI_LEXEME Lexeme = INI_LEXEME_Key;
  INI_LEXEME PrevLexeme = Lexeme;
  size_t CallBackCount = 0;

  while (!feof(Stream)) {
    char ch = fgetc(Stream);
    switch (ch) {
    case '#':
    case ';':
      PrevLexeme = Lexeme;
      Lexeme = INI_LEXEME_Comment;
      continue;
    case '[':
      Lexeme = INI_LEXEME_Section;
      SectionIdx = 0;
      *Section = '\0';
      continue;
    case ']':
      Lexeme = INI_LEXEME_Key;
      Section[SectionIdx] = '\0';
      continue;
    case '=':
      if (Lexeme == INI_LEXEME_Value) {
        goto ValueHandle;
      }
      Lexeme = INI_LEXEME_Value;
      Key[KeyIdx] = '\0';
      KeyIdx = 0;
      continue;
    case '\n':
      Value[ValueIdx] = '\0';
      if (Lexeme == INI_LEXEME_Value ||
          (Lexeme == INI_LEXEME_Comment && PrevLexeme == INI_LEXEME_Value)) {
        size_t CallBackResult =
            Func(StrTruncate(Section), StrTruncate(Key), StrTruncate(Value));
        if (CallBackResult != 0) {
          return CallBackResult;
        }
        CallBackCount++;
      }
      Lexeme = INI_LEXEME_Key;
      KeyIdx = 0;
      ValueIdx = 0;
      continue;
    default:
    ValueHandle:
      switch (Lexeme) {
      case INI_LEXEME_Section:
        if (SectionIdx == __INI_H_BUFSIZE) {
          continue;
        }
        Section[SectionIdx] = ch;
        SectionIdx++;
        continue;
      case INI_LEXEME_Key:
        if (KeyIdx == __INI_H_BUFSIZE) {
          continue;
        }
        Key[KeyIdx] = ch;
        KeyIdx++;
        continue;
      case INI_LEXEME_Value:
        if (ValueIdx == __INI_H_BUFSIZE) {
          continue;
        }
        Value[ValueIdx] = ch;
        ValueIdx++;
        continue;
      default:
        continue;
      }
    }
  }

  // ini file is incorrect
  if (CallBackCount < 1) {
    return 1;
  }
  return 0;
}
