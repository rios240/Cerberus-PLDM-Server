#ifndef PLDM_FIRMWARE_UPDATE_CMDS_H_
#define PLDM_FIRMWARE_UPDATE_CMDS_H_

#include <stdint.h>
#include "base.h"
#include "utils.h"
#include "firmware_update.h"
#include "cmd_interface/cmd_interface.h"

int generate_error_packet(struct cmd_interface *intf, struct cmd_interface_msg *request,
		uint8_t error_code, uint32_t error_data, uint8_t cmd_set);

int request_update_req(uint8_t *pldm_buf, const char *compImgSetVerStrArr, uint8_t compImgSetVerStrLen);


int request_update_resp(struct cmd_interface *intf, struct cmd_interface_msg *response);







#endif /* PLDM_FIRMWARE_UPDATE_CMDS_H_ */