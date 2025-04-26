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
#include <signal.h>
#include <syslog.h>
#include "SeverityLog_api.h"

/************************************/

/***********************************/
/******** Define statements ********/
/***********************************/

#define SVRTY_CRLF      "\r\n"
#define SVRTY_STR_END   '\0'
#define SVRTY_EMPTY_STR "\0"

#define SVRTY_MSG_INIT      "SeverityLog has been properly initialized."
#define SVRTY_MSG_CLEANUP   "Freeing SeverityLog's resources."
#define SVRTY_MSG_SIGNAL    "Received <%s> signal."

#define SVRTY_LOG_STR_DEFAULT_SIZE  10000

#define SVRTY_CLR_BASE  30 // Black color.

#define SVRTY_CHG_CLR       "\033[0;%dm"
#define SVRTY_RST_CLR       "\033[0m"

#define SVRTY_LVL_STR_SIZE  5
#define SVRTY_STR_ERR       "[ERR] "
#define SVRTY_STR_INF       "[INF] "
#define SVRTY_STR_WNG       "[WNG] "
#define SVRTY_STR_DBG       "[DBG] "

#define SVRTY_TIME_DATE_SIZE    128
#define SVRTY_TIME_DATE_FORMAT  "[%c] "

#define SVRTY_LOG_SUCCESS           0
#define SVRTY_LOG_UNINITIALIZED     -1
#define SVRTY_LOG_WNG_SILENT_LVL    -2
#define SVRTY_LOG_ALLOCATION_ERR    -3

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

static bool     is_initialized          = false;
static char*    log_str_buffer          = NULL;
static int      log_str_payload_size    = SVRTY_LOG_STR_DEFAULT_SIZE + 1;
static int      severity_log_mask       = SVRTY_LOG_MASK_EIW;
static bool     print_time_status       = false;
static bool     print_exe_file_name     = false;
static bool     log_to_syslog           = false;

/***********************************/

/*************************************/
/**** Private function prototypes ****/
/*************************************/

__attribute__((constructor)) static void SeverityLogLoad(void);
__attribute__((destructor)) static void SeverityLogUnload(void);

static void SeverityLogHandleSignal(int signal_number);
static void SeverityLogSetupSignalHandlers(void);

static void ChangeSeverityColor(int severity);
static void ResetSeverityColor(void);
static void PrintSeverityLevel(int severity);
static void PrintTime(void);
static void PrintCallingExeFileName(void);
static int CheckSeverityLogMask(int severity);
static void SeverityLogCleanup(void);

/*************************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

///////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Function to be called when current library is loaded. Sets up common signal handler.
/////////////////////////////////////////////////////////////////////////////////////////////// 
__attribute__((constructor)) static void SeverityLogLoad(void)
{
    SeverityLogSetupSignalHandlers();
}

////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Function to be called when current library is loaded. Performs resources cleanup.
////////////////////////////////////////////////////////////////////////////////////////////
__attribute__((destructor)) static void SeverityLogUnload(void)
{
    SeverityLogCleanup();
}

////////////////////////////////////////////////////////////
/// @brief Common signal handler. Executed cleanup function.
/// @param signal_number Target signal number.
////////////////////////////////////////////////////////////
static void SeverityLogHandleSignal(int signal_number)
{
    SVRTY_LOG_WNG(SVRTY_MSG_SIGNAL, strsignal(signal_number));
    SeverityLogCleanup();
}

/////////////////////////////////////////////////////
/// @brief Sets signal handler for different signals.
/////////////////////////////////////////////////////
static void SeverityLogSetupSignalHandlers(void)
{
    struct sigaction sa;
    sa.sa_handler = SeverityLogHandleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);  // Ctrl+C
    sigaction(SIGTERM, &sa, NULL); // Termination request
    sigaction(SIGHUP, &sa, NULL);  // Terminal closed
    sigaction(SIGQUIT, &sa, NULL); // Ctrl+'\'
    // sigaction(SIGPIPE, &sa, NULL); // Broken pipe
    // sigaction(SIGALRM, &sa, NULL); // Timer expiration
}

/////////////////////////////////////////////////////////////
/// @brief Changes log color depending on the severity level.
/// @param severity Severity level (ERR, INF, WNG, DBG)
/////////////////////////////////////////////////////////////
static void ChangeSeverityColor(int severity)
{
    snprintf(   (log_str_buffer + strlen(log_str_buffer))       ,
                (log_str_payload_size - strlen(log_str_buffer)) ,
                SVRTY_CHG_CLR                                   ,
                (SVRTY_CLR_BASE + severity)                     );
}

///////////////////////////////////////
/// @brief Resets log color to default.
///////////////////////////////////////
static void ResetSeverityColor(void)
{
    snprintf(   (log_str_buffer + strlen(log_str_buffer))       ,
                (log_str_payload_size - strlen(log_str_buffer)) ,
                SVRTY_RST_CLR                                   );
}

///////////////////////////////////////////////////////////////
/// @brief Prints a string at the beginning of the log message.
/// @param severity Severity level (ERR, INF, WNG, DBG)
///////////////////////////////////////////////////////////////
static void PrintSeverityLevel(int severity)
{
    char* severity_level_string = SVRTY_EMPTY_STR;

    switch(severity)
    {
        case SVRTY_LVL_ERR:
            severity_level_string = SVRTY_STR_ERR;
        break;

        case SVRTY_LVL_INF:
            severity_level_string = SVRTY_STR_INF;
        break;

        case SVRTY_LVL_WNG:
            severity_level_string = SVRTY_STR_WNG;
        break;

        case SVRTY_LVL_DBG:
            severity_level_string = SVRTY_STR_DBG;
        break;

        default:
        break;
    }

    if(strlen(severity_level_string) > 0)
        snprintf(   (log_str_buffer + strlen(log_str_buffer))       ,
                    (log_str_payload_size - strlen(log_str_buffer)) ,
                    "%s"                                            ,
                    severity_level_string                           );
}

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Sets severity log buffer payload size.
/// @param buffer_size Target payload size (a trailing zero is used to ensure safety).
/// @return 0 if allocation was successful, < 0 otherwise.
//////////////////////////////////////////////////////////////////////////////////////
int SetSeverityLogBufferSize(unsigned long buffer_size)
{
    if(buffer_size <= 0)
        buffer_size = SVRTY_LOG_STR_DEFAULT_SIZE;

    log_str_payload_size = buffer_size;

    if(log_str_buffer == NULL)
        log_str_buffer = (char*)calloc(buffer_size + 1, sizeof(char));
    else
        log_str_buffer = (char*)realloc(log_str_buffer, (buffer_size + 1) * sizeof(char));

    if(log_str_buffer == NULL)
            return SVRTY_LOG_ALLOCATION_ERR;

    return SVRTY_LOG_SUCCESS;
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
        return SVRTY_LOG_SUCCESS;

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
    
    snprintf(   (log_str_buffer + strlen(log_str_buffer))       ,
                (log_str_payload_size - strlen(log_str_buffer)) ,
                "%s"                                            ,
                time_str                                        );
}

/////////////////////////////////////////////////////////////
/// @brief Set value of print_exe_file_name private variable.
/// @param exe_name_status Target status value (T/F).
/////////////////////////////////////////////////////////////
void SetSeverityLogPrintExeNameStatus(bool exe_name_status)
{
    print_exe_file_name = exe_name_status;
}

void SetSeverityLogSyslogStatus(bool log_to_syslog_status)
{
    openlog(NULL, LOG_PID, LOG_USER);

    log_to_syslog = log_to_syslog_status;
}

///////////////////////////////////////////////////////////////////////////////////// 
/// @brief If print_exe_file_name == true, it print the calling executable file name.
/////////////////////////////////////////////////////////////////////////////////////
static void PrintCallingExeFileName(void)
{
    if(!print_exe_file_name)
        return;

    void *buffer[SVRTY_EXE_FILE_STACK_SIZE];
    int size = backtrace(buffer, SVRTY_EXE_FILE_STACK_SIZE);
    char **symbols = backtrace_symbols(buffer, size);
    
    if (size >= SVRTY_EXE_FILE_STACK_LVL)
    {
        char *symbol = symbols[SVRTY_EXE_FILE_STACK_LVL];
            if(symbol == NULL)
                return;
        
        // Erase memory address (starts with '(')
        char *start_of_function_name = symbol;
        char *end_of_function_name = strchr(start_of_function_name, SVRTY_EXE_FILE_ADDR_PREFIX);
        
        if (end_of_function_name)
            *end_of_function_name = SVRTY_STR_END;
        
        char* file_name = basename(start_of_function_name);

        char *so_extension = strstr(file_name, SVRTY_EXE_FILE_SO_SUFFIX);
        if (so_extension) {
            *so_extension = SVRTY_STR_END;

            if (strncmp(file_name, SVRTY_EXE_FILE_SO_PREFIX, SVRTY_EXE_FILE_SO_PREFIX_IDX) == 0)
                file_name += SVRTY_EXE_FILE_SO_PREFIX_IDX;
        }

        snprintf(   (log_str_buffer + strlen(log_str_buffer))       ,
                    (log_str_payload_size - strlen(log_str_buffer)) ,
                    SVRTY_EXE_FILE_FORMAT                           ,
                    file_name                                       );
    }

    if (symbols != NULL)
        free(symbols);
}

int SeverityLogGetSyslogMsgType(int severity)
{
    int syslog_msg_type = SVRTY_LOG_WNG_SILENT_LVL;

    switch(severity)
    {
        case SVRTY_LVL_ERR:
            syslog_msg_type = LOG_ERR;
        break;

        case SVRTY_LVL_INF:
            syslog_msg_type = LOG_INFO;
        break;

        case SVRTY_LVL_WNG:
            syslog_msg_type = LOG_WARNING;
        break;

        case SVRTY_LVL_DBG:
            syslog_msg_type = LOG_DEBUG;
        break;

        default:
        break;
    }

    return syslog_msg_type;
}

void SeverityLogSyslog(int severity, char* log_str_buffer_start)
{
    int syslog_msg_type = SeverityLogGetSyslogMsgType(severity);
    
    if(syslog_msg_type < 0)
        return;
    
    syslog(syslog_msg_type, "%s", log_str_buffer_start);
}

/////////////////////////////////////////////////////////////
/// @brief Sets multiple severity log parameters at once.
/// @param buffer_size Target buffer payload size.
/// @param severity_level_mask Target severity level(s) mask.
/// @param print_time Print log's time and date (T/F).
/// @param print_exe_file Print logging file's name (T/F).
/// @return 0 if succeeded, < 0 otherwise.
/////////////////////////////////////////////////////////////
int SeverityLogInit(unsigned long buffer_size, int severity_level_mask, bool print_time, bool print_exe_file)
{
    is_initialized = true;

    int set_buffer_size = SetSeverityLogBufferSize(buffer_size);
    
    if(set_buffer_size < 0)
        return set_buffer_size;
    
    SetSeverityLogMask(severity_level_mask);
    SetSeverityLogPrintTimeStatus(print_time);
    SetSeverityLogPrintExeNameStatus(print_exe_file);
    SetSeverityLogSyslogStatus(true);

    SVRTY_LOG_DBG(SVRTY_MSG_INIT);

    return SVRTY_LOG_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
/// @brief Performs resources cleanup for current library (frees log buffer).
/////////////////////////////////////////////////////////////////////////////
static void SeverityLogCleanup(void)
{
    if(log_to_syslog)
        closelog();

    if(!log_str_buffer)
        return;
    
    SVRTY_LOG_DBG(SVRTY_MSG_CLEANUP);
    free(log_str_buffer);
    log_str_buffer = NULL;
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
    if(!is_initialized)
        return SVRTY_LOG_UNINITIALIZED;

    if(log_str_buffer == NULL)
    {
        int init_log_buffer = SetSeverityLogBufferSize(log_str_payload_size);

        if(init_log_buffer < 0)
            return init_log_buffer;
    }

    int check_severity_log_mask = CheckSeverityLogMask(severity);

    if(check_severity_log_mask < 0)
        return check_severity_log_mask;

    va_list args;
    int done;

    ChangeSeverityColor(severity);
    PrintTime();

    int log_len_color_date = strlen(log_str_buffer);

    PrintSeverityLevel(severity);
    PrintCallingExeFileName();

    va_start(args, format);
    done = vsnprintf(   (log_str_buffer + strlen(log_str_buffer))       ,
                        (log_str_payload_size - strlen(log_str_buffer)) ,
                        format                                          ,
                        args                                            );
    va_end(args);

    log_str_buffer[strlen(log_str_buffer)] = SVRTY_STR_END;

    SeverityLogSyslog(severity, log_str_buffer + log_len_color_date);

    ResetSeverityColor();
    
    snprintf(   (log_str_buffer + strlen(log_str_buffer)),
                (log_str_payload_size - strlen(log_str_buffer)),
                "%s",
                SVRTY_CRLF);
    
    log_str_buffer[strlen(log_str_buffer)] = SVRTY_STR_END;

    printf("%s", log_str_buffer);
    fflush(stdout);

    memset(log_str_buffer, 0, strlen(log_str_buffer));

    return done;
}

/*************************************/
