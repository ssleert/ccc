#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ccc.h"
#include "cleaner.h"
#include "config.h"
#include "error.h"
#include "log.h"
#include "mem.h"

// private
FILE *CccLockFile = NULL;

// -c <config file>
// -a [switch output colors]
// -s [silent mode]
static const char *Options = "c:as";
static const int32_t ExitSignals[] = {
    SIGTERM, SIGINT, SIGQUIT, SIGKILL, SIGHUP,
};

static inline void PrintHelp(void) {
  puts(" ccc - simple cache cleaner");
  puts("  -c <config file>");
  puts("  -a [switch output colors]");
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

static Error Init(const char ConfigFileStr[], bool SilentLog) {
  assert(ConfigFileStr != NULL);

  if (geteuid() != 0) {
    return ErrorNew("App needs root");
  }

  if (stat(ConfigFileStr, &(struct stat){0}) != 0) {
    LogWarn("Config file '%s' doesnt finded.", ConfigFileStr);
  } else {
    Error Err = ConfigLoad(ConfigFileStr);
    if (ErrorIs(&Err)) {
      return ErrorNew("Config error: %s", ErrorWhat(&Err));
    }
    LogTrace("Config File Parsed.");
  }

  if (SilentLog == false) {
    LogMaxVerbosity =
        ConfigLogSilent ? LOG_VERBOSITY_Warn : LOG_VERBOSITY_Trace;
  } else {
    LogMaxVerbosity = LOG_VERBOSITY_Warn;
  }

  if (stat(ConfigLockFile, &(struct stat){0}) == 0) {
    return ErrorNew("Lock file accured. ccc already working");
  }
  LogTrace("Lock file checked.");

  errno = 0;
  FILE *check = fopen(DropCachesFile, "w");
  if (check == NULL) {
    return ErrorNew("Cant open %s file for writing: %s", DropCachesFile,
                    strerror(errno));
  }
  fclose(check);
  LogTrace("DropCachesFile checked.");

  errno = 0;
  CccLockFile = fopen(ConfigLockFile, "w");
  if (CccLockFile == NULL) {
    return ErrorNew("Cant open %s lock file: %s", ConfigLockFile,
                    strerror(errno));
  }
  fputs("Why do u check lock file?", CccLockFile);
  fflush(CccLockFile);
  LogTrace("Lock file created.");

  return ErrorNo();
}

static void DeInit(int Signal) {
  if (CccLockFile != NULL) {
    fclose(CccLockFile);
    LogTrace("Lock file closed.");
    remove(ConfigLockFile);
    LogTrace("Lock file removed.");
  }
  fflush(stdout);
  LogFatal("DeInit called. Received signal: %d.", Signal);
}

static void RegDeInit(void) {
  for (size_t i = 0; i < sizeof(ExitSignals) / sizeof(ExitSignals[0]); ++i) {
    signal(ExitSignals[i], &DeInit);
  }
}

int main(int argc, char *argv[]) {
  // thx jcs for https://no-color.org/
  char *NoColor = getenv("NO_COLOR");
  LogColored = (NoColor != NULL && NoColor[0] != '\0') ? false : true;

  char ConfigFileStr[BUFSIZE] = "/usr/local/etc/ccc.conf";
  bool SilentLog = false;

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
    case 'a': {
      LogColored = !LogColored;
      break;
    }
    case 's': {
      LogMaxVerbosity = LOG_VERBOSITY_Warn;
      SilentLog = true;
      break;
    }
    default: {
      PrintHelp();
      return 1;
    }
    }
  }

  LogInfo("ccc started.");
  LogTrace("Config file - '%s'.", ConfigFileStr);

  RegDeInit();
  LogTrace("DeInit registered.");

  Error Err = Init(ConfigFileStr, SilentLog);
  if (ErrorIs(&Err)) {
    LogFatal("Error on Init: %s.", ErrorWhat(&Err));
  }
  LogTrace("Initialization completed.");

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

  // count errors before stop
  int32_t ErrorCounter = 0;

  // main loop
  LogInfo("ccc started normaly.");
  while (true) {
    FreeMem = MemGetFree();
    if (ErrorIs(&FreeMem.Err)) {
      LogErr("Error with MemGetTotal(): %s.", ErrorWhat(&Err));
      ErrorCounter++;
    }

    int32_t Percent = ((double)FreeMem.Val / TotalMem.Val) * 100;
    int32_t CleanLevel = GetCleanLevelByPercent(Percent);
    LogInfo("Free memory amount: %.2fmib or %d%% and CleanLevel = %d",
            (double)FreeMem.Val / 1024, Percent, CleanLevel);

    if (CleanLevel > 0) {
      if (ConfigOptionsSync) {
        sync();
        LogInfo("sync() called.");
      }
      Err = CleanerDropCaches(DropCachesFile, CleanLevel);
      if (ErrorIs(&Err)) {
        LogErr("Error with CleanerDropCaches(): %s.", ErrorWhat(&Err));
        ErrorCounter++;
      }
      LogInfo("Caches dropped.");
    }

    int32_t Slept = sleep(ConfigTimeoutsCheck);
    if (Slept > 0) {
      LogErr("Not all time slept. time left: %ds", Slept);
      ErrorCounter++;
    }

    if (ErrorCounter >= ConfigErrorMaxAmount) {
      LogFatal("ErrorCounter is to big. exiting");
    }
  }

  return 0;
}
