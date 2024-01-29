#include <stdint.h>
#include <stdlib.h>
#include "pldm_fwup_mctp.h"

uint8_t *realloc_buf(uint8_t *ptr, size_t length) {
    uint8_t *temp = realloc(ptr, length * sizeof (uint8_t));
    return temp;
}

int generate_and_send_pldm_over_mctp(struct mctp_interface *mctp, struct cmd_channel *cmd_channel, 
                                int (*generate_pldm)(uint8_t *, size_t *))
{
    uint8_t *pldm_payload = (uint8_t *)malloc(PLDM_MAX_PAYLOAD_LENGTH * sizeof (uint8_t));
    size_t payload_length = 0;

    int status = generate_pldm(pldm_payload, &payload_length);
    if (status != 0) {
        free(pldm_payload);
        return status;
    }
    pldm_payload = realloc_buf(pldm_payload, payload_length);
    uint8_t mctp_buf[MCTP_BASE_PROTOCOL_MAX_MESSAGE_LEN];
    status = mctp_interface_issue_request(mctp, cmd_channel, DEST_ADDR, DEST_EID, pldm_payload, payload_length,
                                    mctp_buf,  MCTP_BASE_PROTOCOL_MAX_MESSAGE_LEN, 0);

    free(pldm_payload);
    return status;
    
}


int process_and_receive_pldm_over_mctp(struct mctp_interface *mctp, struct cmd_channel *cmd_channel, 
                                int (*process_pldm)(struct cmd_interface *, struct cmd_interface_msg *))
{
    mctp->cmd_mctp->process_response = process_pldm;
    int status = cmd_channel_receive_and_process(cmd_channel, mctp, MS_TIMEOUT);
    
    return status;
    
}
