#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "pldm_fwup_commands.h"
#include "firmware_update.h"
#include "pldm_fwup_cmd_channel.h"
#include "cmd_interface/device_manager.h"
#include "mctp/mctp_interface.h"
#include "pldm_fwup_interface.h"


struct pldm_fwup_interface *get_fwup_interface()
{
    static struct pldm_fwup_interface fwup;
    return &fwup;
}

uint8_t *realloc_buf(uint8_t *ptr, size_t length) {
    uint8_t *temp = realloc(ptr, length * sizeof (uint8_t));
    return temp;
}

void generate_random_data(uint8_t *data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        data[i] = (uint8_t)rand();
    }
}

void print_bytes(uint8_t *bytes, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", bytes[i]);
    }
    printf("\n");
}


int initialize_firmware_update(struct mctp_interface *mctp, struct cmd_channel *cmd_channel, 
                        struct cmd_interface *cmd_mctp, 
                        struct cmd_interface *cmd_spdm, struct cmd_interface *cmd_cerberus,
                        struct device_manager *device_mgr, struct pldm_fwup_interface *fwup)
{
    cmd_channel->send_packet = send_packet;
    cmd_channel->receive_packet = receive_packet;
    cmd_channel->id = 1;

    int status = device_manager_init(device_mgr, 2, 0, DEVICE_MANAGER_PA_ROT_MODE, DEVICE_MANAGER_SLAVE_BUS_ROLE, 
                1000, 1000, 1000, 1000, 1000, 1000, 5);

    if (status != 0) {
        return status;
    }

    device_mgr->entries->eid = SRC_EID;
    device_mgr->entries->smbus_addr = SRC_ADDR;

    status = mctp_interface_init(mctp, cmd_cerberus, cmd_mctp, cmd_spdm, device_mgr);

    mctp->cmd_cerberus->generate_error_packet = generate_error_packet;
    
    struct pldm_fwup_multipart_transfer multipart_transfer = { 0 };

    fwup->multipart_transfer = multipart_transfer;

    fwup->package_data_size = 50;
    fwup->package_data = (uint8_t *)malloc(fwup->package_data_size * sizeof (uint8_t));
    generate_random_data(fwup->package_data, fwup->package_data_size);

    fwup->meta_data_size = 0;
    fwup->meta_data = (uint8_t *)malloc(sizeof(uint8_t));

    fwup->comp_image_size = 160;
    fwup->comp_image = (uint8_t *)malloc(fwup->comp_image_size * sizeof (uint8_t));
    generate_random_data(fwup->comp_image, fwup->comp_image_size);

    return status;
}


int generate_and_send_pldm_over_mctp(struct mctp_interface *mctp, struct cmd_channel *cmd_channel,
                                int (*generate_pldm)(uint8_t *, size_t *))
{
    uint8_t *pldm_payload = (uint8_t *)malloc(PLDM_MAX_PAYLOAD_LENGTH * sizeof (uint8_t));
    size_t payload_length = 0;

    int status = generate_pldm(pldm_payload, &payload_length);
    if (status != 0) {
        free(pldm_payload);
        return status;
    }
    pldm_payload = realloc_buf(pldm_payload, payload_length);
    uint8_t mctp_buf[MCTP_BASE_PROTOCOL_MAX_MESSAGE_LEN];
    status = mctp_interface_issue_request(mctp, cmd_channel, DEST_ADDR, DEST_EID, pldm_payload, payload_length,
                                    mctp_buf,  MCTP_BASE_PROTOCOL_MAX_MESSAGE_LEN, 0);

    free(pldm_payload);
    return status;
    
}


int process_and_receive_pldm_over_mctp(struct mctp_interface *mctp, struct cmd_channel *cmd_channel,
                                int (*process_pldm)(struct cmd_interface *, struct cmd_interface_msg *))
{
    mctp->cmd_mctp->process_request = process_pldm;
    mctp->cmd_mctp->process_response = process_pldm;
    int status = cmd_channel_receive_and_process(cmd_channel, mctp, MS_TIMEOUT);
    
    return status;
    
}



void clean_up_and_reset_firmware_update(struct mctp_interface *mctp, struct pldm_fwup_interface *fwup)
{
    mctp_interface_reset_message_processing(mctp);

    fwup->multipart_transfer.last_transfer_handle = 0;
    fwup->multipart_transfer.transfer_in_progress = 0;

    fwup->package_data_size = 0;
    free(fwup->package_data);

    fwup->meta_data_size = 0;
    free(fwup->meta_data);

    fwup->comp_image_size = 0;
    free(fwup->comp_image);
}