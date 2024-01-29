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
#include "pldm_fwup_mctp.h"


struct pldm_fwup_interface *get_fwup_interface()
{
    static struct pldm_fwup_interface fwup;
    return &fwup;
}

void generate_random_data(uint8_t *data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        data[i] = (uint8_t)rand();
    }
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

    device_mgr->entries->eid = 0xDE;
    device_mgr->entries->smbus_addr = 0xDE;

    status = mctp_interface_init(mctp, cmd_cerberus, cmd_mctp, cmd_spdm, device_mgr);

    mctp->cmd_cerberus->generate_error_packet = generate_error_packet;
    
    struct pldm_fwup_multipart_transfer multipart_transfer;
    multipart_transfer.transfer_flag = PLDM_START;

    fwup->multipart_transfer = multipart_transfer;
    fwup->max_transfer_size = 10;

    fwup->package_data_size = 50;
    fwup->package_data = (uint8_t *)malloc(fwup->package_data_size * sizeof (uint8_t));
    generate_random_data(fwup->package_data, fwup->package_data_size);

    return status;
}



void clean_up_and_reset_firmware_update(struct mctp_interface *mctp, struct pldm_fwup_interface *fwup)
{
    mctp_interface_reset_message_processing(mctp);

    fwup->multipart_transfer.last_data_transfer_handle = 0;
    fwup->multipart_transfer.transfer_flag = PLDM_START;

    fwup->max_transfer_size = 10;
    fwup->package_data_size = 0;
    free(fwup->package_data);
}