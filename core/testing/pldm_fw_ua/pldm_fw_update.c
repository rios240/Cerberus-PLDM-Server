// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <unistd.h>
#include "testing.h"
#include "base.h"
#include "utils.h"
#include "firmware_update.h"
#include "mctp/mctp_interface.h"
#include "platform_io.h"
#include "crypto/checksum.h"


TEST_SUITE_LABEL ("pldm_fw_update");

/**
 * Helper Functions
*/

static void platform_printbuf(uint8_t *buf, size_t size) {
    for (size_t i = 0; i < size; i++) {
        platform_printf("%02x ", buf[i]);
    }
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


    if (send(sock, packet->data, packet->pkt_size, 0) < 0) {
        close(sock);
        return CMD_CHANNEL_SOC_SEND_FAILURE;
    }

    
    close(sock);

    return 0;
}

int receive_packet(struct cmd_channel *channel, struct cmd_packet *packet, int ms_timeout) {
    sleep(15);
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

    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = ms_timeout / 1000; 
    timeout.tv_usec = (ms_timeout % 1000) * 1000; 

    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    int selectResult = select(sock + 1, &readfds, NULL, NULL, &timeout);

    if (selectResult == -1) {
        close(sock);
        return CMD_CHANNEL_SOC_SELECT_FAILURE;
    } else if (selectResult == 0) {
        close(sock);
        return CMD_CHANNEL_SOC_TIMEOUT;
    }

    ssize_t bytes = recv(sock, packet->data, MCTP_BASE_PROTOCOL_MAX_PACKET_LEN, 0);

    if (bytes < 0) {
        close(sock);
        return CMD_CHANNEL_SOC_SEND_FAILURE;
    }

    packet->pkt_size = bytes;
    packet->dest_addr = 0xDD;

    platform_printf("\nServer received the following MCTP from the client: \n");
    platform_printbuf(packet->data, packet->pkt_size);
    platform_printf(NEWLINE);
    
    close(sock);

    return 0;
}

int process_request(struct cmd_interface *intf, struct cmd_interface_msg *request) {
    return PLDM_ERROR;
}

int process_response(struct cmd_interface *intf, struct cmd_interface_msg *response) {
    uint16_t fd_meta_data_len = 0;
    uint8_t fd_pkg_data = 0;
    uint8_t completion_code = 0;
    int status;

    struct pldm_msg *respMsg = (struct pldm_msg *)&response->data[1];

    platform_printf("After processing the MCTP message and extracting the payload, the server enters the process_response phase.\n");
    platform_printf("Decoding the payload/PLDM gives the following fields.\n\n");
    status = decode_request_update_resp(respMsg, sizeof (struct pldm_request_update_resp), 
                                &completion_code, &fd_meta_data_len, &fd_pkg_data);
    
    platform_printf("Completion Code is: %d,\n", completion_code);
    platform_printf("FDWillSendGetPackageDataCommand is: %d,\n", fd_pkg_data);
    platform_printf("FirmwareDeviceMetaDataLength is: %d\n\n", fd_meta_data_len);

    response->length = 0;

    return status;
}

/**
 * Test Functions
*/

static void pldm_fw_update_test_place_holder(CuTest *test)
{
    TEST_START;

    CuAssertIntEquals(test, 0, 0);
}

static void pldm_fw_update_test_request_update_req_good_response(CuTest *test)
{

    int status;
    struct mctp_interface mctp;
    mctp.msg_type = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;

    struct cmd_interface cmd_cerberus;
    struct cmd_interface cmd_mctp;
    cmd_mctp.process_request = process_request;
    cmd_mctp.process_response = process_response;
    struct cmd_interface cmd_spdm;

    struct device_manager device_mgr;

    struct cmd_channel channel;
    channel.send_packet = send_packet;
    channel.receive_packet = receive_packet;
    channel.id = 1;

    uint8_t dest_addr = 0xAA;

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

    uint8_t pldmBuf[sizeof (struct pldm_msg_hdr) + sizeof (struct request_update_req) + compImgSetVerStrLen + 1];
    pldmBuf[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *pldmMsg = (struct pldm_msg*)&pldmBuf[1];

    platform_printf("\nServer encodes the RequestUpdate PLDM request.\n");
    status = encode_request_update_req(instanceId, pldmMsg, sizeof (struct request_update_req) + inCompImgSetVerStr.length, 
                                        &inReq, &inCompImgSetVerStr);
    CuAssertIntEquals(test, 0, status);

    platform_printf("The PLDM header->{");
    platform_printbuf(&pldmBuf[1], sizeof (struct pldm_msg_hdr));
    platform_printf("} and PLDM payload->{");
    platform_printbuf(&pldmBuf[1 + sizeof (struct pldm_msg_hdr)], sizeof (struct request_update_req) + inCompImgSetVerStr.length);
    platform_printf("}.\n\n");

    platform_printf("The PLDM payload fields are: \nMaximumTransferSize: %d, \nNumberOfComponents: %d," 
            "\nMaximumOutstandingTransferRequests: %d, \nPackageDataLength: %d," 
            "\nComponentImageSetVersionStringType: %d, \nComponentImageSetVersionStringLength: %d,"
            "\nComponentImageSetVersionString: %s\n\n",
                inReq.max_transfer_size, inReq.no_of_comp, 
                inReq.max_outstand_transfer_req, inReq.pkg_data_len, 
                inReq.comp_image_set_ver_str_type, inCompImgSetVerStr.length, inCompImgSetVerStr.ptr);

    status = device_manager_init(&device_mgr, 2, 0, DEVICE_MANAGER_PA_ROT_MODE, DEVICE_MANAGER_MASTER_BUS_ROLE, 
                1000, 1000, 1000, 1000, 1000, 1000, 5);
    CuAssertIntEquals(test, 0, status);
    device_mgr.entries->eid = 0xBB;
    device_mgr.entries->smbus_addr = 0xDD;

    platform_printf("Server initializes the MCTP interface.\n");
    status = mctp_interface_init(&mctp, &cmd_cerberus, &cmd_mctp, &cmd_spdm, &device_mgr);
    CuAssertIntEquals(test, 0, status);

    uint8_t mctpBuf[MCTP_BASE_PROTOCOL_MAX_PACKET_LEN];
    
    platform_printf("The server then generates the PLDM over MCTP binding message to send to the client.\n");
    status = mctp_interface_issue_request(&mctp, &channel, dest_addr, 0xCC, 
                                            pldmBuf, sizeof (pldmBuf), mctpBuf, sizeof (mctpBuf), 0);
    CuAssertIntEquals(test, 0, status);

    platform_printf("The MCTP message: MCTP header->{");
    platform_printbuf(mctpBuf, sizeof(struct mctp_base_protocol_transport_header));
    platform_printf("}, MCTP payload/PLDM message->{");
    platform_printbuf(&mctpBuf[sizeof(struct mctp_base_protocol_transport_header)], sizeof (struct pldm_msg_hdr) + sizeof (struct request_update_req) + compImgSetVerStrLen + 1);
    platform_printf("}, and Checksum->{");
    platform_printbuf(&mctpBuf[sizeof(struct mctp_base_protocol_transport_header) + sizeof (struct pldm_msg_hdr) + sizeof (struct request_update_req) + compImgSetVerStrLen + 1], 1);
    platform_printf("}.\n\n");

    platform_printf("Server then waits for a response from the client.\n");
    status = cmd_channel_receive_and_process(&channel, &mctp, 30000);

    CuAssertIntEquals(test, 0, status);

}

TEST_SUITE_START (pldm_fw_update);

TEST (pldm_fw_update_test_place_holder);
TEST (pldm_fw_update_test_request_update_req_good_response);

TEST_SUITE_END;