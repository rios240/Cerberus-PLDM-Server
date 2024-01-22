#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "testing.h"
#include "platform_io.h"
#include "pldm_fwup/pldm_fwup_interface.h"


TEST_SUITE_LABEL ("pldm_fwup_interface");

static void pldm_fwup_test_perform_firmware_update (CuTest *test) 
{
    struct mctp_interface mctp;
    struct device_manager device_mgr;
    struct cmd_interface cmd_mctp;
    struct cmd_interface cmd_spdm;
    struct cmd_interface cmd_cerberus;
    struct cmd_channel cmd_channel;

    TEST_START;

    int status = initialize_firmware_update(&mctp, &cmd_channel, &cmd_mctp, &cmd_spdm, &cmd_cerberus, &device_mgr);
    CuAssertIntEquals(test, 0, status);

    status = perform_firmware_update(&mctp, &cmd_channel);
    CuAssertIntEquals(test, 0, status);


}


TEST_SUITE_START (pldm_fwup_interface);

TEST (pldm_fwup_test_perform_firmware_update);

TEST_SUITE_END;