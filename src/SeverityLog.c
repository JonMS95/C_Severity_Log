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
#include <pthread.h>
#include <stdint.h>
#include "MutexGuard_api.h"
#include "SignalHandler_api.h"
#include "SeverityLog_api.h"

/************************************/

/***********************************/
/******** Define statements ********/
/***********************************/

#define SVRTY_CRLF      "\r\n"
#define SVRTY_CR        '\r'
#define SVRTY_LF        '\n'
#define SVRTY_STR_END   '\0'
#define SVRTY_EMPTY_STR "\0"

#define SVRTY_MSG_INIT      "SeverityLog has been properly initialized."
#define SVRTY_MSG_CLEANUP   "Freeing SeverityLog's resources."
#define SVRTY_MSG_SIGNAL    "Received <%s> signal."

#define SVRTY_LOG_STR_DEFAULT_SIZE  10000

#define SVRTY_CLR_BASE  30 // Black color.

#define SVRTY_CHG_CLR       "\033[0;%dm"
#define SVRTY_RST_CLR       "\033[0m"

#define SVRTY_CLR_STR_SIZE          17
#define SVRTY_LVL_STR_SIZE          7
#define SVRTY_TIME_DATE_STR_SIZE    128
#define SVRTY_FILE_NAME_STR_SIZE    100
#define SVRTY_LOGGING_TID           21

#define SVRTY_STR_ERR       "[ERR] "
#define SVRTY_STR_INF       "[INF] "
#define SVRTY_STR_WNG       "[WNG] "
#define SVRTY_STR_DBG       "[DBG] "

#define SVRTY_TIME_DATE_SIZE    SVRTY_TIME_DATE_STR_SIZE
#define SVRTY_TIME_DATE_FORMAT  "[%c] "

#define SVRTY_LOG_SUCCESS           0
#define SVRTY_LOG_UNINITIALIZED     -1
#define SVRTY_LOG_WNG_SILENT_LVL    -2
#define SVRTY_LOG_ALLOCATION_ERR    -3

#define SVRTY_EXE_FILE_STACK_SIZE       4
#define SVRTY_EXE_FILE_STACK_LVL        3
#define SVRTY_EXE_FILE_ADDR_PREFIX      '('
#define SVRTY_EXE_FILE_SO_SUFFIX        ".so"
#define SVRTY_EXE_FILE_SO_PREFIX        "lib"
#define SVRTY_EXE_FILE_SO_PREFIX_IDX    3
#define SVRTY_EXE_FILE_FORMAT           "[%s] "

#define SVRTY_TID_FORMAT    "[%#lx] "

#define SVRTY_SET_MASK_LEVEL_MASK       0b11110000
#define SVRTY_SET_MASK_TIME_MASK        0b00001000
#define SVRTY_SET_MASK_FILE_NAME_MASK   0b00000100
#define SVRTY_SET_MASK_TID_MASK         0b00000010
#define SVRTY_SET_MASK_SYSLOG_MASK      0b00000001

#define SVRTY_SET_MASK_LEVEL_OFFSET     4
#define SVRTY_SET_MASK_TIME_OFFSET      3
#define SVRTY_SET_MASK_FILE_NAME_OFFSET 2
#define SVRTY_SET_MASK_TID_OFFSET       1
#define SVRTY_SET_MASK_SYSLOG_OFFSET    0

#define SVRTY_GET_MASK_FIELD(VAR_NAME, FIELD_NAME)  ( (VAR_NAME & SVRTY_SET_MASK_##FIELD_NAME##_MASK) >> SVRTY_SET_MASK_##FIELD_NAME##_OFFSET )

#define SVRTY_CLEAN_STR(str)    memset(str, 0, strlen(str))

/***********************************/

/***********************************/
/******** Private variables ********/
/***********************************/

static          bool    is_initialized                          = false                         ;
static          bool    resources_freed                         = false                         ;
static __thread char    severity_color_str[SVRTY_CLR_STR_SIZE]  = {0}                           ;
static __thread char    time_date_str[SVRTY_TIME_DATE_STR_SIZE] = {0}                           ;
static __thread char    severity_level_str[SVRTY_LVL_STR_SIZE]  = {0}                           ;
static __thread char    file_name_str[SVRTY_FILE_NAME_STR_SIZE] = {0}                           ;
static __thread char    logging_TID[SVRTY_LOGGING_TID]          = {0}                           ;
static          char*   log_str_buffer                          = NULL                          ;
static          MTX_GRD log_buff_mtx                            = {0}                           ;
static          int     log_str_payload_size                    = SVRTY_LOG_STR_DEFAULT_SIZE + 1;
static          int     severity_log_mask                       = SVRTY_LOG_MASK_EIW            ;
static          bool    print_time_status                       = false                         ;
static          bool    print_exe_file_name                     = false                         ;
static          bool    log_to_syslog                           = false                         ;
static          bool    log_TID                                 = false                         ;
static          bool    ignore_leading_lib_nums                 = true                          ;

/***********************************/

/*************************************/
/**** Private function prototypes ****/
/*************************************/

__attribute__((constructor)) static void SeverityLogLoad(void);
__attribute__((destructor)) static void SeverityLogUnload(void);

static void SeverityLogCleanup(void);
static void SeverityLogHandleSignal(const int signal_number);

static void ChangeSeverityColor(const int severity);
static void ResetSeverityColor(void);
static void PrintSeverityLevel(const int severity);
static void PrintTime(void);
static void PrintCallingExeFileName(void);
static int  SeverityLogGetSyslogMsgType(const int severity);
static void SeverityLogSyslog(const int severity, const size_t buffer_len);
static int  CheckSeverityLogMask(const int severity);
static void SeverityLogTokenizeCRLF();

/*************************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

///////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Function to be called when current library is loaded. Sets up common signal handler.
/////////////////////////////////////////////////////////////////////////////////////////////// 
__attribute__((constructor)) static void SeverityLogLoad(void)
{
    resources_freed = false;

    SignalHandlerAddCallback(SeverityLogHandleSignal, SIG_HDL_ALL_SIGNALS_MASK);

    MTX_GRD_INIT(&log_buff_mtx);
    MTX_GRD_ATTR_INIT_SC(   &log_buff_mtx           ,
                            PTHREAD_MUTEX_RECURSIVE ,
                            PTHREAD_PRIO_INHERIT    ,
                            PTHREAD_PROCESS_PRIVATE ,
                            p_log_buff_mtx_attr     );

    MTX_GRD_INIT(&log_buff_mtx);
}

////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Function to be called when current library is loaded. Performs resources cleanup.
////////////////////////////////////////////////////////////////////////////////////////////
__attribute__((destructor)) static void SeverityLogUnload(void)
{
    SeverityLogCleanup();
}

/////////////////////////////////////////////////////////////////////////////
/// @brief Performs resources cleanup for current library (frees log buffer).
/////////////////////////////////////////////////////////////////////////////
static void SeverityLogCleanup(void)
{
    if(resources_freed)
        return;
    
    resources_freed = true;

    MTX_GRD_LOCK(&log_buff_mtx);

    SVRTY_LOG_DBG(SVRTY_MSG_CLEANUP);

    if(log_to_syslog)
        closelog();

    if(log_str_buffer)
    {
        free(log_str_buffer);
        log_str_buffer = NULL;
    }

    MTX_GRD_UNLOCK(&log_buff_mtx);
    MTX_GRD_DESTROY(&log_buff_mtx);
}

////////////////////////////////////////////////////////////
/// @brief Common signal handler. Executed cleanup function.
/// @param signal_number Target signal number.
////////////////////////////////////////////////////////////
static void SeverityLogHandleSignal(const int signal_number)
{
    if(resources_freed)
        return;
    
    resources_freed = true;

    SeverityLogCleanup();
}

/////////////////////////////////////////////////////////////
/// @brief Changes log color depending on the severity level.
/// @param severity Severity level (ERR, INF, WNG, DBG)
/////////////////////////////////////////////////////////////
static void ChangeSeverityColor(const int severity)
{
    SVRTY_CLEAN_STR(severity_color_str);

    snprintf(   severity_color_str          ,
                sizeof(severity_color_str)  ,
                SVRTY_CHG_CLR               ,
                (SVRTY_CLR_BASE + severity) );
}

///////////////////////////////////////
/// @brief Resets log color to default.
///////////////////////////////////////
static void ResetSeverityColor(void)
{
    SVRTY_CLEAN_STR(severity_color_str);

    snprintf(   severity_color_str          ,
                sizeof(severity_color_str)  ,
                SVRTY_RST_CLR               );
}

///////////////////////////////////////////////////////////////
/// @brief Prints a string at the beginning of the log message.
/// @param severity Severity level (ERR, INF, WNG, DBG)
///////////////////////////////////////////////////////////////
static void PrintSeverityLevel(const int severity)
{
    char* severity_level_string_ptr = SVRTY_EMPTY_STR;

    switch(severity)
    {
        case SVRTY_LVL_ERR:
            severity_level_string_ptr = SVRTY_STR_ERR;
        break;

        case SVRTY_LVL_INF:
            severity_level_string_ptr = SVRTY_STR_INF;
        break;

        case SVRTY_LVL_WNG:
            severity_level_string_ptr = SVRTY_STR_WNG;
        break;

        case SVRTY_LVL_DBG:
            severity_level_string_ptr = SVRTY_STR_DBG;
        break;

        default:
            return;
        break;
    }

    SVRTY_CLEAN_STR(severity_level_str);

    snprintf(   severity_level_str          ,
                sizeof(severity_level_str)  ,
                "%s"                        ,
                severity_level_string_ptr   );
}

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Sets severity log buffer payload size.
/// @param buffer_size Target payload size (a trailing zero is used to ensure safety).
/// @return 0 if allocation was successful, < 0 otherwise.
//////////////////////////////////////////////////////////////////////////////////////
int SetSeverityLogBufferSize(size_t buffer_size)
{
    MTX_GRD_LOCK_SC(&log_buff_mtx, p_log_buff_mtx);

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
void SetSeverityLogMask(const uint8_t mask)
{
    severity_log_mask = mask;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Checks the current severity log mask.  
/// @param severity Target message severity level.
/// @return SVRTY_LOG_SUCCESS if the severity level is allowed, SVRTY_LOG_WNG_SILENT_LVL otherwise.
///////////////////////////////////////////////////////////////////////////////////////////////////
static int CheckSeverityLogMask(const int severity)
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
void SetSeverityLogPrintTimeStatus(const bool time_status)
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
    
    SVRTY_CLEAN_STR(time_date_str);

    snprintf(   time_date_str           ,
                sizeof(time_date_str)   ,
                "%s"                    ,
                time_str                );
}

/////////////////////////////////////////////////////////////
/// @brief Set value of print_exe_file_name private variable.
/// @param exe_name_status Target status value (T/F).
/////////////////////////////////////////////////////////////
void SetSeverityLogPrintExeNameStatus(const bool exe_name_status)
{
    print_exe_file_name = exe_name_status;
}

/////////////////////////////////////////////////////
/// @brief Set value of log_TID private variable.
/// @param exe_name_status Target status value (T/F).
/////////////////////////////////////////////////////
void SetSeverityLogPrintTID(const bool print_TID_status)
{
    log_TID = print_TID_status;
}

////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Set syslog print variable status (tells whether messages should be logged to syslog).
/// @param log_to_syslog_status Target status value (T/F).
////////////////////////////////////////////////////////////////////////////////////////////////
void SetSeverityLogSyslogStatus(const bool log_to_syslog_status)
{
    if(log_to_syslog_status)
        openlog(NULL, LOG_PID, LOG_USER);
    else
        closelog();

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

        // Remove leading numbers if needed. Although exotic, it may be required if leading numbers were used to specify linking order.
        if(ignore_leading_lib_nums)
            while(*file_name >= '0' && *file_name <= '9')
                ++file_name;

        SVRTY_CLEAN_STR(file_name_str);

        snprintf(   file_name_str           ,
                    sizeof(file_name_str)   ,
                    SVRTY_EXE_FILE_FORMAT   ,
                    file_name               );
    }

    if (symbols != NULL)
        free(symbols);
}

static void PrintTID(void)
{
    if(!log_TID)
        return;

    SVRTY_CLEAN_STR(logging_TID);

    sprintf(logging_TID, SVRTY_TID_FORMAT, pthread_self());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Returns associated syslog message type based on given severity level.
/// @param severity Provided sverity log level.
/// @return Syslog message type.
////////////////////////////////////////////////////////////////////////////////
static int SeverityLogGetSyslogMsgType(const int severity)
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

//////////////////////////////////////////////////////////////////////////////
/// @brief Logs to syslog or journal (using syslog funcitons).
/// @param severity Severity level.
/// @param buffer_len Target buffer length (required because of tokenization).
//////////////////////////////////////////////////////////////////////////////
static void SeverityLogSyslog(const int severity, const size_t buffer_len)
{
    if(!log_to_syslog)
        return;

    int syslog_msg_type = SeverityLogGetSyslogMsgType(severity);
    
    if(syslog_msg_type < 0)
        return;

    char *ptr   = log_str_buffer;
    char *end   = log_str_buffer + buffer_len;

    while (ptr < end)
    {
        if (*ptr != SVRTY_STR_END)
        {
            syslog( syslog_msg_type     ,
                    "%s%s%s%s"          ,
                    severity_level_str  ,
                    file_name_str       ,
                    logging_TID         ,
                    ptr                 );
            ptr += (strlen(ptr) + 1);
        }
        else
        {
            ++ptr;
        }
    }
}

/////////////////////////////////////////////////////////////////////
/// @brief Inits severity Log functionality by using a settings mask.
/// @param buffer_size Target buffer payload size.
/// @param init_mask Mask including the following fields:
///  0b11110000 -> Severity level mask.
///  0b00001000 -> Print time bit.
///  0b00000100 -> Print calling exe file name bit.
///  0b00000010 -> Print calling thread's TID bit.
///  0b00000001 -> Log to syslog bit.
/// @return 0 if succeeded, < 0 otherwise.
/////////////////////////////////////////////////////////////////////
int SeverityLogInitWithMask(const size_t buffer_size, const uint8_t init_mask)
{
    return SeverityLogInit( buffer_size,
                            (const uint8_t) (SVRTY_GET_MASK_FIELD(init_mask, LEVEL))    ,
                            (const bool)    (SVRTY_GET_MASK_FIELD(init_mask, TIME))     ,
                            (const bool)    (SVRTY_GET_MASK_FIELD(init_mask, FILE_NAME)),
                            (const bool)    (SVRTY_GET_MASK_FIELD(init_mask, TID))      ,
                            (const bool)    (SVRTY_GET_MASK_FIELD(init_mask, SYSLOG))   );
}

/////////////////////////////////////////////////////////////
/// @brief Inits severity Log functionality.
/// @param buffer_size Target buffer payload size.
/// @param severity_level_mask Target severity level(s) mask.
/// @param print_time Print log's time and date (T/F).
/// @param print_exe_file Print logging file's name (T/F).
/// @param print_TID Print logging thread's TID (T/F).
/// @param log_to_syslog Log to syslog/journal file (T/F).
/// @return 0 if succeeded, < 0 otherwise.
/////////////////////////////////////////////////////////////
int SeverityLogInit(const size_t buffer_size            ,
                    const uint8_t severity_level_mask   ,
                    const bool print_time               ,
                    const bool print_exe_file           ,
                    const bool print_TID                ,
                    const bool log_to_syslog            )
{
    int set_buffer_size = SetSeverityLogBufferSize(buffer_size);
    
    if(set_buffer_size < 0)
        return set_buffer_size;
    
    SetSeverityLogMask(severity_level_mask);
    SetSeverityLogPrintTimeStatus(print_time);
    SetSeverityLogPrintExeNameStatus(print_exe_file);
    SetSeverityLogSyslogStatus(log_to_syslog);
    SetSeverityLogPrintTID(print_TID);

    SVRTY_LOG_DBG(SVRTY_MSG_INIT);

    is_initialized = true;

    return SVRTY_LOG_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Tells whether names used to order libraries should be ignored or not when printing calling file name.
/// @param ignore_lead_nums Ignore library name's leading numbers (after "lib").
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SeverityLogIgnoreLeadLibNameNums(bool ignore_lead_nums)
{
    ignore_leading_lib_nums = ignore_lead_nums;
}

///////////////////////////////////////////////////////////////////////
/// @brief Tokenizes log buffer using "\n" and/or "\r\n" as delimiters.
///////////////////////////////////////////////////////////////////////
static void SeverityLogTokenizeCRLF(void)
{
    size_t cur_log_str_buffer_len = strlen(log_str_buffer);
    size_t i = 0;

    MTX_GRD_LOCK_SC(&log_buff_mtx, p_log_buff_mtx);

    // Replace "\r\n" or "\n" with "\0" or "\0\0" respectively.
    while (i < cur_log_str_buffer_len)
    {
        if (log_str_buffer[i] == SVRTY_CR && log_str_buffer[i + 1] == SVRTY_LF)
        {
            log_str_buffer[i] = SVRTY_STR_END;
            log_str_buffer[i + 1] = SVRTY_STR_END;
            i += 2;
        }
        else if (log_str_buffer[i] == SVRTY_LF)
        {
            log_str_buffer[i] = SVRTY_STR_END;
            i++;
        }
        else
        {
            i++;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Prints a log with different color and initial string depending on the severity level.
/// @param severity Severity level (ERR, INF, WNG).
/// @param format Formatted string. Same as what can be used with printf.
/// @param ... Variable number of arguments. Data that is meant to be formatted and printed.
/// @return < 0 if any error happened, number of characters written to stream otherwise.
//////////////////////////////////////////////////////////////////////////////////////////////////
int SeverityLog(const uint8_t severity, const char* restrict format, ...)
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

    ChangeSeverityColor(severity);
    PrintTime();

    PrintSeverityLevel(severity);
    PrintCallingExeFileName();
    PrintTID();

    va_list args;
    int done;

    va_start(args, format);
    
    MTX_GRD_LOCK_SC(&log_buff_mtx, p_log_buff_mtx);

    done = vsnprintf(   (log_str_buffer + strlen(log_str_buffer))       ,
                        (log_str_payload_size - strlen(log_str_buffer)) ,
                        format                                          ,
                        args                                            );
    
    va_end(args);

    log_str_buffer[strlen(log_str_buffer)] = SVRTY_STR_END;

    size_t cur_log_str_buffer_len = strlen(log_str_buffer);
    
    SeverityLogTokenizeCRLF();

    SeverityLogSyslog(severity, cur_log_str_buffer_len);

    // Iterate over tokens
    char *ptr   = log_str_buffer;
    char *end   = log_str_buffer + cur_log_str_buffer_len;

    while (ptr < end)
    {
        if (*ptr != SVRTY_STR_END)
        {
            printf( "%s%s%s%s%s%s%s%s"  ,
                    severity_color_str  ,
                    time_date_str       ,
                    severity_level_str  ,
                    file_name_str       ,
                    logging_TID         ,
                    ptr                 ,
                    SVRTY_RST_CLR       ,
                    SVRTY_CRLF          );
            fflush(stdout);
            ptr += (strlen(ptr) + 1);
        }
        else
        {
            ++ptr;
        }
    }

    ResetSeverityColor();

    SVRTY_CLEAN_STR(log_str_buffer);

    return done;
}

/*************************************/
