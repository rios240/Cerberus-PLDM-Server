// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef CMD_CHANNEL_H_
#define CMD_CHANNEL_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "status/rot_status.h"
#include "platform_api.h"
#include "mctp/mctp_base_protocol.h"


/**
 * The maximum size for a single packet sent or received through a command channel.
 */
#define	CMD_MAX_PACKET_SIZE				MCTP_BASE_PROTOCOL_MAX_PACKET_LEN

/**
 * Valid states for a packet.
 */
enum {
	CMD_VALID_PACKET = 0,				/**< Valid command packet. */
	CMD_OVERFLOW_PACKET,				/**< This packet is part of an overflow condition. */
	CMD_NACK_PACKET,					/**< NACK should be sent, disregard packet data. */
	CMD_NO_RESPONSE,					/**< No response needed. */
	CMD_RX_ERROR,						/**< There was a channel error while receiving the packet data. */
};



/**
 * Information for a single command packet.
 */
struct cmd_packet {
	uint8_t data[CMD_MAX_PACKET_SIZE];	/**< Buffer for packet data. */
	size_t pkt_size;					/**< Total size of the packet data. */
	uint8_t dest_addr;					/**< The destination address for the packet.  This is
	 	 	 	 	 	 	 	 	 	 	assumed to be a 7-bit address compatible with I2C. */
	uint8_t state;						/**< The packet state. */
	platform_clock pkt_timeout;			/**< Time at which processing for the packet must be completed. */
	bool timeout_valid;					/**< Flag indicating if a packet timeout has been set. */
};

/**
 * Information for a single command message.
 */
struct cmd_message {
	uint8_t *data;						/**< Buffer for the message data. */
	size_t msg_size;					/**< Total size of the message data. */
	size_t pkt_size;					/**< Size of each packet in the message. */
	uint8_t dest_addr;					/**< The destination address for the message. */
};


struct mctp_interface;

/**
 * Defines the interface for a communication channel to send and receive command packets.
 *
 * Channels must be implemented to allow for simultaneous receive and send calls.  This will
 * enable senders to be transmitting messages while the channel is blocked waiting to receive
 * packets.
 */
struct cmd_channel {
	/**
	 * Receive a command packet from a communication channel.  This call will block until a packet
	 * has been received or the timeout has expired.
	 *
	 * @param channel The channel to receive a packet from.
	 * @param packet Output for the packet data being received.
	 * @param ms_timeout The amount of time to wait for a received packet, in milliseconds.  A
	 * negative value will wait forever, and a value of 0 will return immediately.
	 *
	 * @return 0 if a packet was successfully received or an error code.
	 */
	int (*receive_packet) (struct cmd_channel *channel, struct cmd_packet *packet, int ms_timeout);

	/**
	 * Send a command packet over a communication channel.
	 *
	 * Returning from this function does not guarantee the packet has been fully transmitted.
	 * Depending on the channel implementation, it is possible the packet is still in flight with
	 * the data buffered in the channel driver.
	 *
	 * @param channel The channel to send a packet on.
	 * @param packet The packet to send.
	 *
	 * @return 0 if the the packet was successfully sent or an error code.
	 */
	int (*send_packet) (struct cmd_channel *channel, struct cmd_packet *packet);

	int id;					/**< ID for the command channel. */
	bool overflow;			/**< Flag if the channel is in an overflow condition. */
	platform_mutex lock;	/**< Synchronization for message transmission. */
};


int cmd_channel_get_id (struct cmd_channel *channel);

int cmd_channel_validate_packet_for_send (const struct cmd_packet *packet);
int cmd_channel_receive_and_process (struct cmd_channel *channel, struct mctp_interface *mctp,
	int ms_timeout);
int cmd_channel_send_message (struct cmd_channel *channel, struct cmd_message *message);

/* Internal functions for use by derived types. */
int cmd_channel_init (struct cmd_channel *channel, int id);
void cmd_channel_release (struct cmd_channel *channel);


#define	CMD_CHANNEL_ERROR(code)		ROT_ERROR (ROT_MODULE_CMD_CHANNEL, code)

/**
 * Error codes that can be generated by a command channel.
 */
enum {
	CMD_CHANNEL_INVALID_ARGUMENT = CMD_CHANNEL_ERROR (0x00),				/**< Input parameter is null or not valid. */
	CMD_CHANNEL_NO_MEMORY = CMD_CHANNEL_ERROR (0x01),						/**< Memory allocation failed. */
	CMD_CHANNEL_RX_FAILED = CMD_CHANNEL_ERROR (0x02),						/**< Error receiving a packet. */
	CMD_CHANNEL_TX_FAILED = CMD_CHANNEL_ERROR (0x03),						/**< Error sending a packet. */
	CMD_CHANNEL_RX_TIMEOUT = CMD_CHANNEL_ERROR (0x04),						/**< No packet was received within the specified time. */
	CMD_CHANNEL_TX_TIMEOUT = CMD_CHANNEL_ERROR (0x05),						/**< Packet transmission timed out. */
	CMD_CHANNEL_PKT_OVERFLOW = CMD_CHANNEL_ERROR (0x06),					/**< Packet overflow encountered. */
	CMD_CHANNEL_INVALID_PKT_STATE = CMD_CHANNEL_ERROR (0x07),				/**< Packet state is not valid. */
	CMD_CHANNEL_PKT_EXPIRED = CMD_CHANNEL_ERROR (0x08),						/**< The timeout on a received packet has expired. */
	CMD_CHANNEL_INVALID_PKT_SIZE = CMD_CHANNEL_ERROR (0x09),				/**< The packet size is larger than the buffer. */										/**< Socket success. */
	CMD_CHANNEL_SOC_INIT_FAILURE = CMD_CHANNEL_ERROR (0x0A),				/**< Socket initialization failure. */
	CMD_CHANNEL_SOC_NET_ADDRESS_FAILURE = CMD_CHANNEL_ERROR (0x0B),			/**< Network address structure failure. */
	CMD_CHANNEL_SOC_CONNECT_FAILURE = CMD_CHANNEL_ERROR (0x0C),				/**< Socket connections failure. */
	CMD_CHANNEL_SOC_SEND_FAILURE = CMD_CHANNEL_ERROR (0x0D),				/**< Socket send failure. */
	CMD_CHANNEL_SOC_RECEIVE_FAILURE = CMD_CHANNEL_ERROR (0x0E),				/**< Socket receive failure. */
	CMD_CHANNEL_SOC_SELECT_FAILURE = CMD_CHANNEL_ERROR (0x10),				/**< Socket select failure. */
	CMD_CHANNEL_SOC_TIMEOUT = CMD_CHANNEL_ERROR(0x11),						/**< Socket timeout error. */
	CMD_CHANNEL_SOC_BIND_FAILURE = CMD_CHANNEL_ERROR(0x12),					/**<Socket bind error. */
};


#endif /* CMD_CHANNEL_H_ */
