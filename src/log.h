#ifndef _LOG_H_
#define _LOG_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define __LOG_H_BUFSIZE 4096

typedef enum {
  LOG_VERBOSITY_None,
  LOG_VERBOSITY_Trace,
  LOG_VERBOSITY_Info,
  LOG_VERBOSITY_Warn,
  LOG_VERBOSITY_Error,
  LOG_VERBOSITY_Fatal,
} LOG_VERBOSITY;
#define LOG_VERBOSITY_LEN 6

extern LOG_VERBOSITY LogMaxVerbosity;
extern bool LogColored;
extern bool LogAddNewLine;
extern bool LogAddDate;

void LogFlog(LOG_VERBOSITY Verbosity, FILE *Stream, size_t Line,
             const char Filename[], const char Fmt[], ...);

#define LogTrace(...)                                                          \
  LogFlog(LOG_VERBOSITY_Trace, stderr, __LINE__, __FILE__, __VA_ARGS__)
#define LogInfo(...)                                                           \
  LogFlog(LOG_VERBOSITY_Info, stdout, __LINE__, __FILE__, __VA_ARGS__)
#define LogWarn(...)                                                           \
  LogFlog(LOG_VERBOSITY_Warn, stderr, __LINE__, __FILE__, __VA_ARGS__)
#define LogErr(...)                                                            \
  LogFlog(LOG_VERBOSITY_Error, stderr, __LINE__, __FILE__, __VA_ARGS__)
#define LogFatal(...)                                                          \
  do {                                                                         \
    LogFlog(LOG_VERBOSITY_Fatal, stderr, __LINE__, __FILE__, __VA_ARGS__);     \
    exit(1);                                                                   \
  } while (false);

#define FlogTrace(File, ...)                                                   \
  LogFlog(LOG_VERBOSITY_Trace, File, __LINE__, __FILE__, __VA_ARGS__)
#define FlogInfo(File, ...)                                                    \
  LogFlog(LOG_VERBOSITY_Info, File, __LINE__, __FILE__, __VA_ARGS__)
#define FlogWarn(File, ...)                                                    \
  LogFlog(LOG_VERBOSITY_Warn, File, __LINE__, __FILE__, __VA_ARGS__)
#define FlogErr(File, ...)                                                     \
  LogFlog(LOG_VERBOSITY_Error, File, __LINE__, __FILE__, __VA_ARGS__)
#define FlogFatal(File, ...)                                                   \
  do {                                                                         \
    LogFlog(LOG_VERBOSITY_Fatal, File, __LINE__, __FILE__, __VA_ARGS__);       \
    fflush(File);                                                              \
    exit(1);                                                                   \
  } while (false)

#endif /* _LOG_H_ */
