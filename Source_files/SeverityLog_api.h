#ifndef SEVERITY_LOG_API_H
#define SEVERITY_LOG_API_H

#define SVRTY_LVL_ERR   1
#define SVRTY_LVL_INF   2
#define SVRTY_LVL_WNG   3

int SeverityLog(int severity, const char* format, ...);

#endif