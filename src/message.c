/*
 * Copyright (c) 2021 Ably Medical
 */

#include <message.h>
#include "portable.h"

#define PIEZO_DATA_HEADER_SIZE 3
#define ACCEL_SAMPLE_SET_SIZE 12 /* 3 * sizeof(uint32_t) */
#define PIEZO_SAMPLE_SET_HEADER_SIZE 4

static void encode_next8(uint8_t byte, uint8_t *const buf, int buflen, int *size, uint8_t *OK)
{
	if (*OK == 0 || *size >= buflen) {
		*OK = 0;
	} else {
		buf[*size] = byte;
		*size = *size + 1;
	}
}

static void encode_next16(uint16_t in, uint8_t *const buf, int buflen, int *size, uint8_t *OK)
{
	union {
		uint16_t f;
		uint8_t s[2];
	} tmp;

	tmp.f = htons(in);
	encode_next8(tmp.s[0], buf, buflen, size, OK);
	encode_next8(tmp.s[1], buf, buflen, size, OK);
}

static void encode_next32(uint32_t in, uint8_t *const buf, int buflen, int *size, uint8_t *OK)
{
	union {
		uint32_t f;
		uint8_t s[4];
	} tmp;

	tmp.f = htonl(in);
	encode_next8(tmp.s[0], buf, buflen, size, OK);
	encode_next8(tmp.s[1], buf, buflen, size, OK);
	encode_next8(tmp.s[2], buf, buflen, size, OK);
	encode_next8(tmp.s[3], buf, buflen, size, OK);
}

static void encode_accel_data(const struct piezo_sample_set_t *const sample, uint8_t *const buf,
			      int buflen, int *size, uint8_t *OK)
{
	/* We dont check buffer length as the check is done in calling function */
	for (int i = 0; i < ACCEL_CHANNELS; i++) {
		encode_next32(sample->accelerometer_data[i], buf, buflen, size, OK);
	}
}

static void encode_piezo_sample_set(const struct piezo_sample_set_t *const sample,
				    uint8_t *const buf, int buflen, int *size, uint8_t *OK)
{
	encode_accel_data(sample, buf, buflen, size, OK);
	encode_next16(sample->status, buf, buflen, size, OK);
	encode_next16(sample->sequence_number, buf, buflen, size, OK);

	for (int i = 0; i < CONFIG_PROTOCOL_MAX_PIEZO_CHANNELS; i++) {
		encode_next16(sample->samples[i], buf, buflen, size, OK);
	}
}

static inline int calc_sample_data_size(const struct piezo_data_t *const data)
{
	return PIEZO_DATA_HEADER_SIZE + (data->sets_per_message * ACCEL_SAMPLE_SET_SIZE) +
	       (data->sets_per_message *
		(PIEZO_SAMPLE_SET_HEADER_SIZE + (CONFIG_PROTOCOL_MAX_PIEZO_CHANNELS * 2)));
}

static void encode_piezo_data(const struct piezo_data_t *const data, uint8_t *const buf, int buflen,
			      int *size, uint8_t *OK)
{
	if (buflen < calc_sample_data_size(data)) {
		*OK = 0;
	} else {
		encode_next8(data->sequence_number, buf, buflen, size, OK);
		encode_next8(data->sets_per_message, buf, buflen, size, OK);
		encode_next8(data->samplerate_limiter, buf, buflen, size, OK);

		for (int i = 0; i < data->sets_per_message; i++) {
			encode_piezo_sample_set(&(data->sample_sets[i]), buf, buflen, size, OK);
		}
	}
}

int encode(const struct message_t *const msg, uint8_t *buf, int buflen)
{
	uint8_t OK = 1;
	int size = 0;

	memset(buf, 0, buflen);
	encode_next32(msg->header.code, buf, buflen, &size, &OK);
	encode_next32(msg->header.sequence_number, buf, buflen, &size, &OK);
	encode_next32(msg->header.timestamp, buf, buflen, &size, &OK);

	switch (msg->header.code) {
	case START_SAMPLING_REQUEST:
		encode_next32(4, buf, buflen, &size, &OK);
		encode_next32(msg->start_sampling.timestamp, buf, buflen, &size, &OK);
		break;

	case STOP_SAMPLING_REQUEST:
	case STOP_SAMPLING_RESPONSE:
	case START_SAMPLING_RESPONSE:
	case PIEZO_CONFIG_RESPONSE:
		encode_next32(0, buf, buflen, &size, &OK);
		break;

	case PIEZO_DATA:
		encode_next32(calc_sample_data_size(&(msg->piezo_data)), buf, buflen, &size, &OK);
		encode_piezo_data(&(msg->piezo_data), buf, buflen, &size, &OK);
		break;

	case PIEZO_CONFIG_REQUEST:
		encode_next32(2, buf, buflen, &size, &OK);
		encode_next8(msg->piezo_config.sets_per_message, buf, buflen, &size, &OK);
		encode_next8(msg->piezo_config.samplerate_limiter, buf, buflen, &size, &OK);
		break;

	case INVALID:
	case NO_OF_MESSAGE_TYPES:
	default:
		OK = 0;
		break;
	}

	return OK ? size : 0;
}

static uint8_t decode_next8(const uint8_t *const buf, int buflen, int *size, uint8_t *OK)
{
	if (*OK == 0 || *size >= buflen) {
		*OK = 0;
		return 0;
	} else {
		uint8_t b = buf[*size];
		*size = *size + 1;
		return b;
	}
}

static uint16_t decode_next16(const uint8_t *const buf, int buflen, int *size, uint8_t *OK)
{
	uint8_t b0 = decode_next8(buf, buflen, size, OK);
	uint8_t b1 = decode_next8(buf, buflen, size, OK);

	uint16_t portable = (b1 << 8) | b0;
	return ntohs(portable);
}

static uint32_t decode_next32(const uint8_t *const buf, int buflen, int *size, uint8_t *OK)
{
	uint8_t b0 = decode_next8(buf, buflen, size, OK);
	uint8_t b1 = decode_next8(buf, buflen, size, OK);
	uint8_t b2 = decode_next8(buf, buflen, size, OK);
	uint8_t b3 = decode_next8(buf, buflen, size, OK);

	uint32_t portable = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
	return ntohl(portable);
}

static void decode_accel_data(struct piezo_sample_set_t *const data, const uint8_t *const buf,
			      int buflen, int *size, uint8_t *OK)
{
	for (int i = 0; i < ACCEL_CHANNELS; i++) {
		data->accelerometer_data[i] = decode_next32(buf, buflen, size, OK);
	}
}

static void decode_piezo_sample_set(struct piezo_sample_set_t *const data, const uint8_t *const buf,
				    int buflen, int *size, uint8_t *OK)
{
	decode_accel_data(data, buf, buflen, size, OK);
	data->status = decode_next16(buf, buflen, size, OK);
	data->sequence_number = decode_next16(buf, buflen, size, OK);

	for (int i = 0; i < CONFIG_PROTOCOL_MAX_PIEZO_CHANNELS; i++) {
		data->samples[i] = decode_next16(buf, buflen, size, OK);
	}
}

static void decode_piezo_data(struct piezo_data_t *const data, const uint8_t *const buf, int buflen,
			      int *size, uint8_t *OK)
{
	data->sequence_number = decode_next8(buf, buflen, size, OK);
	data->sets_per_message = decode_next8(buf, buflen, size, OK);
	data->samplerate_limiter = decode_next8(buf, buflen, size, OK);

	for (int i = 0; i < data->sets_per_message; i++) {
		decode_piezo_sample_set(&data->sample_sets[i], buf, buflen, size, OK);
	}
}

int decode(struct message_t *const msg, const uint8_t *const buf, int buflen)
{
	uint8_t OK = 1;

	int size = decode_header(&msg->header, buf, buflen);

	if (size == 0) {
		return 0;
	}

	switch (msg->header.code) {
	case START_SAMPLING_REQUEST:
		msg->start_sampling.timestamp = decode_next32(buf, buflen, &size, &OK);
		break;

	case PIEZO_CONFIG_RESPONSE:
	case START_SAMPLING_RESPONSE:
	case STOP_SAMPLING_REQUEST:
	case STOP_SAMPLING_RESPONSE:
		break;

	case PIEZO_DATA:
		decode_piezo_data(&(msg->piezo_data), buf, buflen, &size, &OK);
		break;

	case PIEZO_CONFIG_REQUEST:
		msg->piezo_config.sets_per_message = decode_next8(buf, buflen, &size, &OK);
		msg->piezo_config.samplerate_limiter = decode_next8(buf, buflen, &size, &OK);
		break;

	case INVALID:
	case NO_OF_MESSAGE_TYPES:
	default:
		OK = 0;
		break;
	}

	return OK ? size : 0;
}

int decode_header(struct message_header_t *const header, const uint8_t *const buf, int buflen)
{
	uint8_t OK = 1;
	int size = 0;

	header->code = decode_next32(buf, buflen, &size, &OK);
	header->sequence_number = decode_next32(buf, buflen, &size, &OK);
	header->timestamp = decode_next32(buf, buflen, &size, &OK);
	header->payload_length = decode_next32(buf, buflen, &size, &OK);

	return OK ? size : 0;
}
