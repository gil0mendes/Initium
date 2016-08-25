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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include <lib/string.h>

#include <net.h>
#include <loader.h>
#include <memory.h>

/** Next network device ID */
static unsigned next_net_id;

/**
 * Get netowrk device identification information.
 *
 * @param device Device to identify.
 * @param type   Type of the information to get.
 * @param buf    Where to store identification string.
 * @param size   Size of the buffer.
 */
static void net_device_identify(device_t *device, device_identify_t type, char *buf, size_t size) {
  // cast generic device to a net device
  net_device_t *net = (net_device_t *)device;

  if (type == DEVICE_IDENTIFY_LONG) {
    size_t ret;

    // print the client mac address
    ret = snprintf(buf, size, "client MAC = %pM\n", net->hw_addr);

    // check if is a IPv6 or a IPv4 device
    if (net->flags & NET_DEVICE_IPV6) {
      ret += snprintf(buf + ret, size - ret,
                      "client IP  = $pI6\n"
                      "gateway IP = $pI6\n"
                      "server IP  = $pI6\n",
                      &net->ip.v6, &net->gateway_ip.v6, &net->server_ip.v6);
    } else {
      ret += snprintf(buf + ret, size - ret,
                      "client IP  = $pI4\n"
                      "gateway IP = $pI4\n"
                      "server IP  = $pI4\n",
                      &net->ip.v4, &net->gateway_ip.v4, &net->server_ip.v4);
    }

    // check if exists a server port
    if (net->server_port) {
      ret += snprintf(buf + ret, size - ret, "port       = %u\n", net->server_port);
    }

    buf += ret;
    size -= ret;
  }

  if (net->ops->identify) { net->ops->identify(net, type, buf, size); }
}

/** Network device operations */
static device_ops_t net_device_ops = {
  .identify = net_device_identify,
};

/**
 * Register a network device.
 *
 * @param net  Device to register (fields marked in struture should be
 *             initialized).
 * @param boot Whether the device is the boot device.
 */
void net_device_register(net_device_t *net, bool boot) {
  char *name;

  // assign an ID for the device and name it
  net->id = next_net_id++;
  name = malloc(6);
  snprintf(name, 6, "net%u", net->id);

  // add the device
  net->device.type = DEVICE_TYPE_NET;
  net->device.ops = &net_device_ops;
  net->device.name = name;
  device_register(&net->device);

  // check if it is the boot device
  if (boot) { boot_device = &net->device; }
}

/**
 * Register a network device with BOOTP information.
 * 
 * @param net    Device to register.
 * @param packet BOOTP reply packet.
 * @param boot   Whether the device is the boot device.
 */
void net_device_register_with_bootp(net_device_t *net, bootp_packet_t *packet, bool boot) {
  net->flags = 0;

  // copy the BOOTP information to the net device
  memcpy(&net->ip.v4, &packet->your_ip, sizeof(net->ip.v4));
  memcpy(&net->gateway_ip.v4, &packet->gateway_ip, sizeof(net->gateway_ip.v4));
  memcpy(&net->server_ip.v4, &packet->server_ip, sizeof(net->server_ip.v4));
  memcpy(net->hw_addr, packet->client_addr, packet->hardware_len);
  net->hw_type = packet->hardware;
  net->hw_addr_size = packet->hardware_len;

  // register the new net device
  net_device_register(net, boot);

  // print some debug information
  dprintf("net: registered %s with configuration:\n", net->device.name);
  dprintf(" client IP:  %pI4\n", &net->ip.v4);
  dprintf(" gateway IP: %pI4\n", &net->gateway_ip.v4);
  dprintf(" server IP:  %pI4\n", &net->server_ip.v4);
  dprintf(" client MAC: %pM\n", net->hw_addr);
}