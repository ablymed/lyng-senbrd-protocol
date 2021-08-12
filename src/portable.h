/*
 * Copyright (c) 2021 Ably Medical
 */

#ifndef _PORTABLE_H_
#define _PORTABLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_SENBRD_PROTOCOL

#include <sys/byteorder.h>

#define ntohs(x) sys_be16_to_cpu(x)
#define ntohl(x) sys_be32_to_cpu(x)
#define htons(x) sys_cpu_to_be16(x)
#define htonl(x) sys_cpu_to_be32(x)

#elif CONFIG_BIG_ENDIAN

#define htons(A) (A)
#define htonl(A) (A)
#define ntohs(A) (A)
#define ntohl(A) (A)

#else

#define htons(A) ((((uint16_t)(A)&0xff00) >> 8) | (((uint16_t)(A)&0x00ff) << 8))
#define htonl(A)                                                                                   \
	((((uint32_t)(A)&0xff000000) >> 24) | (((uint32_t)(A)&0x00ff0000) >> 8) |                  \
	 (((uint32_t)(A)&0x0000ff00) << 8) | (((uint32_t)(A)&0x000000ff) << 24))
#define ntohs htons
#define ntohl htonl

#endif

#ifdef __cplusplus
}
#endif

#endif
