#ifndef PLDM_FIRMWARE_CMD_CHANNEL_H_
#define PLDM_FIRMWARE_CMD_CHANNEL_H_

#include <stdint.h>
#include "mctp/mctp_interface.h"
#include "cmd_interface/cmd_channel.h"


int send_packet(struct cmd_channel *channel, struct cmd_packet *packet);
int receive_packet(struct cmd_channel *channel, struct cmd_packet *packet, int ms_timeout);



#endif /* PLDM_FIRMWARE_CMD_CHANNEL_H_ */