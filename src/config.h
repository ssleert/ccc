#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdbool.h>
#include <stdint.h>

#include "ccc.h"
#include "error.h"
#include "log.h"

#define ConfigFileDef "/usr/local/etc/ccc.conf"

extern char ConfigLockFile[BUFSIZE];
extern bool ConfigLogSilent;
extern bool ConfigOptionsSync;
extern int32_t ConfigLevelsFirst;
extern int32_t ConfigLevelsSecond;
extern int32_t ConfigLevelsThird;
extern size_t ConfigTimeoutsCheck;
extern int32_t ConfigErrorMaxAmount;

Error ConfigLoad(const char ConfigFile[]);

#endif
