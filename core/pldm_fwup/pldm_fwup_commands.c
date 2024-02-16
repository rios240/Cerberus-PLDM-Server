#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "pldm_fwup_commands.h"
#include "firmware_update.h"
#include "pldm_fwup_interface.h"


int request_query_device_identifiers(uint8_t *request, size_t *payload_length)
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
    struct pldm_fwup_interface *fwup = get_fwup_interface();

    struct pldm_msg *respMsg = (struct pldm_msg *)(&response->data[1]);
    
    uint8_t completion_code;
    uint32_t device_identifiers_len;
    uint8_t descriptor_count;
    uint8_t *descriptor_data;

    const size_t payload_length = response->length - sizeof (struct pldm_msg_hdr) - 1;
    int status = decode_query_device_identifiers_resp(respMsg, payload_length, 
                                            &completion_code, &device_identifiers_len, &descriptor_count, &descriptor_data);
    
    response->length = 0;

    fwup->completion_code = completion_code;
    return status;
}


int request_get_firmware_parameters(uint8_t *request, size_t *payload_length)
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
    struct pldm_fwup_interface *fwup = get_fwup_interface();

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

    fwup->completion_code = resp_data.completion_code;
    return status;


}


int request_update(uint8_t *request, size_t *payload_length)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();

    uint8_t instance_id = 1;
    const char* comp_image_set_ver_str_arr = "cerberus_v2.0";

    struct request_update_req req_data;
    req_data.max_transfer_size = 60;
    req_data.no_of_comp = 1;
    req_data.max_outstand_transfer_req = 1;
    req_data.pkg_data_len = fwup->package_data_size;
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
    struct pldm_fwup_interface *fwup = get_fwup_interface();
    struct pldm_msg *respMsg = (struct pldm_msg *)(&response->data[1]);

    uint8_t completion_code;
    uint16_t fd_meta_data_len;
    uint8_t fd_pkg_data;

    const size_t payload_length = response->length - sizeof (struct pldm_msg_hdr) - 1;
    int status = decode_request_update_resp(respMsg, payload_length, &completion_code, &fd_meta_data_len, &fd_pkg_data);

    fwup->meta_data_size = fd_meta_data_len;
    fwup->meta_data = realloc_buf(fwup->meta_data, fd_meta_data_len);
    fwup->expect_package_data_command = fd_pkg_data;

    response->length = 0;

    fwup->completion_code = completion_code;
    return status;

}


int process_and_respond_get_package_data(struct cmd_interface *intf, struct cmd_interface_msg *request)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();
    static uint32_t portion_size = 10;

    size_t payload_length = request->length - sizeof (struct pldm_msg_hdr) - 1;
    struct pldm_msg *reqMsg = (struct pldm_msg *)(&request->data[1]);

    uint32_t data_transfer_handle;
    uint8_t transfer_operation_flag;

    int status = decode_get_pacakge_data_req(reqMsg, (const size_t) payload_length, &data_transfer_handle, &transfer_operation_flag);
    if (status != 0) {
        return status;
    }

    struct get_fd_data_resp resp_data = { 0 };
    resp_data.completion_code = PLDM_SUCCESS;
    struct variable_field portion_of_pkg_data;
    portion_of_pkg_data.length = portion_size;

    if ((transfer_operation_flag == PLDM_GET_FIRSTPART && fwup->multipart_transfer.transfer_in_progress == 1) ||
        (transfer_operation_flag == PLDM_GET_NEXTPART && fwup->multipart_transfer.transfer_in_progress == 0)) {
        resp_data.completion_code = INVALID_TRANSFER_OPERATION_FLAG;
    }
    if (transfer_operation_flag != PLDM_GET_FIRSTPART && data_transfer_handle != fwup->multipart_transfer.last_transfer_handle) {
        resp_data.completion_code = INVALID_DATA_TRANSFER_HANDLE;
    }
    if (fwup->package_data_size == 0) {
        resp_data.completion_code = NO_PACKAGE_DATA;
    }
  
    if (transfer_operation_flag == PLDM_GET_FIRSTPART && fwup->multipart_transfer.transfer_in_progress == 0) {
        if (fwup->package_data_size <= portion_size) {
            resp_data.transfer_flag = PLDM_START_AND_END;
        } else {
            resp_data.transfer_flag = PLDM_START;
            fwup->multipart_transfer.transfer_in_progress = 1;
        }
    }
    if (transfer_operation_flag == PLDM_GET_NEXTPART && fwup->multipart_transfer.transfer_in_progress == 1) {
        resp_data.transfer_flag = PLDM_MIDDLE;
    }
    if (fwup->multipart_transfer.last_transfer_handle + portion_size == fwup->package_data_size) {
        resp_data.transfer_flag = PLDM_END;
        fwup->multipart_transfer.transfer_in_progress = 0;
    }

    portion_of_pkg_data.ptr = fwup->package_data + fwup->multipart_transfer.last_transfer_handle;
    fwup->multipart_transfer.last_transfer_handle += portion_size;
    resp_data.next_data_transfer_handle = fwup->multipart_transfer.last_transfer_handle;

    uint8_t instance_id = 1;
    payload_length = sizeof (struct pldm_msg_hdr) + sizeof (struct get_fd_data_resp) + portion_of_pkg_data.length + 1;

    request->length = payload_length;
    request->data[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *respMsg = (struct pldm_msg *)(&request->data[1]);

    status = encode_get_package_data_resp(instance_id, payload_length, respMsg, &resp_data, &portion_of_pkg_data);

    fwup->completion_code = resp_data.completion_code;

    print_bytes((uint8_t *)portion_of_pkg_data.ptr, portion_of_pkg_data.length);

    return status;
    

}

/*
int request_get_device_meta_data(uint8_t *request, size_t *payload_length)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();

    uint32_t data_transfer_handle;
    uint8_t transfer_operation_flag;

    if (fwup->multipart_transfer.transfer_in_progress == 0) {
        transfer_operation_flag = PLDM_GET_FIRSTPART;
    } else {
        transfer_operation_flag = PLDM_GET_NEXTPART;
    }

    data_transfer_handle = fwup->multipart_transfer.last_transfer_handle;

    uint8_t instance_id = 1;
    *payload_length = sizeof (struct pldm_msg_hdr)
                + sizeof (struct get_device_meta_data_req)
                + 1;

    const size_t pldm_payload_length = sizeof (struct get_device_meta_data_req);
    
    request[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *reqMsg = (struct pldm_msg *)(request + 1);
    int status = encode_get_device_meta_data_req(instance_id, reqMsg, pldm_payload_length, data_transfer_handle, transfer_operation_flag);

    return status;
}

int process_get_device_meta_data(struct cmd_interface *intf, struct cmd_interface_msg *response)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();

    size_t payload_length = response->length - sizeof (struct pldm_msg_hdr) - 1;
    struct pldm_msg *respMsg = (struct pldm_msg *)(&response->data[1]);
    
    uint8_t completion_code;
    uint32_t next_data_transfer_handle;
    uint8_t transfer_flag;
    struct variable_field portion_of_device_meta_data;

    int status = decode_get_device_meta_data_resp(respMsg, (const size_t)payload_length, &completion_code, 
                    &next_data_transfer_handle, &transfer_flag, &portion_of_device_meta_data);
    
    if (transfer_flag == PLDM_END || transfer_flag == PLDM_START_AND_END) {
        fwup->multipart_transfer.transfer_in_progress = 0;
    } else {
        fwup->multipart_transfer.transfer_in_progress = 1;
    }
    
    if (transfer_flag == PLDM_START) {
        memcpy(fwup->meta_data, portion_of_device_meta_data.ptr, portion_of_device_meta_data.length);
    } else {
        memcpy(fwup->meta_data + fwup->multipart_transfer.last_transfer_handle, portion_of_device_meta_data.ptr, 
            portion_of_device_meta_data.length);
    }
    fwup->multipart_transfer.last_transfer_handle = next_data_transfer_handle;

    response->length = 0;
    fwup->completion_code = completion_code;

    return status;
    
}
*/


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