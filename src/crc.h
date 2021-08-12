/*
 * Copyright (c) 2021 Ably Medical
 */

#ifndef _CRC_H_
#define _CRC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define POLYNOMIUM 0x1021

/**
 * @brief
 *
 * @param addr
 * @param num
 * @return uint16_t
 */
static inline uint16_t calculate_crc16(const uint8_t *addr, int num)
{
	uint16_t crc = 0;

#if CONFIG_PROTOCOL_ENABLE_CHECKSUM
	for (int i = num; i > 0; i--) {
		crc = crc ^ (*addr++ << 8);

		for (int j = 0; j < 8; j++) {
			crc = crc << 1;

			if (crc & 0x10000) {
				crc = (crc ^ POLYNOMIUM) & 0xFFFF;
			}
		}
	}
#else
	(void)addr;
	(void)num;
#endif

	return crc;
}

#ifdef __cplusplus
}
#endif

#endif
