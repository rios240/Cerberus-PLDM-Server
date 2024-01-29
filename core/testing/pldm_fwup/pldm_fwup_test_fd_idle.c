#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "testing.h"
#include "platform_io.h"
#include "pldm_fwup/pldm_fwup_interface.h"
#include "pldm_fwup/pldm_fwup_mctp.h"
#include "pldm_fwup/pldm_fwup_commands.h"
#include "firmware_update.h"


TEST_SUITE_LABEL ("pldm_fwup_test_fd_idle");


static void pldm_fwup_test_fd_idle_good_responses (CuTest *test) {

    struct mctp_interface mctp;
    struct device_manager device_mgr;
    struct cmd_interface cmd_mctp;
    struct cmd_interface cmd_spdm;
    struct cmd_interface cmd_cerberus;
    struct cmd_channel cmd_channel;
    struct pldm_fwup_interface *fwup = get_fwup_interface();

    TEST_START;

    int status = initialize_firmware_update(&mctp, &cmd_channel, &cmd_mctp, &cmd_spdm, &cmd_cerberus, &device_mgr, fwup);
    CuAssertIntEquals(test, 0, status);

    status = generate_and_send_pldm_over_mctp(&mctp, &cmd_channel, issue_query_device_identifiers);
    CuAssertIntEquals(test, 0, status);
    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, process_query_device_identifiers);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_SUCCESS, fwup->completion_code);

    status = generate_and_send_pldm_over_mctp(&mctp, &cmd_channel, issue_get_firmware_parameters);
    CuAssertIntEquals(test, 0, status);
    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, process_get_firmware_parameters);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_SUCCESS, fwup->completion_code);

    status = generate_and_send_pldm_over_mctp(&mctp, &cmd_channel, issue_request_update);
    CuAssertIntEquals(test, 0, status);
    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, process_request_update);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_SUCCESS, fwup->completion_code);

    clean_up_and_reset_firmware_update(&mctp, fwup);

}

static void pldm_fwup_test_fd_idle_retry_request_update (CuTest *test) {

    struct mctp_interface mctp;
    struct device_manager device_mgr;
    struct cmd_interface cmd_mctp;
    struct cmd_interface cmd_spdm;
    struct cmd_interface cmd_cerberus;
    struct cmd_channel cmd_channel;
    struct pldm_fwup_interface *fwup = get_fwup_interface();

    TEST_START;

    int status = initialize_firmware_update(&mctp, &cmd_channel, &cmd_mctp, &cmd_spdm, &cmd_cerberus, &device_mgr, fwup);
    CuAssertIntEquals(test, 0, status);

    status = generate_and_send_pldm_over_mctp(&mctp, &cmd_channel, issue_request_update);
    CuAssertIntEquals(test, 0, status);
    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, process_request_update);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, RETRY_REQUEST_UPDATE, fwup->completion_code);

    status = generate_and_send_pldm_over_mctp(&mctp, &cmd_channel, issue_request_update);
    CuAssertIntEquals(test, 0, status);
    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, process_request_update);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_SUCCESS, fwup->completion_code);
    

    clean_up_and_reset_firmware_update(&mctp, fwup);


}

static void pldm_fwup_test_fd_idle_unable_to_initiate_update (CuTest *test) {

    struct mctp_interface mctp;
    struct device_manager device_mgr;
    struct cmd_interface cmd_mctp;
    struct cmd_interface cmd_spdm;
    struct cmd_interface cmd_cerberus;
    struct cmd_channel cmd_channel;
    struct pldm_fwup_interface *fwup = get_fwup_interface();

    TEST_START;

    int status = initialize_firmware_update(&mctp, &cmd_channel, &cmd_mctp, &cmd_spdm, &cmd_cerberus, &device_mgr, fwup);
    CuAssertIntEquals(test, 0, status);

    status = generate_and_send_pldm_over_mctp(&mctp, &cmd_channel, issue_request_update);
    CuAssertIntEquals(test, 0, status);
    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, process_request_update);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, UNABLE_TO_INITIATE_UPDATE, fwup->completion_code);

    clean_up_and_reset_firmware_update(&mctp, fwup);


}



TEST_SUITE_START (pldm_fwup_test_fd_idle);

TEST (pldm_fwup_test_fd_idle_good_responses);
TEST (pldm_fwup_test_fd_idle_retry_request_update);
TEST (pldm_fwup_test_fd_idle_unable_to_initiate_update);

TEST_SUITE_END;