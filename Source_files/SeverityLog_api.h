#ifndef SEVERITY_LOG_API_H
#define SEVERITY_LOG_API_H

#define SVRTY_LVL_ERR   1
#define SVRTY_LVL_INF   2
#define SVRTY_LVL_WNG   3
#define SVRTY_LVL_DBG   4   

//////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Prints a log with different color and initial string depending on the severity level.
/// @param severity Severity level (ERR, INF, WNG).
/// @param format Formatted string. Same as what can be used with printf.
/// @param  Variable Variable number of arguments. Data that is meant to be formatted and printed.
/// @return < 0 if any error hapenned, number of characters written to stream otherwise.
//////////////////////////////////////////////////////////////////////////////////////////////////
int SeverityLog(int severity, const char* format, ...);

#endif