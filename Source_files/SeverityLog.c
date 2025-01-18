/************************************/
/******** Include statements ********/
/************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>
#include <execinfo.h>
#include <stdlib.h>
#include <libgen.h>
#include "SeverityLog_api.h"

/************************************/

/***********************************/
/******** Define statements ********/
/***********************************/

#define SVRTY_CRLF      "\r\n"
#define SVRTY_STR_END   '\0'

#define SVRTY_CLR_BASE  30 // Black color.

#define SVRTY_CHG_CLR       "\033[0;%dm"
#define SVRTY_RST_CLR       "\033[0m"

#define SVRTY_STR_ERR       "[ERR] "
#define SVRTY_STR_INF       "[INF] "
#define SVRTY_STR_WNG       "[WNG] "
#define SVRTY_STR_DBG       "[DBG] "

#define SVRTY_TIME_DATE_SIZE    128
#define SVRTY_TIME_DATE_FORMAT  "[%c] "

#define SVRTY_LOG_SUCCESS           0
#define SVRTY_LOG_WNG_SILENT_LVL    -1

#define SVRTY_EXE_FILE_STACK_SIZE       3
#define SVRTY_EXE_FILE_STACK_LVL        2
#define SVRTY_EXE_FILE_ADDR_PREFIX      '('
#define SVRTY_EXE_FILE_SO_SUFFIX        ".so"
#define SVRTY_EXE_FILE_SO_PREFIX        "lib"
#define SVRTY_EXE_FILE_SO_PREFIX_IDX    3
#define SVRTY_EXE_FILE_FORMAT           "[%s] "

/***********************************/

/***********************************/
/******** Private variables ********/
/***********************************/

static int  severity_log_mask   = SVRTY_LOG_MASK_EIW;
static bool print_time_status   = false;
static bool print_exe_file_name = false;

/***********************************/

/*************************************/
/**** Private function prototypes ****/
/*************************************/

static void ChangeSeverityColor(int severity);
static void ResetSeverityColor(void);
static void PrintSeverityLevel(int severity);
static void PrintTime(void);
static void PrintCallingExeFileName(void);
static int CheckSeverityLogMask(int severity);

/*************************************/

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
static void PrintTime(void)
{
    if(!print_time_status)
        return;

    time_t current_time;
    struct tm* time_info;

    time(&current_time);  // Get the current time
    time_info = localtime(&current_time);  // Convert to local time

    if (time_info == NULL)
        return;

    char time_str[SVRTY_TIME_DATE_SIZE];
    strftime(time_str, sizeof(time_str), SVRTY_TIME_DATE_FORMAT, time_info);
    printf("%s", time_str);
}

/////////////////////////////////////////////////////////////
/// @brief Set value of print_exe_file_name private variable.
/// @param exe_name_status Target status value (T/F).
/////////////////////////////////////////////////////////////
void SetSeverityLogPrintExeNameStatus(bool exe_name_status)
{
    print_exe_file_name = exe_name_status;
}

///////////////////////////////////////////////////////////////////////////////////// 
/// @brief If print_exe_file_name == true, it print the calling executable file name.
/////////////////////////////////////////////////////////////////////////////////////
static void PrintCallingExeFileName(void)
{
    if(!print_exe_file_name)
        return;

    void *buffer[SVRTY_EXE_FILE_STACK_SIZE];
    int size = backtrace(buffer, SVRTY_EXE_FILE_STACK_SIZE);  // Capture the call stack
    char **symbols = backtrace_symbols(buffer, size);  // Resolve the symbols
    
    if (size >= SVRTY_EXE_FILE_STACK_LVL)
    {
        char *symbol = symbols[SVRTY_EXE_FILE_STACK_LVL];  // The calling function (1st index in backtrace)
        
        // Look for the first '(' character, which indicates the start of the memory address
        char *start_of_function_name = symbol;
        char *end_of_function_name = strchr(start_of_function_name, SVRTY_EXE_FILE_ADDR_PREFIX);  // Find the '('
        
        if (end_of_function_name) {
            *end_of_function_name = SVRTY_STR_END;
        }
        
        char* file_name = basename(start_of_function_name);

        char *so_extension = strstr(file_name, SVRTY_EXE_FILE_SO_SUFFIX);
        if (so_extension) {
            *so_extension = SVRTY_STR_END;

            if (strncmp(file_name, SVRTY_EXE_FILE_SO_PREFIX, SVRTY_EXE_FILE_SO_PREFIX_IDX) == 0) {
                file_name += SVRTY_EXE_FILE_SO_PREFIX_IDX;
            }
        }

        // Print just the function name
        printf(SVRTY_EXE_FILE_FORMAT, file_name);  
    }

    if (symbols != NULL)
        free(symbols);
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
    PrintCallingExeFileName();

    va_start(args, format);
    done = vfprintf(stdout, format, args);
    va_end(args);

    ResetSeverityColor();

    printf(SVRTY_CRLF);

    return done;
}

/*************************************/
