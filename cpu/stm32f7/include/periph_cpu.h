/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_stm32f7
 * @{
 *
 * @file
 * @brief           CPU specific definitions for internal peripheral handling
 *
 * @author          Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 */

#ifndef PERIPH_CPU_H
#define PERIPH_CPU_H

#include "cpu.h"
#include "periph_cpu_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Starting address of the CPU ID
 */
#ifdef CPU_LINE_STM32F722xx
#define CPUID_ADDR          (0x1ff07a10)
#else
#define CPUID_ADDR          (0x1ff0f420)
#endif

/**
 * @brief   Available ports
 */
enum {
    PORT_A = 0,             /**< port A */
    PORT_B = 1,             /**< port B */
    PORT_C = 2,             /**< port C */
    PORT_D = 3,             /**< port D */
    PORT_E = 4,             /**< port E */
    PORT_F = 5,             /**< port F */
    PORT_G = 6,             /**< port G */
    PORT_H = 7,             /**< port H */
    PORT_I = 8,             /**< port I */
    PORT_J = 9,             /**< port J */
    PORT_K = 10             /**< port K */
};

/**
 * @brief   Ethernet Peripheral configuration
 */
typedef struct {
    enum {
        MII = 18,                       /**< Configuration for MII */
        RMII = 9,                       /**< Configuration for RMII */
        SMI = 2,                        /**< Configuration for SMI */
    } mode;                             /**< Select configuration mode */
    enum {
        ETH_SPEED_10T_HD = 0x0000,
        ETH_SPEED_10T_FD = 0x0100,
        ETH_SPEED_100TX_HD = 0x2000,
        ETH_SPEED_100TX_FD = 0x2100,
    } speed;                            /**< Speed selection */
    uint8_t dma;                        /**< Locical CMA Descriptor used for TX */
    uint8_t dma_chan;                   /**< DMA channel used for TX */
    char mac[6];                        /**< Et hernet MAC address */
    char phy_addr;                      /**< PHY address */
    gpio_t pins[];                      /**< Pins to use. MII requires 18 pins,
                                             RMII 9 and SMI 9. Not all speeds are
                                             supported by all modes. */
} eth_conf_t;

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CPU_H */
/** @} */
