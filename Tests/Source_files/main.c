#include "SeverityLog_api.h"

#define TEST_MSG_ERR "this is an ERROR message."
#define TEST_MSG_INF "this is an INFORMATION message."
#define TEST_MSG_WNG "this is a WARNING message."
#define TEST_MSG_DBG "this is a DEBUG message."


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief For a given severity log mask, check that only the specified messages are shown.
/// @param severity_log_mask target severity log mask to be used. Reset at the end of the function.
/// @return < 0 if any error happened, number of characters written to stream otherwise.
///////////////////////////////////////////////////////////////////////////////////////////////////
int PrintAllMessages(int severity_log_mask)
{
    SetSeverityLogMask(severity_log_mask);

    int test_result = 0;
    
    int test_err_bit = LOG_ERR(TEST_MSG_ERR);
    int test_inf_bit = LOG_INF(TEST_MSG_INF);
    int test_wng_bit = LOG_WNG(TEST_MSG_WNG);
    int test_dbg_bit = LOG_DBG(TEST_MSG_DBG);

    SetSeverityLogMask(SVRTY_LOG_MASK_OFF);

    test_result =   ( (test_dbg_bit >= 0 ? 1 : 0) << 3) +
                    ( (test_wng_bit >= 0 ? 1 : 0) << 2) +
                    ( (test_inf_bit >= 0 ? 1 : 0) << 1) +
                      (test_err_bit >= 0 ? 1 : 0)       ;

    return test_result;
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

    for(int i = 0; i < (sizeof(severity_log_masks) / sizeof(severity_log_masks[0])); i++)
    {
        SetSeverityLogMask(SVRTY_LOG_MASK_INF);

        LOG_INF("******** Test %d ********", i + 1);

        SetSeverityLogMask(SVRTY_LOG_MASK_OFF);

        test_result = PrintAllMessages(severity_log_masks[i]);
        
        test_result_log_mask = (test_result == severity_log_masks[i] ? SVRTY_LOG_MASK_INF : SVRTY_LOG_MASK_ERR);

        SetSeverityLogMask(test_result_log_mask);

        SeverityLog(test_result_log_mask, "Test %d %s.\r\n", i + 1, (test_result < 0 ? "failed" : "succeed"));

        if(test_result < 0)
        {
            SeverityLog(test_result_log_mask, "TESTS FAILED.\r\n");
            return -1;
        }

        SetSeverityLogMask(SVRTY_LOG_MASK_OFF);
    }

    SetSeverityLogMask(SVRTY_LOG_MASK_INF);

    LOG_INF("TESTS SUCCEDED");

    SetSeverityLogMask(SVRTY_LOG_MASK_OFF);

    return 0;
}