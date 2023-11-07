#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ccc.h"
#include "cleaner.h"
#include "config.h"
#include "error.h"
#include "log.h"
#include "mem.h"

FILE *CccLogFile = NULL;

// -c <config file>
// -l <log file>
// -s [silent mode]
// -n [disable log colors]
// -a [disable fs sync before cache drop]
static const char *Options = "sac:l:n";

static inline void PrintHelp(void) {
  puts(" ccc - simple cache cleaner");
  puts("  -c <config file>                      ");
  puts("  -l <log file>                         ");
  puts("  -s [silent mode]                      ");
  puts("  -n [disable log colors]               ");
  puts("  -a [disable fs sync before cache drop]");
}

static inline int32_t GetCleanLevelByPercent(int32_t Percent) {
  assert(Percent >= 0 && Percent <= 100);

  int32_t Result = 0;
  if (Percent <= ConfigLevelsFirst)
    Result++;
  if (Percent <= ConfigLevelsSecond)
    Result++;
  if (Percent <= ConfigLevelsThird)
    Result++;
  return Result;
}

static Error Init(const char ConfigFileStr[], const char LogFileStr[]) {
  assert(ConfigFileStr != NULL);
  assert(LogFileStr != NULL);

  //if (geteuid() != 0) {
  //  return ErrorNew("App needs root");
  //}

  Error Err = ConfigLoad(ConfigFileStr);
  if (ErrorIs(&Err)) {
    return ErrorNew("Config error: %s", ErrorWhat(&Err));
  }
  LogTrace("Config File Parsed.");

  if (LogFileStr[0] != '\0') {
    strncpy(ConfigLogFile, LogFileStr, sizeof(ConfigLogFile) - 1);
    ConfigLogFile[sizeof(ConfigLogFile) - 1] = '\0';
  }

  errno = 0;
  CccLogFile = (strncmp(ConfigLogFile, "stdout", BUFSIZE) == 0)
                   ? stdout
                   : fopen(ConfigLogFile, "w");
  if (CccLogFile == NULL) {
    return ErrorNew("Log File open error: %s", strerror(errno));
  }
  LogTrace("Log File opened.");

  //errno = 0;
  //FILE *check = fopen(DropCachesFile, "w");
  //if (check == NULL) {
  //  return ErrorNew("Cant open %s file for writing: %s", DropCachesFile,
  //                  strerror(errno));
  //}
  //fclose(check);
  LogTrace("DropCachesFile checked.");

  return ErrorNo();
}

static void DeInit(void) {
  if (CccLogFile != NULL && CccLogFile != stdout) {
    fclose(CccLogFile);
    LogTrace("Log File closed.");
  }
}

int main(int argc, char *argv[]) {
  LogInfo("ccc started.");

  char ConfigFileStr[BUFSIZE] = {0};
  char LogFileStr[BUFSIZE] = {0};
  bool NeedSync = true;

  // flag parsing
  while (true) {
    int32_t r = getopt(argc, argv, Options);
    if (r == -1)
      break;
    switch (r) {
    case 'c': {
      size_t OptArgLen = strnlen(optarg, BUFSIZE);
      if (OptArgLen > sizeof(ConfigFileStr) - 1) {
        LogFatal("Stack Buffer overwrited "
                 "on ConfigFile. %d > %d",
                 OptArgLen, sizeof(optarg));
      }
      strncpy(ConfigFileStr, optarg, sizeof(ConfigFileStr) - 1);
      ConfigFileStr[sizeof(ConfigFileStr) - 1] = '\0';
      break;
    }
    case 'l': {
      size_t OptArgLen = strnlen(optarg, BUFSIZE);
      if (OptArgLen > sizeof(LogFileStr) - 1) {
        LogFatal("Stack Buffer overwrited "
                 "on LogFile. %d > %d",
                 OptArgLen, sizeof(optarg));
      }
      strncpy(LogFileStr, optarg, sizeof(LogFileStr) - 1);
      LogFileStr[sizeof(LogFileStr) - 1] = '\0';
      break;
    }
    case 's': {
      LogMaxVerbosity = LOG_VERBOSITY_Warn;
      break;
    }
    case 'n': {
      LogColored = false;
      break;
    }
    case 'a': {
      NeedSync = false;
      break;
    }
    default: {
      PrintHelp();
      return 1;
    }
    }
  }
  LogTrace("Config file - '%s'.", ConfigFileStr);
  LogTrace("Log file - '%s'.", LogFileStr);

  Error Err = Init(ConfigFileStr, LogFileStr);
  if (ErrorIs(&Err)) {
    LogFatal("Error on Init: %s.", ErrorWhat(&Err));
  }
  atexit(DeInit);

  MemSizeError TotalMem = MemGetTotal();
  if (ErrorIs(&TotalMem.Err)) {
    LogFatal("Error with MemGetTotal(): %s.", ErrorWhat(&Err));
  }
  MemSizeError FreeMem = MemGetFree();
  if (ErrorIs(&FreeMem.Err)) {
    LogFatal("Error with MemGetTotal(): %s.", ErrorWhat(&Err));
  }

  LogInfo("Total mem: %.2fmib.", (double)TotalMem.Val / 1024);
  LogInfo("Free mem: %.2fmib.", (double)FreeMem.Val / 1024);
  LogWarn("Used logging file: '%s'.", ConfigLogFile);

  fflush(stdout);
  fflush(stderr);

  // count errors before stop
  int32_t ErrorCounter = 0;

  // main loop
  FlogInfo(CccLogFile, "ccc started normaly.");
  while (true) {
    FreeMem = MemGetFree();
    if (ErrorIs(&FreeMem.Err)) {
      FlogErr(CccLogFile, "Error with MemGetTotal(): %s.", ErrorWhat(&Err));
      ErrorCounter++;
    }

    int32_t Percent = ((double)FreeMem.Val / TotalMem.Val) * 100;
    int32_t CleanLevel = GetCleanLevelByPercent(Percent);
    FlogInfo(CccLogFile,
             "Free memory amount: %.2fmib or %d%% and CleanLevel = %d",
             (double)FreeMem.Val / 1024, Percent, CleanLevel);

    if (CleanLevel > 0) {
      fflush(CccLogFile);
      if (NeedSync) {
        sync();
        FlogInfo(CccLogFile, "sync() called.");
      }
      Err = CleanerDropCaches(DropCachesFile, CleanLevel);
      if (ErrorIs(&Err)) {
        FlogErr(CccLogFile, "Error with CleanerDropCaches(): %s.",
                ErrorWhat(&Err));
        ErrorCounter++;
      }
      FlogInfo(CccLogFile, "Caches dropped.");
    }

    int32_t Slept = sleep(ConfigTimeoutsCheck + 1);
    if (Slept > 0) {
      FlogErr(CccLogFile, "Not all time slept. time left: %ds", Slept);
      ErrorCounter++;
    }

    if (ErrorCounter >= ConfigErrorMaxAmount) {
      FlogFatal(CccLogFile, "ErrorCounter is to big. exiting");
    }
  }

  return 0;
}
