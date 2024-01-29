#ifndef PLDM_FWUP_INTERFACE_H_
#define PLDM_FWUP_INTERFACE_H_

#include "mctp/mctp_interface.h"
#include "cmd_interface/cmd_channel.h"

enum multipart_transfer_status {
    MPT_START,
    MPT_IN_PROGRESS,
    MPT_END
};

struct pldm_fwup_multipart_transfer {
    uint8_t transfer_flag;
    uint32_t last_data_transfer_handle;
};

struct pldm_fwup_interface {
    uint8_t completion_code;
    struct pldm_fwup_multipart_transfer multipart_transfer;
    uint32_t max_transfer_size;
    uint32_t package_data_size;
    uint8_t *package_data;

};

struct pldm_fwup_interface *get_fwup_interface();


int initialize_firmware_update(struct mctp_interface *mctp, struct cmd_channel *cmd_channel, 
                        struct cmd_interface *cmd_mctp, 
                        struct cmd_interface *cmd_spdm, struct cmd_interface *cmd_cerberus,
                        struct device_manager *device_mgr, struct pldm_fwup_interface *fwup);

int perform_firmware_update(struct mctp_interface *mctp, struct cmd_channel *cmd_channel);

void clean_up_and_reset_firmware_update(struct mctp_interface *mctp, struct pldm_fwup_interface *fwup);



#endif /* PLDM_FWUP_INTERFACE_H_ */