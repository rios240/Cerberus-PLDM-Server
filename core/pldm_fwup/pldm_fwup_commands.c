#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "pldm_fwup_commands.h"
#include "firmware_update.h"


int issue_query_device_identifiers(uint8_t *request, size_t *payload_length)
{

    uint8_t instance_id = 1;

    *payload_length = sizeof (struct pldm_msg)
                    + 1;

    request[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *reqMsg = (struct pldm_msg *)(request + 1);

    int status = encode_query_device_identifiers_req(instance_id, reqMsg, 0);

    return status;

}

int process_query_device_identifiers(struct cmd_interface *intf, struct cmd_interface_msg *response)
{
    struct pldm_msg *respMsg = (struct pldm_msg *)(&response->data[1]);
    
    uint8_t completion_code;
    uint32_t device_identifiers_len;
    uint8_t descriptor_count;
    uint8_t *descriptor_data;

    const size_t payload_length = response->length - sizeof (struct pldm_msg_hdr) - 1;
    int status = decode_query_device_identifiers_resp(respMsg, payload_length, 
                                            &completion_code, &device_identifiers_len, &descriptor_count, &descriptor_data);
    
    response->length = 0;

    return status;
}


int issue_get_firmware_parameters(uint8_t *request, size_t *payload_length)
{
    uint8_t instance_id = 1;

    *payload_length = sizeof (struct pldm_msg)
                    + 1;

    request[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *reqMsg = (struct pldm_msg *)(request + 1);

    int status = encode_get_firmware_parameters_req(instance_id, reqMsg, 0);

    return status;
}

int process_get_firmware_parameters(struct cmd_interface *intf, struct cmd_interface_msg *response)
{
    struct pldm_msg *respMsg = (struct pldm_msg *)(&response->data[1]);

    struct get_firmware_parameters_resp resp_data;
    struct variable_field active_comp_image_set_ver_str;
    struct variable_field pending_comp_image_set_ver_str;

    const size_t payload_length = response->length - sizeof (struct pldm_msg_hdr) - 1;
    int status = decode_get_firmware_parameters_comp_img_set_resp(respMsg, payload_length, &resp_data,
                                                        &active_comp_image_set_ver_str, &pending_comp_image_set_ver_str);
    
    struct component_parameter_table comp_parameter_table;
    struct variable_field active_comp_ver_str;
    struct variable_field pending_comp_ver_str;

    const uint8_t *comp_parameter_table_data = respMsg->payload + sizeof (struct get_firmware_parameters_resp) + active_comp_image_set_ver_str.length
                                            + pending_comp_image_set_ver_str.length;
    const size_t comp_parameter_table_length = response->length - sizeof (struct get_firmware_parameters_resp) - active_comp_image_set_ver_str.length
                                            - pending_comp_image_set_ver_str.length - 1;

    status = decode_get_firmware_parameters_comp_resp(comp_parameter_table_data, comp_parameter_table_length, &comp_parameter_table, 
                                                &active_comp_ver_str, &pending_comp_ver_str);
    
    response->length = 0;

    return status;


}


int issue_request_update(uint8_t *request, size_t *payload_length)
{
    uint8_t instance_id = 1;
    const char* comp_image_set_ver_str_arr = "cerberus_v2.0";

    struct request_update_req req_data;
    req_data.max_transfer_size = 60;
    req_data.no_of_comp = 1;
    req_data.max_outstand_transfer_req = 1;
    req_data.comp_image_set_ver_str_type = PLDM_COMP_ASCII;
    req_data.comp_image_set_ver_str_len = strlen(comp_image_set_ver_str_arr);

    struct variable_field comp_image_set_ver_str;
    comp_image_set_ver_str.ptr = (const uint8_t *)comp_image_set_ver_str_arr;
    comp_image_set_ver_str.length = req_data.comp_image_set_ver_str_len;

    *payload_length = sizeof (struct pldm_msg_hdr)
                    + sizeof (struct request_update_req)
                    + comp_image_set_ver_str.length
                    + 1;

    const size_t pldm_payload_length = sizeof (struct request_update_req) + comp_image_set_ver_str.length;

    request[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *reqMsg = (struct pldm_msg *)(request + 1);

    int status = encode_request_update_req(instance_id, reqMsg, pldm_payload_length, &req_data, &comp_image_set_ver_str);

    return status;
}


int process_request_update(struct cmd_interface *intf, struct cmd_interface_msg *response)
{
    struct pldm_msg *respMsg = (struct pldm_msg *)(&response->data[1]);

    uint8_t completion_code;
    uint16_t fd_meta_data_len;
    uint8_t fd_pkg_data;

    const size_t payload_length = response->length - sizeof (struct pldm_msg_hdr) - 1;
    int status = decode_request_update_resp(respMsg, payload_length, &completion_code, &fd_meta_data_len, &fd_pkg_data);

    response->length = 0;

    return status;

}

int issue_pass_component_table(uint8_t *request, size_t *payload_length)
{
    uint8_t instance_id = 1;

    const char *comp_ver_str_arr = "BIOS_v2.0";

    struct pass_component_table_req req_data;
    req_data.transfer_flag = 0x5;
    req_data.comp_classification = COMP_FIRMWARE_OR_BIOS;
    req_data.comp_identifier = 0xDEAD;
    req_data.comp_classification_index = 0xDE;
    req_data.comp_comparison_stamp = 0xDEADBEEF;
    req_data.comp_ver_str_type = PLDM_COMP_ASCII;
    req_data.comp_ver_str_len = strlen(comp_ver_str_arr);

    struct variable_field comp_ver_str;
    comp_ver_str.ptr = (const uint8_t *)comp_ver_str_arr;
    comp_ver_str.length = req_data.comp_ver_str_len;

    *payload_length = sizeof (struct pldm_msg_hdr)
                    + sizeof (struct pass_component_table_req)
                    + comp_ver_str.length
                    + 1;

    const size_t pldm_payload_length = sizeof (struct pass_component_table_req) + comp_ver_str.length;

    request[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *reqMsg = (struct pldm_msg *)(request + 1);

    int status = encode_pass_component_table_req(instance_id, reqMsg, pldm_payload_length, &req_data, &comp_ver_str);

    return status;
}


int process_pass_component_table(struct cmd_interface *intf, struct cmd_interface_msg *response)
{
    struct pldm_msg *respMsg = (struct pldm_msg *)(&response->data[1]);

    uint8_t completion_code;
    uint8_t comp_resp;
    uint8_t comp_resp_code;

    const size_t payload_length = response->length - sizeof (struct pldm_msg_hdr) - 1;
    int status = decode_pass_component_table_resp(respMsg, payload_length, &completion_code, &comp_resp, &comp_resp_code);

    response->length = 0;

    return status;
}