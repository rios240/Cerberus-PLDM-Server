#include <string.h>
#include <unistd.h>
#include "pldm_firmware_update_cmds.h"
#include "mctp/mctp_interface.h"
#include "cmd_interface/cmd_interface.h"
#include "base.h"
#include "utils.h"
#include "firmware_update.h"

int generate_error_packet(struct cmd_interface *intf, struct cmd_interface_msg *request,
		uint8_t error_code, uint32_t error_data, uint8_t cmd_set) 
{
    return 0;
}

int request_update_req(uint8_t *pldm_buf, const char *compImgSetVerStrArr, uint8_t compImgSetVerStrLen) {
    pldm_buf[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *pldmMsg = (struct pldm_msg*)&pldm_buf[1];

    uint8_t instanceId = 1;

    struct variable_field inCompImgSetVerStr;
    inCompImgSetVerStr.ptr = (const uint8_t*) compImgSetVerStrArr;
    inCompImgSetVerStr.length = compImgSetVerStrLen;

    struct request_update_req inReq = {};
    inReq.max_transfer_size = 512;
    inReq.no_of_comp = 3;
    inReq.max_outstand_transfer_req = 2;
    inReq.pkg_data_len = 0x1234;
    inReq.comp_image_set_ver_str_type = PLDM_COMP_ASCII;
    inReq.comp_image_set_ver_str_len = compImgSetVerStrLen;

    return encode_request_update_req(instanceId, pldmMsg, sizeof (struct request_update_req) + inCompImgSetVerStr.length, 
                                        &inReq, &inCompImgSetVerStr);
}


int request_update_resp(struct cmd_interface *intf, struct cmd_interface_msg *response) {
    uint16_t fd_meta_data_len = 0;
    uint8_t fd_pkg_data = 0;
    uint8_t completion_code = 0;
    int status;

    struct pldm_msg *respMsg = (struct pldm_msg *)&response->data[1];

    status = decode_request_update_resp(respMsg, sizeof (struct pldm_request_update_resp), 
                                &completion_code, &fd_meta_data_len, &fd_pkg_data);

    response->length = 0;

    return status;

}