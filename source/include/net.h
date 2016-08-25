/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Gil Mendes <gil00mendes@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 * @brief   Network device support.
 */

#ifndef __NET_H
#define __NET_H

#include <device.h>

struct net_device;

/** Type used to store a MAC address */
typedef uint8_t mac_addr_t[16];

/** Type used to store an IPv4 address */
typedef union ipv4_addr {
  uint32_t val;
  uint8_t bytes[4];
} ipv4_addr_t;

/** TYpe used to store an IPv6 address */
typedef union ipv6_addr {
  struct {
    uint64_t high;
    uint64_t low;
  } val;
  uint8_t bytes[16];
} ipv6_addr_t;

/** Type used to store an IP address */
typedef union ip_addr {
  ipv4_addr_t v4;             /**< IPv4 address */
  ipv6_addr_t v6;             /**< IPv6 address */
} ip_addr_t;

/** Network device operations structure */
typedef struct net_ops {
  /**
   * Get identification information for the device.
   *
   * @param net     Device to identify.
   * @param type    Tye of the information to get.
   * @param buf     Where to store identification string.
   * @param size    Size of the buffer.
   */
  void (*identity)(struct net_device *net, device_identify_t type, char *buf, size_t size);
} net_ops_t;

/** Network device information */
typedef struct net_device {
  device_t device;              /**< Device header */

  /** Fields which should be initialized before registering */
  const net_ops_t *ops;         /**< Network device operations */
  uint32_t flags;               /**< Behavior flags */
  ip_addr_t ip;                 /**< IP address configured for the device */
  ip_addr_t gateway_ip;         /**< Gateway IP address */
  uint8_t hw_type;              /**< Hardware type (according to RFC 1700) */
  mac_addr_t hw_addr;           /**< MAC address of the device */
  uint8_t hw_addr_size;         /**< Hardware address size (in bytes) */
  ip_addr_t server_ip;          /**< Server IP address */
  uint16_t server_port;         /**< UDP port number of TFTP server */

  /** Fields set internally */
  unsigned id;                  /**< ID of the device */
} net_device_t;

/** Network device flags */
#define NET_DEVICE_IPV6 (1<<0)  /**< Device if configured using IPv6 */

/** Type used to store a MAC address is the BOOTP packet */
typedef uint8_t bootp_mac_addr_t[16];

/** BOOTP packet structure */
typedef struct bootp_packet {
  uint8_t opcode;               /**< message opcode */
  uint8_t hardware;             /**< hardware type */
  uint8_t hardware_len;         /**< hardware address length */
  uint8_t gate_hops;            /**< set to 0 */
  uint32_t ident;               /**< random number chosen by client */
  uint16_t seconds;             /**< seconds since obtained address */
  uint16_t flags;               /**< BOOTP/DHCP flags */
  ipv4_addr_t client_ip;        /**< client IP */
  ipv4_addr_t your_ip;          /**< your IP */
  ipv4_addr_t server_ip;        /**< server IP */
  ipv4_addr_t gateway_ip;       /**< gateway IP */
  bootp_mac_addr_t client_addr; /**< client hardware address */
  uint8_t server_name[64];      /**< server host name */
  uint8_t boot_file[128];       /**< boot file name */
  uint8_t vendor[64];           /**< DHCP vendor options */
} bootp_packet_t;

#ifdef CONFIG_TARGET_HAS_NET

extern void net_device_register(net_device_t *net, bool boot);
extern void net_device_register_with_bootp(net_device_t *net, bootp_packet_t *packet, bool boot);

#endif // CONFIG_TARGET_HAS_NET
#endif // __NET_H