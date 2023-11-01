#ifndef SEVERITY_LOG_H
#define SEVERITY_LOG_H

/************************************/
/******** Include statements ********/
/************************************/

#include "SeverityLog_api.h"

/************************************/

/***********************************/
/******** Define statements ********/
/***********************************/

#define SVRTY_CLR_BASE  30 // Black color.

#define SVRTY_CHG_CLR       "\033[0;%dm"
#define SVRTY_RST_CLR       "\033[0m"

#define SVRTY_STR_ERR       "[ERR] "
#define SVRTY_STR_INF       "[INF] "
#define SVRTY_STR_WNG       "[WNG] "
#define SVRTY_STR_DBG       "[DBG] "

#define SVRTY_STR_TIME_DATE "[%c] "

#define SVRTY_LOG_SUCCESS           0
#define SVRTY_LOG_WNG_SILENT_LVL    -1

/***********************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

static void ChangeSeverityColor(int severity);
static void ResetSeverityColor(void);
static void PrintSeverityLevel(int severity);
static void PrintTime(void);
static int CheckSeverityLogMask(int severity);

/*************************************/

#endif