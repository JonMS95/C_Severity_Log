/************************************/
/******** Include statements ********/
/************************************/

#include "SeverityLog_api.h"

/************************************/

/***********************************/
/******** Define statements ********/
/***********************************/

#define TEST_LOG_BUFFER_SIZE    1000

#define TEST_MSG_ERR "this is an ERROR message."
#define TEST_MSG_INF "this is an INFORMATION message."
#define TEST_MSG_WNG "this is a WARNING message."
#define TEST_MSG_DBG "this is a DEBUG message."

#define TEST_MSG_MULTIPLE_LINES "This is line 1\nThis is line 2\r\nThis is line 3"

#define TEST_MSG_HEADER     "******** Test %d ********"
#define TEST_MSG_RESULT     "Test %d %s.\n"
#define TEST_MSG_FAILED     "failed"
#define TEST_MSG_SUCCEEDED  "succeeded"
#define TEST_MSG_FAILURE    "TESTS FAILED.\n"
#define TEST_MSG_SUCCESS    "TESTS SUCCEDED"

/***********************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief For a given severity log mask, check that only the specified messages are shown.
/// @param severity_log_mask target severity log mask to be used. Reset at the end of the function.
/// @return < 0 if any error happened, number of characters written to stream otherwise.
///////////////////////////////////////////////////////////////////////////////////////////////////
int PrintAllMessages(int severity_log_mask)
{
    SetSeverityLogMask(severity_log_mask);

    int test_result = 0;
    
    int test_err_bit = SVRTY_LOG_ERR(TEST_MSG_ERR);
    int test_inf_bit = SVRTY_LOG_INF(TEST_MSG_INF);
    int test_wng_bit = SVRTY_LOG_WNG(TEST_MSG_WNG);
    int test_dbg_bit = SVRTY_LOG_DBG(TEST_MSG_DBG);

    SetSeverityLogMask(SVRTY_LOG_MASK_OFF);

    test_result =   ( (test_dbg_bit >= 0 ? 1 : 0) << 3) +
                    ( (test_wng_bit >= 0 ? 1 : 0) << 2) +
                    ( (test_inf_bit >= 0 ? 1 : 0) << 1) +
                      (test_err_bit >= 0 ? 1 : 0)       ;

    return test_result;
}

void PrintMultiLineMessage(void)
{
    SetSeverityLogMask(SVRTY_LOG_MASK_ALL);

    SVRTY_LOG_INF(TEST_MSG_MULTIPLE_LINES);

    SetSeverityLogMask(SVRTY_LOG_MASK_OFF);
}

int main()
{
    int severity_log_masks[] = {SVRTY_LOG_MASK_OFF,
                                SVRTY_LOG_MASK_ERR,
                                SVRTY_LOG_MASK_INF,
                                SVRTY_LOG_MASK_WNG,
                                SVRTY_LOG_MASK_DBG,
                                SVRTY_LOG_MASK_EIW,
                                SVRTY_LOG_MASK_ALL};
    int test_result = 0;
    int test_result_log_mask = 0;

    SeverityLogInit(TEST_LOG_BUFFER_SIZE, SVRTY_LOG_MASK_ALL, true, true, true);

    for(int i = 0; i < (sizeof(severity_log_masks) / sizeof(severity_log_masks[0])); i++)
    {
        SetSeverityLogMask(SVRTY_LOG_MASK_INF);

        SVRTY_LOG_INF(TEST_MSG_HEADER, i + 1);

        SetSeverityLogMask(SVRTY_LOG_MASK_OFF);

        test_result = PrintAllMessages(severity_log_masks[i]);
        
        test_result_log_mask = (test_result == severity_log_masks[i] ? SVRTY_LOG_MASK_INF : SVRTY_LOG_MASK_ERR);

        SetSeverityLogMask(test_result_log_mask);

        SeverityLog(test_result_log_mask, TEST_MSG_RESULT, i + 1, (test_result < 0 ? TEST_MSG_FAILED : TEST_MSG_SUCCEEDED));

        if(test_result < 0)
        {
            SeverityLog(test_result_log_mask, TEST_MSG_FAILURE);
            return -1;
        }

        SetSeverityLogMask(SVRTY_LOG_MASK_OFF);
    }

    SetSeverityLogMask(SVRTY_LOG_MASK_INF);

    SVRTY_LOG_INF(TEST_MSG_SUCCESS);

    PrintMultiLineMessage();

    return 0;
}

/*************************************/
