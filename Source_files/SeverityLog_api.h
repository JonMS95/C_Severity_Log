#ifndef SEVERITY_LOG_API_H
#define SEVERITY_LOG_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

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

/////////////////////////////////////////////////////
/// @brief Sets severity log mask to the input value.
/// @param mask Target severity log mask.
/////////////////////////////////////////////////////
void SetSeverityLogMask(int mask);

//////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Prints a log with different color and initial string depending on the severity level.
/// @param severity Severity level (ERR, INF, WNG).
/// @param format Formatted string. Same as what can be used with printf.
/// @param  Variable Variable number of arguments. Data that is meant to be formatted and printed.
/// @return < 0 if any error happened, number of characters written to stream otherwise.
//////////////////////////////////////////////////////////////////////////////////////////////////
int SeverityLog(int severity, const char* format, ...);

#define LOG_ERR(...) SeverityLog(SVRTY_LVL_ERR, __VA_ARGS__)
#define LOG_INF(...) SeverityLog(SVRTY_LVL_INF, __VA_ARGS__)
#define LOG_WNG(...) SeverityLog(SVRTY_LVL_WNG, __VA_ARGS__)
#define LOG_DBG(...) SeverityLog(SVRTY_LVL_DBG, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif