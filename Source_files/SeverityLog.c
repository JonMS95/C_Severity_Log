#include <stdio.h>
#include <stdarg.h>
#include "SeverityLog.h"

/////////////////////////////////////////////////////////////
/// @brief Changes log color depending on the severity level.
/// @param severity Severity level (ERR, INF, WNG)
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
/// @param severity Severity level (ERR, INF, WNG)
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

        default:
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Prints a log with different color and initial string depending on the severity level.
/// @param severity Severity level (ERR, INF, WNG).
/// @param format Formatted string. Same as what can be used with printf.
/// @param  Variable Variable number of arguments. Data that is meant to be formatted and printed.
/// @return < 0 if any error hapenned, number of characters written to stream otherwise.
//////////////////////////////////////////////////////////////////////////////////////////////////
int SeverityLog(int severity, const char* format, ...)
{
    va_list args;
    int done;

    ChangeSeverityColor(severity);

    PrintSeverityLevel(severity);

    va_start(args, format);
    done = vfprintf(stdout, format, args);
    va_end(args);

    ResetSeverityColor();

    return done;
}

int main()
{
    SeverityLog(SVRTY_LVL_INF, "Hello, my name is \"%s\"\r\n", "Jon");
    SeverityLog(SVRTY_LVL_ERR, "I'm %d years old\r\n", 27);
    SeverityLog(SVRTY_LVL_WNG, "Soon, I will become %.2f.\r\n", 28.1);
    
    return 0;
}