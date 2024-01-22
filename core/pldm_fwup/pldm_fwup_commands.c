#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "pldm_fwup_commands.h"
#include "firmware_update.h"


/*
struct firmware_update_package pldm_fup;

int initialize_firmware_update_package() {

    const char *comp_image_set_version_str = "cerberus_v2.0";
    uint8_t fw_device_pkg_data[10];
    for (int i = 0; i < 10; i++) {
        fw_device_pkg_data[i] = rand() % 256;
    }

    struct descriptor_tlv descriptors[2];

    int iana_number = 135;
    descriptors[0].descriptor_type = 0;
    descriptors[0].descriptor_length = 2;
    memcpy(descriptors[0].descriptor_data, &iana_number, descriptors[0].descriptor_length);


    const char *uuid_string = "8d759454a78111eea5060242ac120002";
    descriptors[1].descriptor_type = 2;
    descriptors[1].descriptor_length = 16;
    memcpy(descriptors[1].descriptor_data, &uuid_string, descriptors[1].descriptor_length);



    struct firmware_device_id_record record;
    record.descriptor_count = 2;
    record.device_update_option_flags.value = 1;
    record.comp_image_set_version_string_type = 1;
    record.comp_image_set_version_string_length = strlen(comp_image_set_version_str);
    record.fw_device_pkg_data_length = 10;
    record.applicable_components.byte = 0x00;
    record.applicable_components.bits.bit0 = 1;
    record.applicable_components.bits.bit1 = 1;
    record.comp_image_set_version_str = (const uint8_t *)comp_image_set_version_str;
    record.record_descriptors = descriptors;
    record.device_package_data = fw_device_pkg_data;

    const char *comp_version_string = "bios_v2.0";

    struct component_image_information comp_info[1];
    comp_info[0].comp_classification = COMP_FIRMWARE_OR_BIOS;
    comp_info[0].comp_identifier = rand() % 65536;
    comp_info[0].comp_options.value = 0;
    comp_info[0].comp_options.bits.bit0 = 1;
    comp_info[0].comp_comparison_stamp = 0xFFFFFFF;
    comp_info[0].requested_comp_activation_method.value = 0;
    comp_info[0].requested_comp_activation_method.bits.bit0 = 0x1b;
    

    return 0;

}

*/

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

    int status = decode_get_firmware_parameters_comp_img_set_resp(respMsg, response->length, &resp_data,
                                                        &active_comp_image_set_ver_str, &pending_comp_image_set_ver_str);
    
    struct component_parameter_table comp_parameter_table;
    struct variable_field active_comp_ver_str;
    struct variable_field pending_comp_ver_str;

    const uint8_t *comp_parameter_table_data = respMsg->payload + sizeof (struct get_firmware_parameters_resp) + active_comp_image_set_ver_str.length
                                            + pending_comp_image_set_ver_str.length;
    const size_t payload_length = response->length - sizeof (struct get_firmware_parameters_resp) - active_comp_image_set_ver_str.length
                                            - pending_comp_image_set_ver_str.length - 1;

    status = decode_get_firmware_parameters_comp_resp(comp_parameter_table_data, payload_length, &comp_parameter_table, 
                                                &active_comp_ver_str, &pending_comp_ver_str);
    
    response->length = 0;

    return status;


}