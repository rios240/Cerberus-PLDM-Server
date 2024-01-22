#ifndef PLDM_FWUP_COMMANDS_H_
#define PLDM_FWUP_COMMANDS_H_


#include <stdint.h>
#include "cmd_interface/cmd_interface.h"

int issue_query_device_identifiers(uint8_t *request, size_t *payload_length);

int process_query_device_identifiers(struct cmd_interface *intf, struct cmd_interface_msg *response);

int issue_get_firmware_parameters(uint8_t *request, size_t *payload_length);

int process_get_firmware_parameters(struct cmd_interface *intf, struct cmd_interface_msg *response);

int issue_request_update(uint8_t *request, size_t *payload_length);

int process_request_update(struct cmd_interface *intf, struct cmd_interface_msg *response);

int issue_pass_component_table(uint8_t *request, size_t *payload_length);

int process_pass_component_table(struct cmd_interface *intf, struct cmd_interface_msg *response);




#endif /* PLDM_FWUP_COMMANDS_H_ */