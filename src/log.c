#include "log.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

LOG_VERBOSITY LogMaxVerbosity = LOG_VERBOSITY_Trace;
bool LogColored = true;
bool LogAddNewLine = true;
bool LogAddDate = false;

static const char *LogVerbosityStrings[] = {
    "why you're watching inside binary?",
    "[TRACE]",
    "[INFO] ",
    "[WARN] ",
    "[ERROR]",
    "[FATAL]",
};

static const char *LogVerbosityStringsColored[] = {
    "sfome on the swag",          "[\033[0;34mTRACE\033[1;0m]",
    "[\033[0;32mINFO\033[1;0m] ", "[\033[0;33mWARN\033[1;0m] ",
    "[\033[0;31mERROR\033[1;0m]", "[\033[0;31mFATAL\033[1;0m]",
};

static inline struct tm *LogCurrentTime(void) {
  time_t t = time(NULL);
  return localtime(&t);
}

void LogFlog(LOG_VERBOSITY Verbosity, FILE *Stream, size_t Line,
             const char Filename[], const char Fmt[], ...) {
  assert(Stream != NULL);
  assert(Filename != NULL);
  assert(Fmt != NULL);
  assert(strnlen(Fmt, __LOG_H_BUFSIZE) < __LOG_H_BUFSIZE);

  if (LogMaxVerbosity == LOG_VERBOSITY_None ||
      Verbosity == LOG_VERBOSITY_None) {
    return;
  }
  if (Verbosity < LogMaxVerbosity) {
    return;
  }

  char TimeBuffer[64];
  char StringBuffer[__LOG_H_BUFSIZE + 256];
  char *StringPointer = StringBuffer;
  bool LocalLogColored =
      (Stream == stdout || Stream == stderr) ? LogColored : false;

  strftime(TimeBuffer, sizeof(TimeBuffer),
           (LogAddDate) ? "%Y-%m-%d %H:%M:%S" : "%H:%M:%S", LogCurrentTime());

  StringPointer +=
      sprintf(StringPointer, "%s %s %s:%zu: ", TimeBuffer,
              (LocalLogColored) ? LogVerbosityStringsColored[Verbosity]
                                : LogVerbosityStrings[Verbosity],
              Filename, Line);

  va_list Args;
  va_start(Args, Fmt);
  size_t w = vsnprintf(StringPointer, __LOG_H_BUFSIZE - 1, Fmt, Args);
  va_end(Args);

  StringPointer += (w >= __LOG_H_BUFSIZE) ? __LOG_H_BUFSIZE - 1 : w;

  if (LogAddNewLine) {
    StringPointer[0] = '\n';
    StringPointer[1] = '\0';
  }

  fputs(StringBuffer, Stream);
  return;
}
