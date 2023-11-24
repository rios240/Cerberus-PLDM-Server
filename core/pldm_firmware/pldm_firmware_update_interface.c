#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "mctp/mctp_interface.h"
#include "pldm_firmware_update_interface.h"
#include "pldm_firmware_cmd_channel.h"
#include "pldm_firmware_update_cmds.h"
#include "platform_io.h"



int pldm_firmware_update_init(struct mctp_interface *mctp, struct cmd_channel *channel, struct device_manager *device_mgr,
                                uint8_t device_eid, uint8_t device_smbus_addr) 
{
    int status;
    struct cmd_interface cmd_cerberus;
    struct cmd_interface cmd_mctp;
    struct cmd_interface cmd_spdm;

    channel->send_packet = send_packet;
    channel->receive_packet = receive_packet;
    channel->id = 1;

    status = device_manager_init(device_mgr, 2, 0, DEVICE_MANAGER_PA_ROT_MODE, DEVICE_MANAGER_SLAVE_BUS_ROLE, 
                1000, 1000, 1000, 1000, 1000, 1000, 5);

    if (status != 0) {
        return status;
    }

    device_mgr->entries->eid = device_eid;
    device_mgr->entries->smbus_addr = device_smbus_addr;

    status = mctp_interface_init(mctp, &cmd_cerberus, &cmd_mctp, &cmd_spdm, device_mgr);

    return status;

}

int pldm_firmware_update_run(struct mctp_interface *mctp, struct cmd_channel *channel, 
                                int ms_timeout, uint8_t dest_addr, uint8_t dest_eid) 
{

    const char *compImgSetVerStrArr = "flash_update_v1.0";
    uint8_t compImgSetVerStrLen = strlen(compImgSetVerStrArr);
    uint8_t pldm_buf[sizeof (struct pldm_msg_hdr) + sizeof (struct request_update_req) + compImgSetVerStrLen + 1];
    
    int status = request_update_req(pldm_buf, compImgSetVerStrArr, compImgSetVerStrLen); 
    if (status != PLDM_SUCCESS) {
        return status;
    }
    
    uint8_t mctp_buf[MCTP_BASE_PROTOCOL_MAX_PACKET_LEN];
    mctp->msg_type = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    
    status = mctp_interface_issue_request(mctp, channel, dest_addr, dest_eid, 
                                            pldm_buf, sizeof (pldm_buf), mctp_buf, sizeof (mctp_buf), 0);
    if (status != 0) {
        return status;
    }

    mctp->cmd_mctp->process_response = request_update_resp;

    status = cmd_channel_receive_and_process(channel, mctp, ms_timeout);
    return status;
}
