/*
 * Copyright (c) 2021 Ably Medical
 */

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

enum message_type_t {
	INVALID,

	START_SAMPLING_REQUEST,
	START_SAMPLING_RESPONSE,

	STOP_SAMPLING_REQUEST,
	STOP_SAMPLING_RESPONSE,

	PIEZO_DATA,
	PIEZO_CONFIG_REQUEST,
	PIEZO_CONFIG_RESPONSE,

	NO_OF_MESSAGE_TYPES,
};

struct start_sampling_data_t {
	uint32_t timestamp;
};

struct piezo_sample_set_t {
	uint16_t status;
	uint16_t sequence_number;
	int16_t samples[CONFIG_PROTOCOL_MAX_PIEZO_CHANNELS];
};

struct piezo_data_t {
	uint8_t sequence_number;
	uint8_t sets_per_message;
	uint8_t samplerate_limiter;
	struct piezo_sample_set_t sample_sets[CONFIG_PROTOCOL_MAX_PIEZO_SAMPLE_SETS_PER_MSG];
};

struct piezo_config_t {
	uint8_t sets_per_message;
	uint8_t samplerate_limiter;
};

struct message_t {
	enum message_type_t code;
	uint32_t sequence_number;
	uint32_t timestamp;

	union {
		struct start_sampling_data_t start_sampling;
		struct piezo_data_t piezo_data;
		struct piezo_config_t piezo_config;
	};

	uint32_t checksum;
};

/**
 * @brief encodes a message object into a byte buffer
 *
 * @param message pointer to a message object that will be encoded as bytes
 * @param buf byte buffer large enough to hold the encoded message
 * @param buflen byte buffer capacity that the encoder will respect
 * @return int number of encoded bytes on success, 0 on error
 */
int encode(const struct message_t *const message, uint8_t *buf, int buflen);

/**
 * @brief decode a byte buffer into a message object
 *
 * @param message pointer to a message object that will hold the decoded data
 * @param buf byte buffer containing the encoded message
 * @param buflen number of bytes in the buffer
 * @return int number of decoded bytes on success, 0 on error
 */
int decode(struct message_t *const message, uint8_t *const buf, int buflen);

#ifdef __cplusplus
}
#endif

#endif // _MESSAGE_H_
