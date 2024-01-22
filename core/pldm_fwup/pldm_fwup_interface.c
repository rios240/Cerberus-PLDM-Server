#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "pldm_fwup_commands.h"
#include "firmware_update.h"
#include "pldm_fwup_cmd_channel.h"
#include "cmd_interface/device_manager.h"
#include "mctp/mctp_interface.h"
#include "pldm_fwup_interface.h"

int initialize_firmware_update(struct mctp_interface *mctp, struct cmd_channel *cmd_channel, 
                        struct cmd_interface *cmd_mctp, 
                        struct cmd_interface *cmd_spdm, struct cmd_interface *cmd_cerberus,
                        struct device_manager *device_mgr)
{
    cmd_channel->send_packet = send_packet;
    cmd_channel->receive_packet = receive_packet;
    cmd_channel->id = 1;

    int status = device_manager_init(device_mgr, 2, 0, DEVICE_MANAGER_PA_ROT_MODE, DEVICE_MANAGER_SLAVE_BUS_ROLE, 
                1000, 1000, 1000, 1000, 1000, 1000, 5);

    if (status != 0) {
        return status;
    }

    device_mgr->entries->eid = 0xDE;
    device_mgr->entries->smbus_addr = 0xDE;

    status = mctp_interface_init(mctp, cmd_cerberus, cmd_mctp, cmd_spdm, device_mgr);

    mctp->cmd_cerberus->generate_error_packet = generate_error_packet;

    return status;
}


int perform_firmware_update(struct mctp_interface *mctp, struct cmd_channel *cmd_channel)
{
    uint8_t *pldm_buf = (uint8_t *)malloc(PLDM_MAX_PAYLOAD_LENGTH + sizeof (uint8_t));
    size_t payload_length = 0;

    int status = issue_query_device_identifiers(pldm_buf, &payload_length);
    if (status != 0) {
        return status;
    }
    uint8_t pldm_request[payload_length];
    memcpy(pldm_request, pldm_buf, payload_length);
    free(pldm_buf);

    uint8_t mctp_buf[MCTP_BASE_PROTOCOL_MAX_MESSAGE_LEN];

    status = mctp_interface_issue_request(mctp, cmd_channel, DEST_ADDR, DEST_EID, pldm_request, payload_length,
                                    mctp_buf,  MCTP_BASE_PROTOCOL_MAX_MESSAGE_LEN, 0);
    if (status != 0) {
        return status;
    }

    mctp->cmd_mctp->process_response = process_query_device_identifiers;

    status = cmd_channel_receive_and_process(cmd_channel, mctp, MS_TIMEOUT);
    return status;
}