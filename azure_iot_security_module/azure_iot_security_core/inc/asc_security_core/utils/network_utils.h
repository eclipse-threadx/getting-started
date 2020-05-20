/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <stdint.h>

#define IPV6_ADDRESS_COPY(source, destination) \
    do {                                       \
        (destination)[0] = (source)[0];        \
        (destination)[1] = (source)[1];        \
        (destination)[2] = (source)[2];        \
        (destination)[3] = (source)[3];        \
    } while (0)


#define IPV6_ADDRESS_HASH(address) \
    ((address)[0] ^ (address)[1] ^ (address)[2] ^ (address)[3])


#define IPV6_ADDRESS_EQUALS(a, b)   \
    (                               \
        (a)[0] == (b)[0] &&         \
        (a)[1] == (b)[1] &&         \
        (a)[2] == (b)[2] &&         \
        (a)[3] == (b)[3]            \
    )


typedef enum NETWORK_PROTOCOL_TAG {
    NETWORK_PROTOCOL_IPV4,
    NETWORK_PROTOCOL_IPV6
} NETWORK_PROTOCOL;

/**
 * @brief Convert IPv4 and IPv6 addresses from binary to text form. See man inet_ntop(3) for more info.
 *
 * @param address_family The network protocol.
 * @param source A pointer to the source buffer, in network byte order.
 * @param destination A pointer to the destination buffer.
 * @param size The size of the destination buffer.
 *
 * @return On success, the address of the destination buffer, NULL otherwise.
 */
const char* network_utils_inet_ntop(NETWORK_PROTOCOL network_protocol, const void* source, char* destination, uint32_t size);


/**
 * @brief Convert IPv4 and IPv6 addresses from text to binary form. See man inet_pton(3) for more info.
 *
 * @param address_family The network protocol.
 * @param source A pointer to the source buffer, in network byte order.
 * @param destination A pointer to the destination buffer.
 * @param size The size of the destination buffer.
 *
 * @return On success, the address of the destination buffer, NULL otherwise.
 */
int network_utils_inet_pton(NETWORK_PROTOCOL network_protocol, const char* source, void* destination);


#endif /* NETWORK_UTILS_H */