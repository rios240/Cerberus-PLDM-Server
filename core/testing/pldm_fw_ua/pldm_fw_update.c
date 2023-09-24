// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
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

int send_packet(struct cmd_channel *channel, struct cmd_packet *packet) {
    const int port = 5000;
    const char *address = "127.0.0.1";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return CMD_CHANNEL_SOC_INIT_FAILURE;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &(server.sin_addr)) <= 0) {
        close(sock);
        return CMD_CHANNEL_SOC_NET_ADDRESS_FAILURE;
    }

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        close(sock);
        return CMD_CHANNEL_SOC_CONNECT_FAILURE;
    }


    if (send(sock, packet->data, sizeof (packet->data), 0) < 0) {
        close(sock);
        return CMD_CHANNEL_SOC_SEND_FAILURE;
    }

    
    close(sock);

    return 0;
}

int receive_packet(struct cmd_channel *channel, struct cmd_packet *packet, int ms_timeout) {
    const int port = 5000;
    const char *address = "127.0.0.1";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return CMD_CHANNEL_SOC_INIT_FAILURE;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &(server.sin_addr)) <= 0) {
        close(sock);
        return CMD_CHANNEL_SOC_NET_ADDRESS_FAILURE;
    }

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        close(sock);
        return CMD_CHANNEL_SOC_CONNECT_FAILURE;
    }


    if (recv(sock, packet->data, sizeof (packet->data), 0) < 0) {
        close(sock);
        return CMD_CHANNEL_SOC_SEND_FAILURE;
    }

    
    close(sock);

    return 0;
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

    int status;
    uint8_t dest_addr = 0xff;
    struct mctp_interface mctp;
    mctp.msg_type = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct cmd_interface cmd_cerberus;
    struct cmd_interface cmd_mctp;
    struct cmd_interface cmd_spdm;
    struct device_manager device_mgr;
    struct cmd_channel channel;
    channel.send_packet = send_packet;
    channel.receive_packet = receive_packet;
    channel.id = 1;

    uint8_t instanceId = 1;
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

    TEST_START;

    uint8_t pldmBuf[sizeof (struct pldm_msg_hdr) + sizeof (struct request_update_req) + compImgSetVerStrLen];
    struct pldm_msg *pldmMsg = (struct pldm_msg*) pldmBuf;

    status = encode_request_update_req(instanceId, pldmMsg, sizeof (struct request_update_req) + inCompImgSetVerStr.length, 
                                        &inReq, &inCompImgSetVerStr);

    struct request_update_req *outReq = (struct request_update_req *)(pldmBuf + sizeof (struct pldm_msg_hdr)); 

    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, outReq->max_transfer_size, inReq.max_transfer_size);
    CuAssertIntEquals(test, outReq->pkg_data_len, inReq.pkg_data_len);
    CuAssertIntEquals(test, outReq->comp_image_set_ver_str_len, inReq.comp_image_set_ver_str_len);
    printBuf((uint8_t *)inCompImgSetVerStr.ptr, inCompImgSetVerStr.length);
    printBuf(pldmMsg->payload + sizeof (struct request_update_req), inCompImgSetVerStr.length);

    status = device_manager_init(&device_mgr, 1, 1, DEVICE_MANAGER_PA_ROT_MODE, DEVICE_MANAGER_MASTER_BUS_ROLE, 
                1000, 1000, 1000, 1000, 1000, 1000, 5);

    CuAssertIntEquals(test, 0, status);

    status = mctp_interface_init(&mctp, &cmd_cerberus, &cmd_mctp, &cmd_spdm, &device_mgr);

    CuAssertIntEquals(test, 0, status);

    uint8_t mctpBuf[MCTP_BASE_PROTOCOL_MAX_PACKET_LEN];

    status = mctp_interface_issue_request(&mctp, &channel, dest_addr, MCTP_BASE_PROTOCOL_NULL_EID, 
                                            pldmBuf, sizeof (pldmBuf), mctpBuf, sizeof (mctpBuf), 0);

    CuAssertIntEquals(test, 0, status);

}

TEST_SUITE_START (pldm_fw_update);

TEST (pldm_fw_update_test_place_holder);
TEST (pldm_fw_update_test_request_update_req);

TEST_SUITE_END;