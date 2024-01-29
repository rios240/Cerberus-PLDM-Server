#ifndef PLDM_FWUP_MCTP_H_
#define PLDM_FWUP_MCTP_H_


#include "mctp/mctp_interface.h"

#define PLDM_MAX_PAYLOAD_LENGTH 512

#define MS_TIMEOUT 60000

#define DEST_ADDR 0xDA
#define DEST_EID 0xDA


int generate_and_send_pldm_over_mctp(struct mctp_interface *mctp, struct cmd_channel *cmd_channel, 
                                int (*generate_pldm)(uint8_t *, size_t *));

int process_and_receive_pldm_over_mctp(struct mctp_interface *mctp, struct cmd_channel *cmd_channel,
                                int (*process_pldm)(struct cmd_interface *, struct cmd_interface_msg *));






#endif /* PLDM_FWUP_MCTP_H_ */