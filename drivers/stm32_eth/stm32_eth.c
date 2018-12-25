/*
 * Copyright (C) 2016 TriaGnoSys GmbH
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     stm32_eth_netdev
 * @{
 *
 * @file
 * @brief       Netdev wrapper for stm32 ethernet
 *
 * @author      Víctor Ariño <victor.arino@triagnosys.com>
 *
 * @}
 */

#include "periph_conf.h"
#include "mutex.h"
#include "net/netdev/eth.h"
#include "net/ethernet.h"
#define ENABLE_DEBUG (0)
#include "debug.h"

#include <string.h>
static mutex_t _tx = MUTEX_INIT;
static mutex_t _rx = MUTEX_INIT;
char send_buf[ETH_TX_BUFFER_SIZE];
netdev_t *_netdev;

void set_mac(const char *mac);
void get_mac(char *out);
int eth_init(void);
int eth_receive_blocking(char *data, unsigned max_len);
int eth_send(const char *data, unsigned len);
int get_rx_status_owned(void);

static void _isr(netdev_t *netdev) {
    if(get_rx_status_owned()) {
        netdev->event_callback(netdev, NETDEV_EVENT_RX_COMPLETE);
    }
}

void isr_eth(void)
{
    volatile unsigned tmp = ETH->DMASR;

    if ((tmp & ETH_DMASR_TS)) {
        ETH->DMASR = ETH_DMASR_TS | ETH_DMASR_NIS;
        mutex_unlock(&_tx);
    }

    if ((tmp & ETH_DMASR_RS)) {
        ETH->DMASR = ETH_DMASR_RS | ETH_DMASR_NIS;
        mutex_unlock(&_rx);
        if (_netdev) {
            _netdev->event_callback(_netdev, NETDEV_EVENT_ISR);
        }
    }

    /* printf("r:%x\n\n", tmp); */

    cortexm_isr_end();
}

static int _recv(netdev_t *netdev, void *buf, size_t len, void *info)
{
    (void)info;
    (void)netdev;
    if(!get_rx_status_owned()){
                mutex_lock(&_rx);
    }
    int ret = eth_receive_blocking((char *)buf, len);
#ifdef MODULE_NETSTATS_L2
    if (buf) {
        netdev->stats.rx_count++;
        netdev->stats.rx_bytes += len;
    }
#endif

    DEBUG("stm32_eth_netdev: _recev: %d\n", ret);

    return ret;
}

static int _send(netdev_t *netdev, const struct iolist *iolist)
{
    (void)netdev;
    int ret = 0, len = 0;
    if(get_rx_status_owned()) {
        mutex_lock(&_tx);
    }
    for (const iolist_t *iol = iolist; iol; iol = iol->iol_next) {
      memcpy(send_buf+len,iol->iol_base, iol->iol_len);
      len += iol->iol_len;
    }
    ret = eth_send(send_buf,len);
    DEBUG("stm32_eth_netdev: _send: %d %d\n", ret, len);

#ifdef MODULE_NETSTATS_L2
    netdev->stats.tx_bytes += len;
#endif

    if (ret < 0) {
        return ret;
    }

    return len;
}

static int _set(netdev_t *dev, netopt_t opt, const void *value, size_t max_len)
{
    int res = -1;

    switch (opt) {
        case NETOPT_ADDRESS:
            assert(max_len >= ETHERNET_ADDR_LEN);
            set_mac((char *)value);
            res = ETHERNET_ADDR_LEN;
            break;
        default:
            res = netdev_eth_set(dev, opt, value, max_len);
            break;
    }

    return res;
}

static int _get(netdev_t *dev, netopt_t opt, void *value, size_t max_len)
{
    int res = -1;

    switch (opt) {
        case NETOPT_ADDRESS:
            assert(max_len >= ETHERNET_ADDR_LEN);
            get_mac((char *)value);
            res = ETHERNET_ADDR_LEN;
            break;
        default:
            res = netdev_eth_get(dev, opt, value, max_len);
            break;
    }

    return res;
}

static int _init(netdev_t *netdev)
{
    (void)netdev;
    return eth_init();
}

static const netdev_driver_t netdev_driver_stm32f4eth = {
    .send = _send,
    .recv = _recv,
    .init = _init,
    .isr = _isr,
    .get = _get,
    .set = _set,
};

void stm32_eth_netdev_setup(netdev_t *netdev)
{
    _netdev = netdev;
    netdev->driver = &netdev_driver_stm32f4eth;
}