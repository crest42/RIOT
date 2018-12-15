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
    uint8_t dma_stream;                 /**< DMA stream used for TX */
    uint8_t dma_chan;                   /**< DMA channel used for TX */
    char mac[6];                        /**< Et hernet MAC address */
    char phy_addr;                      /**< PHY address */
    gpio_t pins[];                      /**< Pins to use. MII requires 18 pins,
                                             RMII 9 and SMI 9. Not all speeds are
                                             supported by all modes. */
} eth_conf_t;

  /**
   * @brief   Power on the DMA device the given stream belongs to
   *
   * @param[in] stream    logical DMA stream
   */
  static inline void dma_poweron(int stream)
  {
      if (stream < 8) {
          periph_clk_en(AHB1, RCC_AHB1ENR_DMA1EN);
      }
      else {
          periph_clk_en(AHB1, RCC_AHB1ENR_DMA2EN);
      }
  }

  /**
   * @brief   Get DMA base register
   *
   * For simplifying DMA stream handling, we map the DMA channels transparently to
   * one integer number, such that DMA1 stream0 equals 0, DMA2 stream0 equals 8,
   * DMA2 stream 7 equals 15 and so on.
   *
   * @param[in] stream    logical DMA stream
   */
  static inline DMA_TypeDef *dma_base(int stream)
  {
      return (stream < 8) ? DMA1 : DMA2;
  }

  /**
   * @brief   Get the DMA stream base address
   *
   * @param[in] stream    logical DMA stream
   *
   * @return  base address for the selected DMA stream
   */
  static inline DMA_Stream_TypeDef *dma_stream(int stream)
  {
      uint32_t base = (uint32_t)dma_base(stream);
      return (DMA_Stream_TypeDef *)(base + (0x10 + (0x18 * (stream & 0x7))));
  }
 /**
   * @brief   Select high or low DMA interrupt register based on stream number
   *
   * @param[in] stream    logical DMA stream
   *
   * @return  0 for streams 0-3, 1 for streams 3-7
   */
  static inline int dma_hl(int stream)
  {
    return ((stream & 0x4) >> 2);
  }

  /**
   * @brief   Get the interrupt flag clear bit position in the DMA LIFCR register
   *
   * @param[in] stream    logical DMA stream
   */
  static inline uint32_t dma_ifc(int stream)
  {
    switch (stream & 0x3) {
          case 0:
            return (1 << 5);
          case 1:
            return (1 << 11);
          case 2:
            return (1 << 21);
          case 3:
            return (1 << 27);
          default:
              return 0;
      }
  }

  static inline void dma_isr_enable(int stream)
  {
      if (stream < 7) {
          NVIC_EnableIRQ((IRQn_Type)((int)DMA1_Stream0_IRQn + stream));
      }
      else if (stream == 7) {
          NVIC_EnableIRQ(DMA1_Stream7_IRQn);
      }
      else if (stream < 13) {
          NVIC_EnableIRQ((IRQn_Type)((int)DMA2_Stream0_IRQn + (stream - 8)));
      }
      else if (stream < 16) {
          NVIC_EnableIRQ((IRQn_Type)((int)DMA2_Stream5_IRQn + (stream - 13)));
      }
  }

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CPU_H */
/** @} */
