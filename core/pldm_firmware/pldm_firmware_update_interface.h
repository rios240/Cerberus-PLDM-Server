#ifndef PLDM_FIRMWARE_UPDATE_INTERFACE_H_
#define PLDM_FIRMWARE_UPDATE_INTERFACE_H_

#include <stdint.h>
#include "mctp/mctp_interface.h"
#include "cmd_interface/cmd_channel.h"


int pldm_firmware_update_init(struct mctp_interface *mctp, struct cmd_channel *channel, struct cmd_interface *cmd_mctp,
                                struct cmd_interface *cmd_spdm, struct cmd_interface *cmd_cerberus,
                                struct device_manager *device_mgr, uint8_t device_eid, uint8_t device_smbus_addr);

int pldm_firmware_update_run(struct mctp_interface *mctp, struct cmd_channel *channel, 
                                int ms_timeout, uint8_t dest_addr, uint8_t dest_eid);








#endif /* PLDM_FIRMWARE_UPDATE_INTERFACE_H_ */