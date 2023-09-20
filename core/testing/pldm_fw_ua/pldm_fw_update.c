// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stdint.h>
#include <string.h>
#include "testing.h"
#include "base.h"
#include "utils.h"
#include "firmware_update.h"
#include "mctp/mctp_interface.h"
#include "platform_io.h"


TEST_SUITE_LABEL ("pldm_fw_update");

/**
 * Helper Functions
*/

static void printBuf(uint8_t *buf, size_t size) {
    for (size_t i = 0; i < size; i++) {
        platform_printf("%02x ", buf[i]);
    }
    platform_printf("\n");
}

/**
 * Test Functions
*/

static void pldm_fw_update_test_place_holder(CuTest *test)
{
    TEST_START;

    CuAssertIntEquals(test, 0, 0);
}

static void pldm_fw_update_test_request_update_req(CuTest *test)
{
    uint8_t instanceId = 1;
    int status;

    const char *compImgSetVerStrArr = "0penBmcv1.0";
    uint8_t compImgSetVerStrLen = strlen(compImgSetVerStrArr);

    struct variable_field inCompImgSetVerStr;
    inCompImgSetVerStr.ptr = (const uint8_t*) compImgSetVerStrArr;
    inCompImgSetVerStr.length = compImgSetVerStrLen;

    struct request_update_req inReq = {};

    inReq.max_transfer_size = 512;
    inReq.no_of_comp = 3;
    inReq.max_outstand_transfer_req = 2;
    inReq.pkg_data_len = 0x1234;
    inReq.comp_image_set_ver_str_type = PLDM_COMP_ASCII;
    inReq.comp_image_set_ver_str_len = compImgSetVerStrLen;

    uint8_t pldmBuf[sizeof (struct pldm_msg_hdr) + sizeof (struct request_update_req) + compImgSetVerStrLen];
    struct pldm_msg *pldmMsg = (struct pldm_msg*) pldmBuf;

    TEST_START;

    status = encode_request_update_req(instanceId, pldmMsg, sizeof (struct request_update_req) + inCompImgSetVerStr.length, 
                                        &inReq, &inCompImgSetVerStr);

    struct request_update_req *outReq = (struct request_update_req *)(pldmBuf + sizeof (struct pldm_msg_hdr)); 

    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, outReq->max_transfer_size, inReq.max_transfer_size);
    CuAssertIntEquals(test, outReq->pkg_data_len, inReq.pkg_data_len);
    CuAssertIntEquals(test, outReq->comp_image_set_ver_str_len, inReq.comp_image_set_ver_str_len);
    printBuf((uint8_t *)inCompImgSetVerStr.ptr, inCompImgSetVerStr.length);
    printBuf(pldmMsg->payload + sizeof (struct request_update_req), inCompImgSetVerStr.length);

    





}

TEST_SUITE_START (pldm_fw_update);

TEST (pldm_fw_update_test_place_holder);
TEST (pldm_fw_update_test_request_update_req);

TEST_SUITE_END;