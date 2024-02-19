#ifndef PLDM_FWUP_COMMANDS_H_
#define PLDM_FWUP_COMMANDS_H_


#include <stdint.h>
#include "cmd_interface/cmd_interface.h"

int request_query_device_identifiers(uint8_t *request, size_t *payload_length);

int process_query_device_identifiers(struct cmd_interface *intf, struct cmd_interface_msg *response);

int request_get_firmware_parameters(uint8_t *request, size_t *payload_length);

int process_get_firmware_parameters(struct cmd_interface *intf, struct cmd_interface_msg *response);

int request_update(uint8_t *request, size_t *payload_length);

int process_request_update(struct cmd_interface *intf, struct cmd_interface_msg *response);

int process_and_respond_get_package_data(struct cmd_interface *intf, struct cmd_interface_msg *request);

int request_get_device_meta_data(uint8_t *request, size_t *payload_length);

int process_get_device_meta_data(struct cmd_interface *intf, struct cmd_interface_msg *response);

int pass_component_table(uint8_t *request, size_t *mctp_payload_length);

int process_pass_component_table_resp(struct cmd_interface *intf, struct cmd_interface_msg *response);

int update_component(uint8_t *request, size_t *mctp_payload_length);

int process_update_component_resp(struct cmd_interface *intf, struct cmd_interface_msg *response);



#endif /* PLDM_FWUP_COMMANDS_H_ */