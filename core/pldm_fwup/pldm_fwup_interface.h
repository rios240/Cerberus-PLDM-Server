#ifndef PLDM_FWUP_INTERFACE_H_
#define PLDM_FWUP_INTERFACE_H_

#include "mctp/mctp_interface.h"
#include "cmd_interface/cmd_channel.h"


#define PLDM_MAX_PAYLOAD_LENGTH 512
#define MS_TIMEOUT 60000

#define DEST_ADDR 0xDA
#define DEST_EID 0xDA


int initialize_firmware_update(struct mctp_interface *mctp, struct cmd_channel *cmd_channel, 
                        struct cmd_interface *cmd_mctp, 
                        struct cmd_interface *cmd_spdm, struct cmd_interface *cmd_cerberus,
                        struct device_manager *device_mgr);

int perform_firmware_update(struct mctp_interface *mctp, struct cmd_channel *cmd_channel);





#endif /* PLDM_FWUP_INTERFACE_H_ */