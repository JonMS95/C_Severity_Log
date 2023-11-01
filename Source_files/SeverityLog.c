/************************************/
/******** Include statements ********/
/************************************/

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>
#include "SeverityLog.h"

/************************************/

/***********************************/
/******** Private variables ********/
/***********************************/

static int  severity_log_mask   = SVRTY_LOG_MASK_EIW;
static bool print_time_status   = false;

/***********************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

/////////////////////////////////////////////////////////////
/// @brief Changes log color depending on the severity level.
/// @param severity Severity level (ERR, INF, WNG, DBG)
/////////////////////////////////////////////////////////////
static void ChangeSeverityColor(int severity)
{
    int color = SVRTY_CLR_BASE + severity;

    printf(SVRTY_CHG_CLR, color);
}

///////////////////////////////////////
/// @brief Resets log color to default.
///////////////////////////////////////
static void ResetSeverityColor()
{
    printf(SVRTY_RST_CLR);
}

///////////////////////////////////////////////////////////////
/// @brief Prints a string at the beginning of the log message.
/// @param severity Severity level (ERR, INF, WNG, DBG)
///////////////////////////////////////////////////////////////
static void PrintSeverityLevel(int severity)
{
    switch(severity)
    {
        case SVRTY_LVL_ERR:
        {
            printf(SVRTY_STR_ERR);
        }
        break;

        case SVRTY_LVL_INF:
        {
            printf(SVRTY_STR_INF);
        }
        break;

        case SVRTY_LVL_WNG:
        {
            printf(SVRTY_STR_WNG);
        }
        break;

        case SVRTY_LVL_DBG:
        {
            printf(SVRTY_STR_DBG);
        }
        break;

        default:
        break;
    }
}

/////////////////////////////////////////////////////
/// @brief Sets severity log mask to the input value.
/// @param mask Target severity log mask.
/////////////////////////////////////////////////////
void SetSeverityLogMask(int mask)
{
    severity_log_mask = mask;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Checks the current severity log mask.  
/// @param severity Target message severity level.
/// @return SVRTY_LOG_SUCCESS if the severity level is allowed, SVRTY_LOG_WNG_SILENT_LVL otherwise.
///////////////////////////////////////////////////////////////////////////////////////////////////
static int CheckSeverityLogMask(int severity)
{
    int bit_to_check = (1 << (severity - 1));
    if( (severity_log_mask & bit_to_check) != 0)
    {
        return SVRTY_LOG_SUCCESS;
    }

    return SVRTY_LOG_WNG_SILENT_LVL;
}

///////////////////////////////////////////////////////////
/// @brief Set value of print_time_status private variable.
/// @param time_status Target status value (T/F).
///////////////////////////////////////////////////////////
void SetSeverityLogPrintTimeStatus(bool time_status)
{
    print_time_status = time_status;
}

////////////////////////////////////////////////////////////////////////////////////////// 
/// @brief If print_time_status == true, it prints time (includes date) in local timezone.
//////////////////////////////////////////////////////////////////////////////////////////
void PrintTime(void)
{
    if(!print_time_status)
        return;

    time_t current_time;
    struct tm* time_info;

    time(&current_time);  // Get the current time
    time_info = localtime(&current_time);  // Convert to local time

    if (time_info == NULL)
        return;

    char time_str[128];
    strftime(time_str, sizeof(time_str), SVRTY_STR_TIME_DATE, time_info);
    printf("%s", time_str);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Prints a log with different color and initial string depending on the severity level.
/// @param severity Severity level (ERR, INF, WNG).
/// @param format Formatted string. Same as what can be used with printf.
/// @param  Variable Variable number of arguments. Data that is meant to be formatted and printed.
/// @return < 0 if any error happened, number of characters written to stream otherwise.
//////////////////////////////////////////////////////////////////////////////////////////////////
int SeverityLog(int severity, const char* format, ...)
{
    int check_severity_log_mask = CheckSeverityLogMask(severity);

    if(check_severity_log_mask < 0)
    {
        return check_severity_log_mask;
    }

    va_list args;
    int done;

    ChangeSeverityColor(severity);

    PrintTime();

    PrintSeverityLevel(severity);

    va_start(args, format);
    done = vfprintf(stdout, format, args);
    va_end(args);

    ResetSeverityColor();

    printf("\r\n");

    return done;
}

/*************************************/
