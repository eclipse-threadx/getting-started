/**
  ******************************************************************************
  * @file    mx_wifi_spi.c
  * @author  MCD Application Team
  * @brief   This file implements the IO operations to deal with the mx_wifi
  *          module. It mainly Inits and Deinits the SPI interface. Send and
  *          receive data over it.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include <main.h>
#include <string.h>
#include "mx_wifi.h"
#include "core/mx_wifi_hci.h"

#if 0
#define DEBUG_ERROR(M, ...)     printf((M), ##__VA_ARGS__)
#define DEBUG_LOG(M, ...)       printf((M), ##__VA_ARGS__)
#define DEBUG_WARNING(M, ...)   printf((M), ##__VA_ARGS__)
#else
#define DEBUG_ERROR     printf
#define DEBUG_LOG(...)
#define DEBUG_WARNING(...)
#endif /* Debug 0 */


#pragma pack(1)
typedef struct _spi_header
{
  uint8_t  type;
  uint16_t len;
  uint16_t lenx;
  uint8_t  dummy[3];
} spi_header_t;
#pragma pack()


#ifndef MX_WIFI_RESET_PIN

#define MX_WIFI_RESET_PIN        MXCHIP_RESET_Pin
#define MX_WIFI_RESET_PORT       MXCHIP_RESET_GPIO_Port

/* wifi spi cs */
#define MX_WIFI_SPI_CS_PIN       MXCHIP_NSS_Pin
#define MX_WIFI_SPI_CS_PORT      MXCHIP_NSS_GPIO_Port

/* wifi spi slave notify */
#define MX_WIFI_SPI_IRQ_PIN      MXCHIP_NOTIFY_Pin
#define MX_WIFI_SPI_IRQ_PORT     MXCHIP_NOTIFY_GPIO_Port
#define MX_WIFI_SPI_IRQ          MXCHIP_NOTIFY_EXTI_IRQn

#define MX_WIFI_SPI_FLOW_PIN     MXCHIP_FLOW_Pin
#define MX_WIFI_SPI_FLOW_PORT    MXCHIP_FLOW_GPIO_Port

#endif /* MX_WIFI_RESET_PIN */


#if (MX_WIFI_USE_SPI == 1)

#ifndef NET_PERF_TASK_TAG
#define NET_PERF_TASK_TAG(...)
#endif /* NET_PERF_TASK_TAG */

/* Private define ------------------------------------------------------------*/
/* SPI protocol */
#define SPI_WRITE         (0x0A)
#define SPI_READ          (0x0B)
#define SPI_HEADER_SIZE   (5)
#define SPI_DATA_SIZE     (MX_WIFI_HCI_DATA_SIZE)

#define SPI_WRITE_SLAVE_IDLE_TIMEOUT    (100)
#define SPI_WRITE_DATA_TIMEOUT          (100)
#define SPI_READ_DATA_TIMEOUT           (100)

/* HW RESET */

#define MX_WIFI_HW_RESET() \
  do { \
    HAL_GPIO_WritePin(MX_WIFI_RESET_PORT, MX_WIFI_RESET_PIN, GPIO_PIN_RESET); \
    HAL_Delay(100); \
    HAL_GPIO_WritePin(MX_WIFI_RESET_PORT, MX_WIFI_RESET_PIN, GPIO_PIN_SET); \
    HAL_Delay(1200); \
  } while(0)

/* SPI CS */
#define MX_WIFI_SPI_CS_HIGH() \
  HAL_GPIO_WritePin( MX_WIFI_SPI_CS_PORT, MX_WIFI_SPI_CS_PIN, GPIO_PIN_SET)

#define MX_WIFI_SPI_CS_LOW() \
  HAL_GPIO_WritePin( MX_WIFI_SPI_CS_PORT, MX_WIFI_SPI_CS_PIN, GPIO_PIN_RESET)

#define MX_WIFI_SPI_CS_IS_HIGH() \
  (GPIO_PIN_SET == HAL_GPIO_ReadPin(MX_WIFI_SPI_CS_PORT, MX_WIFI_SPI_CS_PIN))

/* SPI IRQ */
#define MX_WIFI_SPI_IRQ_ENABLE() \
  HAL_NVIC_EnableIRQ(MX_WIFI_SPI_IRQ)

#define MX_WIFI_SPI_IRQ_DISABLE() \
  HAL_NVIC_DisableIRQ(MX_WIFI_SPI_IRQ)

#define MX_WIFI_SPI_IRQ_IS_HIGH() \
  (GPIO_PIN_SET == HAL_GPIO_ReadPin(MX_WIFI_SPI_IRQ_PORT, MX_WIFI_SPI_IRQ_PIN))

#define MX_WIFI_SPI_FLOW_IS_LOW() \
  (GPIO_PIN_RESET == HAL_GPIO_ReadPin(MX_WIFI_SPI_FLOW_PORT, MX_WIFI_SPI_FLOW_PIN))

/* Global variables  ---------------------------------------------------------*/
extern SPI_HandleTypeDef MXCHIP_SPI;

/* Private variables ---------------------------------------------------------*/
static MX_WIFIObject_t MxWifiObj;
static SPI_HandleTypeDef *hspi_mx = &MXCHIP_SPI;

/* Private functions ---------------------------------------------------------*/
static uint16_t MX_WIFI_SPI_Read(uint8_t *buffer, uint16_t buff_size);
static int32_t TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint8_t *rxdata, uint32_t datalen,
                               uint32_t timeout);
static int32_t Transmit(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint32_t datalen, uint32_t timeout);
static int32_t Receive(SPI_HandleTypeDef *hspi, uint8_t *rxdata, uint32_t datalen, uint32_t timeout);

static int8_t wait_flow_high(uint32_t timeout);
static uint16_t MX_WIFI_SPI_Write(uint8_t *data, uint16_t len);

static SEM_DECLARE(spi_txrx_sem);
static SEM_DECLARE(spi_flow_rise_sem);
static SEM_DECLARE(spi_transfer_done_sem);

static uint8_t *spi_tx_data = NULL;
static uint16_t spi_tx_len  = 0;

THREAD_DECLARE(MX_WIFI_TxRxThreadId);
static int8_t mx_wifi_spi_txrx_start(void);
static int8_t mx_wifi_spi_txrx_stop(void);

static void MX_WIFI_IO_DELAY(uint32_t ms);
static int8_t MX_WIFI_SPI_Init(uint16_t mode);
static int8_t MX_WIFI_SPI_DeInit(void);


#ifndef MX_WIFI_BARE_OS_H
static void mx_wifi_spi_txrx_task(THREAD_CONTEXT_TYPE argument);
#endif /* MX_WIFI_BARE_OS_H */

/* SPI protocol functions */
void process_txrx_poll(uint32_t timeout);


static void MX_WIFI_IO_DELAY(uint32_t ms)
{
  DELAYms(ms);
}


/**
  * @brief  Initialize the SPI1
  * @param  None
  * @retval None
  */
static int8_t MX_WIFI_SPI_Init(uint16_t mode)
{
  int8_t ret = 0;
  if (MX_WIFI_RESET == mode)
  {
    MX_WIFI_HW_RESET();
  }
  else
  {
    ret = mx_wifi_spi_txrx_start();
  }
  return ret;
}


/**
  * @brief  DeInitialize the SPI
  * @param  None
  * @retval None
  */
static int8_t MX_WIFI_SPI_DeInit(void)
{
  mx_wifi_spi_txrx_stop();
  return 0;
}


void HAL_SPI_TransferCallback(SPI_HandleTypeDef *hspi)
{
  SEM_SIGNAL(spi_transfer_done_sem);
}


void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
  while (1);
}


/**
  * @brief  Interrupt handler for  IRQ and FLOW pin
  * @param  None
  * @retval None
  */
void mxchip_WIFI_ISR(uint16_t isr_source)
{
  if (MX_WIFI_SPI_IRQ_PIN == isr_source)
  {
    SEM_SIGNAL(spi_txrx_sem);
  }
  if (MX_WIFI_SPI_FLOW_PIN == isr_source)
  {
    SEM_SIGNAL(spi_flow_rise_sem);
  }
}


static int8_t wait_flow_high(uint32_t timeout)
{
  int8_t ret = 0;
  if (SEM_WAIT(spi_flow_rise_sem, timeout, NULL) != SEM_OK)
  {
    ret = -1;
  }
  if (MX_WIFI_SPI_FLOW_IS_LOW())
  {
    printf("flow is low\r\n");
    return -1;
  }
  return ret;
}


static uint16_t MX_WIFI_SPI_Write(uint8_t *data, uint16_t len)
{
  if ((NULL == data) || (0 == len) || (len > SPI_DATA_SIZE))
  {
    return 0;
  }

  spi_tx_data = data;
  spi_tx_len  = len;

  if (SEM_SIGNAL(spi_txrx_sem) != SEM_OK)
  {
    /* Happen if received thread did not have a chance to run on time, need to increase priority */
    DEBUG_WARNING("Warning, spi semaphore has been already notified\n");
  }

  return len;
}


static uint16_t MX_WIFI_SPI_Read(uint8_t *buffer, uint16_t buff_size)
{
  return 0;
}


#if DMA_ON_USE == 1

static int32_t TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint8_t *rxdata, uint32_t datalen,
                               uint32_t timeout)
{
  int32_t ret;
  DEBUG_LOG("Spi Tx Rx %d\n", datalen);

  ret = HAL_SPI_TransmitReceive_DMA(hspi, txdata, rxdata, datalen);
  SEM_WAIT(spi_transfer_done_sem, timeout, NULL);
  return ret;
}


static int32_t Transmit(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint32_t datalen, uint32_t timeout)
{
  int32_t ret;
  DEBUG_LOG("Spi Tx %d\n", datalen);

  ret = HAL_SPI_Transmit_DMA(hspi, txdata, datalen);
  SEM_WAIT(spi_transfer_done_sem, timeout, NULL);
  return ret;
}

static int32_t Receive(SPI_HandleTypeDef *hspi, uint8_t *rxdata, uint32_t datalen, uint32_t timeout)
{
  int32_t ret;
  DEBUG_LOG("Spi Rx %d\n", datalen);

  ret = HAL_SPI_Receive_DMA(hspi, rxdata, datalen);
  SEM_WAIT(spi_transfer_done_sem, timeout, NULL);
  return ret;
}

#else

static int32_t TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint8_t *rxdata, uint32_t datalen,
                               uint32_t timeout)
{
  int32_t ret;
  DEBUG_LOG("Spi Tx Rx %d\n", datalen);
  ret = HAL_SPI_TransmitReceive(hspi, txdata, rxdata, datalen, timeout);
  return ret;
}

static int32_t Transmit(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint32_t datalen, uint32_t timeout)
{
  int32_t ret;
  DEBUG_LOG("Spi Tx %d\n", datalen);
  ret = HAL_SPI_Transmit(hspi, txdata, datalen, timeout);
  return ret;
}

static int32_t Receive(SPI_HandleTypeDef *hspi, uint8_t *rxdata, uint32_t datalen, uint32_t timeout)
{
  int32_t ret;
  DEBUG_LOG("Spi Rx %d\n", datalen);
  ret = HAL_SPI_Receive(hspi, rxdata, datalen, timeout);
  return ret;
}
#endif /* DMA_ON_USE */

void process_txrx_poll(uint32_t timeout)
{
  spi_header_t mheader, sheader;
  int32_t  ret;
  uint8_t *txdata;
  uint8_t *p = NULL;
  uint16_t datalen;
  static mx_buf_t *netb = NULL;
  bool first_miss = true;

  MX_WIFI_SPI_CS_HIGH();

  while (netb == NULL)
  {
    netb = MX_NET_BUFFER_ALLOC(MX_WIFI_BUFFER_SIZE);
    if (netb == NULL)
    {
      DELAYms(1);
      if (true == first_miss)
      {
        first_miss = false;
        DEBUG_WARNING("Running Out of buffer for RX\n");
      }
    }
  }

  /* waiting for data to be sent or to be received */
  if (SEM_WAIT(spi_txrx_sem, timeout, NULL) == SEM_OK)
  {
    NET_PERF_TASK_TAG(0);

    if (spi_tx_data == NULL)
    {
      if (!MX_WIFI_SPI_IRQ_IS_HIGH())
      {
        /* tx data null means no data to senf , IRQ low meand no data to received */
        return;
      }
      txdata = NULL;
      mheader.len = 0;
    }
    else
    {
      mheader.len = spi_tx_len;
      txdata = spi_tx_data;
    }

    mheader.type = SPI_WRITE;
    mheader.lenx = ~mheader.len;

    MX_WIFI_SPI_CS_LOW();

    /* wait EMW to be ready */
    if (wait_flow_high(20) != 0)
    {
      MX_WIFI_SPI_CS_HIGH();
      DEBUG_ERROR("wait flow timeout 0\r\n");
      return;
    }

    /* transmit only header part */
    sheader.type = 0;
    sheader.len = 0;

    if (HAL_OK != TransmitReceive(hspi_mx, (uint8_t *)&mheader, (uint8_t *)&sheader, sizeof(mheader), timeout))
    {
      MX_WIFI_SPI_CS_HIGH();
      DEBUG_ERROR("Send mheader error\r\n");
      return;
    }

    if (sheader.type != SPI_READ)
    {
      DEBUG_ERROR("Invalid SPI type %02x\r\n", sheader.type);
      MX_WIFI_SPI_CS_HIGH();
      return;
    }
    if ((sheader.len ^ sheader.lenx) != 0xFFFF)
    {
      MX_WIFI_SPI_CS_HIGH();
      DEBUG_ERROR("Invalid len %04x-%04x\r\n", sheader.len, sheader.lenx);
      return;
    }

    /* send or received header must be not null */
    if ((sheader.len == 0) && (mheader.len == 0))
    {
      MX_WIFI_SPI_CS_HIGH();
      return;
    }

    if ((sheader.len > SPI_DATA_SIZE) || (mheader.len > SPI_DATA_SIZE))
    {
      MX_WIFI_SPI_CS_HIGH();
      DEBUG_ERROR("SPI length invalid: %d-%d\r\n", sheader.len, mheader.len);
      return;
    }

    /* keep max length */
    if (mheader.len > sheader.len)
    {
      datalen = mheader.len;
    }
    else
    {
      datalen = sheader.len;
    }

    /* allocate a buffer */
    if (sheader.len > 0)
    {
      /* get payload */
      p = MX_NET_BUFFER_PAYLOAD(netb);
    }
    else
    {
      p = NULL;
    }

    /* flow must be high */

    if (wait_flow_high(20) != 0)
    {
      MX_WIFI_SPI_CS_HIGH();
      DEBUG_ERROR("wait flow timeout 1\r\n");
      return;
    }

    /* transmit and received */
    if (NULL != txdata)
    {
      spi_tx_data = NULL;
      spi_tx_len = 0;
      if (NULL != p)
      {
        ret = TransmitReceive(hspi_mx, txdata, p, datalen, timeout);
      }
      else
      {
        ret = Transmit(hspi_mx, txdata, datalen, timeout);
      }
    }
    else
    {
      ret = Receive(hspi_mx, p, datalen, timeout);
    }

    if (HAL_OK != ret)
    {
      MX_WIFI_SPI_CS_HIGH();
      DEBUG_ERROR("Transmit/Receive data timeout\r\n");
      return;
    }

    /* resize the input buffer and sent it back to processing thread */
    if (sheader.len > 0)
    {
      NET_PERF_TASK_TAG(1);
      MX_NET_BUFFER_SET_PAYLOAD_SIZE(netb, sheader.len);
      mx_wifi_hci_input(netb);
      netb = NULL;

    }
    else
    {
      NET_PERF_TASK_TAG(2);
    }
  }
}

#ifndef MX_WIFI_BARE_OS_H
static void mx_wifi_spi_txrx_task(THREAD_CONTEXT_TYPE argument)
{
  while (1)
  {
    process_txrx_poll(WAIT_FOREVER);
  }
}
#endif /*MX_WIFI_BARE_OS_H */

static int8_t mx_wifi_spi_txrx_start(void)
{
  int8_t ret = 0;

  SEM_INIT(spi_txrx_sem, 2);
  SEM_INIT(spi_flow_rise_sem, 1);
  SEM_INIT(spi_transfer_done_sem, 1);

  if (THREAD_OK != THREAD_INIT(MX_WIFI_TxRxThreadId, mx_wifi_spi_txrx_task, NULL,
                               MX_WIFI_SPI_THREAD_STACK_SIZE,
                               MX_WIFI_SPI_THREAD_PRIORITY))
  {
    ret = -1;
  }
  return ret;
}

static int8_t mx_wifi_spi_txrx_stop(void)
{
  THREAD_DEINIT(MX_WIFI_TxRxThreadId);
  SEM_DEINIT(spi_txrx_sem);
  SEM_DEINIT(spi_flow_rise_sem);
  return 0;
}


/**
  * @brief  probe function to register wifi to connectivity framwotk
  * @param  None
  * @retval None
  */
int32_t mxwifi_probe(void **ll_drv_context)
{
  if (MX_WIFI_RegisterBusIO(&MxWifiObj,
                            MX_WIFI_SPI_Init,
                            MX_WIFI_SPI_DeInit,
                            MX_WIFI_IO_DELAY,
                            MX_WIFI_SPI_Write,
                            MX_WIFI_SPI_Read) == 0)
  {
    *ll_drv_context = &MxWifiObj;
    return 0;
  }

  return -1;
}


MX_WIFIObject_t *wifi_obj_get(void)
{
  return &MxWifiObj;
}

#endif /* (MX_WIFI_USE_SPI == 1) */
