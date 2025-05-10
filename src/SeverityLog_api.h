#ifndef SEVERITY_LOG_API_H
#define SEVERITY_LOG_API_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************/
/******** Include statements ********/
/************************************/

#include <stdbool.h>

/************************************/

/***********************************/
/******** Define statements ********/
/***********************************/

#define C_SEVERITY_LOG_API __attribute__((visibility("default")))

#define SVRTY_LVL_ERR   1
#define SVRTY_LVL_INF   2
#define SVRTY_LVL_WNG   3
#define SVRTY_LVL_DBG   4

#define SVRTY_LOG_MASK_OFF  0b0000
#define SVRTY_LOG_MASK_ERR  0b0001
#define SVRTY_LOG_MASK_INF  0b0010
#define SVRTY_LOG_MASK_WNG  0b0100
#define SVRTY_LOG_MASK_DBG  0b1000
#define SVRTY_LOG_MASK_EIW  0b0111 // EIW stands for ERR, INF, WNG
#define SVRTY_LOG_MASK_ALL  0b1111

/***********************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Sets severity log buffer payload size.
/// @param buffer_size Target payload size (a trailing zero is used to ensure safety).
/// @return 0 if allocation was successful, < 0 otherwise.
//////////////////////////////////////////////////////////////////////////////////////
C_SEVERITY_LOG_API int SetSeverityLogBufferSize(unsigned long buffer_size);

/////////////////////////////////////////////////////
/// @brief Sets severity log mask to the input value.
/// @param mask Target severity log mask.
/////////////////////////////////////////////////////
C_SEVERITY_LOG_API void SetSeverityLogMask(const int mask);

///////////////////////////////////////////////////////////
/// @brief Set value of print_time_status private variable.
/// @param time_status Target status value (T/F).
///////////////////////////////////////////////////////////
C_SEVERITY_LOG_API void SetSeverityLogPrintTimeStatus(const bool time_status);

/////////////////////////////////////////////////////////////
/// @brief Set value of print_exe_file_name private variable.
/// @param exe_name_status Target status value (T/F).
/////////////////////////////////////////////////////////////
C_SEVERITY_LOG_API void SetSeverityLogPrintExeNameStatus(const bool exe_name_status);

////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Set syslog print variable status (tells whether messages should be logged to syslog).
/// @param log_to_syslog_status Target status value (T/F).
////////////////////////////////////////////////////////////////////////////////////////////////
C_SEVERITY_LOG_API void SetSeverityLogSyslogStatus(const bool log_to_syslog_status);

/////////////////////////////////////////////////////////////
/// @brief Sets multiple severity log parameters at once.
/// @param buffer_size Target buffer payload size.
/// @param severity_level_mask Target severity level(s) mask.
/// @param print_time Print log's time and date (T/F).
/// @param print_exe_file Print logging file's name (T/F).
/// @return 0 if succeeded, < 0 otherwise.
/////////////////////////////////////////////////////////////
C_SEVERITY_LOG_API int SeverityLogInit( const unsigned long buffer_size ,
                                        const int  severity_level_mask  ,
                                        const bool print_time           ,
                                        const bool print_exe_file       ,       
                                        const bool log_to_syslog        );

//////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Prints a log with different color and initial string depending on the severity level.
/// @param severity Severity level (ERR, INF, WNG).
/// @param format Formatted string. Same as what can be used with printf.
/// @param  Variable Variable number of arguments. Data that is meant to be formatted and printed.
/// @return < 0 if any error happened, number of characters written to stream otherwise.
//////////////////////////////////////////////////////////////////////////////////////////////////
C_SEVERITY_LOG_API int SeverityLog(const int severity, const char* restrict format, ...);

#define SVRTY_LOG_ERR(...) SeverityLog(SVRTY_LVL_ERR, __VA_ARGS__)
#define SVRTY_LOG_INF(...) SeverityLog(SVRTY_LVL_INF, __VA_ARGS__)
#define SVRTY_LOG_WNG(...) SeverityLog(SVRTY_LVL_WNG, __VA_ARGS__)
#define SVRTY_LOG_DBG(...) SeverityLog(SVRTY_LVL_DBG, __VA_ARGS__)

/*************************************/

#ifdef __cplusplus
}
#endif

#endif