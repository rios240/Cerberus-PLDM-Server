// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "testing.h"
#include "platform_io.h"
#include "pldm_firmware/pldm_firmware_update_interface.h"


TEST_SUITE_LABEL ("pldm_firmware_update_interface");


static void place_holder_test(CuTest *test)
{
    TEST_START;

    CuAssertIntEquals(test, 0, 0);
}

static void pldm_firmware_update_interface_test_workflow(CuTest *test) {
    struct mctp_interface mctp;
    struct cmd_interface cmd_mctp;
    struct cmd_interface cmd_spdm;
    struct cmd_interface cmd_cerberus;
    struct cmd_channel com;
    struct device_manager device_mgr;
    uint8_t device_eid = 0x05;
    uint8_t device_smbus_addr = 0x4C;
    int status;

    TEST_START;

    status = pldm_firmware_update_init(&mctp, &com, &cmd_mctp, &cmd_spdm, &cmd_cerberus, &device_mgr, device_eid, device_smbus_addr);
    CuAssertIntEquals(test, 0, status);

    status = pldm_firmware_update_run(&mctp, &com, 10000, 0x1E, 0x03);
    CuAssertIntEquals(test, 0, status);

}

TEST_SUITE_START (pldm_firmware_update_interface);

TEST (place_holder_test);
TEST (pldm_firmware_update_interface_test_workflow);

TEST_SUITE_END;