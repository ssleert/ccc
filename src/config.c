#include <ctype.h>
#include <errno.h>

#include "config.h"
#include "ini.h"
#include "log.h"

char ConfigLockFile[BUFSIZE] = "/tmp/ccc.lock";
bool ConfigLogSilent = false;
bool ConfigOptionsSync = true;
int32_t ConfigLevelsFirst = 15;
int32_t ConfigLevelsSecond = 10;
int32_t ConfigLevelsThird = 5;
size_t ConfigTimeoutsCheck = 10;
int32_t ConfigErrorMaxAmount = 10;

static int32_t IniCallBackFunc(const char Section[], const char Key[],
                               const char Val[]) {
  assert(Section != NULL);
  assert(Key != NULL);
  assert(Val != NULL);
  if (IniCheckValue(Section, Key, "Files", "Lock")) {
    if (Val[0] == '\0') {
      LogErr("Error with LogFile. string is empty");
      return 0;
    }
    strncpy(ConfigLockFile, Val, sizeof(ConfigLockFile) - 1);
    ConfigLockFile[sizeof(ConfigLockFile) - 1] = '\0';

    LogInfo("ConfigLockFile = '%s'", ConfigLockFile);
    return 0;
  }

  if (IniCheckValue(Section, Key, "Log", "Silent")) {
    ConfigLogSilent = (strncmp(Val, "true", BUFSIZE - 1) == 0);

    LogInfo("ConfigLogSilent = %s", ConfigLogSilent ? "true" : "false");
    return 0;
  }

  if (IniCheckValue(Section, Key, "Options", "Sync")) {
    ConfigOptionsSync = (strncmp(Val, "true", BUFSIZE - 1) == 0);

    LogInfo("ConfigOptionsSync = %s", ConfigOptionsSync ? "true" : "false");
    return 0;
  }

  if (IniCheckValue(Section, Key, "Levels", "First")) {
    int8_t Level = atoi(Val);
    if (Level <= 0 || Level >= 100) {
      LogErr("Error with LevelsFirst. incorrect value: %d", Level);
      return 0;
    }

    ConfigLevelsFirst = Level;
    LogInfo("ConfigLevelsFirst = %d", ConfigLevelsFirst);
    return 0;
  }

  if (IniCheckValue(Section, Key, "Levels", "Second")) {
    int8_t Level = atoi(Val);
    if (Level <= 0 || Level >= 100) {
      LogErr("Error with LevelsSecond. incorrect value: %d", Level);
      return 0;
    }

    ConfigLevelsSecond = Level;
    LogInfo("ConfigLevelsSecond = %d", ConfigLevelsSecond);
    return 0;
  }

  if (IniCheckValue(Section, Key, "Levels", "Third")) {
    int8_t Level = atoi(Val);
    if (Level <= 0 || Level >= 100) {
      LogErr("Error with LevelsThird. incorrect value: %d", Level);
      return 0;
    }

    ConfigLevelsThird = Level;
    LogInfo("ConfigLevelsThird = %d", ConfigLevelsThird);
    return 0;
  }

  if (IniCheckValue(Section, Key, "Timeouts", "Check")) {
    int32_t Check = atoi(Val);
    if (Check <= 0) {
      LogErr("Error with TimeoutsCheck. incorrect value: %d sec", Check);
      return 0;
    }

    ConfigTimeoutsCheck = Check;
    LogInfo("ConfigTimeoutsCheck = %d", ConfigTimeoutsCheck);
    return 0;
  }

  if (IniCheckValue(Section, Key, "Error", "MaxAmount")) {
    int32_t Amount = atoi(Val);
    if (Amount <= 0) {
      LogErr("Error with ErrorMaxAmount. incorrect value: %d", Amount);
      return 0;
    }

    ConfigErrorMaxAmount = Amount;
    LogInfo("ConfigErrorMaxAmount = %d", ConfigErrorMaxAmount);
    return 0;
  }

  return 0;
}

Error ConfigLoad(const char ConfigFileStr[]) {
  assert(ConfigFileStr != NULL);

  Error Err = ErrorNo();
  FILE *ConfigFile = NULL;
  {
    const char *FileStr =
        (ConfigFileStr[0] == '\0') ? ConfigFileDef : ConfigFileStr;

    errno = 0;
    ConfigFile = fopen(FileStr, "r");
    if (ConfigFile == NULL) {
      Err = ErrorNew("Config File %s open error: %s", FileStr, strerror(errno));
      goto Cleanup;
    }

    int32_t IniParsingResult = IniLoad(ConfigFile, IniCallBackFunc);
    if (IniParsingResult == 1) {
      Err = ErrorNew("Config file doesn't finded");
      goto Cleanup;
    }
    if (IniParsingResult != 0) {
      Err = ErrorNew("Ini parsing error: %d code", IniParsingResult);
      goto Cleanup;
    }
  }
Cleanup:
  if (ConfigFile != NULL) {
    fclose(ConfigFile);
  }
  return Err;
}
