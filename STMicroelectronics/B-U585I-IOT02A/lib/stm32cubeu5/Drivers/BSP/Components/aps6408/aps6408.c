/**
  ******************************************************************************
  * @file    aps6408.c
  * @modify  MCD Application Team
  * @brief   This file provides the APS6408 SRAM OSPI driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "aps6408.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @defgroup APS6408 APS6408
  * @{
  */

/** @defgroup APS6408_Exported_Functions APS6408 Exported Functions
  * @{
  */

/* Read/Write Array Commands *********************/
/**
  * @brief  Reads an amount of data from the OSPI memory.
  * @param  Ctx Component object pointer
  * @param  pData Pointer to data to be read
  * @param  ReadAddr Read start address
  * @param  Size Size of data to read
  * @param  LatencyCode Latency used for the access
  * @param  BurstType Type of burst used for the access
  * @retval OSPI memory status
  */
int32_t APS6408_Read(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size, uint32_t LatencyCode,
                     uint32_t BurstType)
{
  OSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the read command */
  sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
  sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = (BurstType == 0U) ? APS6408_READ_LINEAR_BURST_CMD : APS6408_READ_CMD;
  sCommand.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address            = ReadAddr;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode           = HAL_OSPI_DATA_8_LINES;
  sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  sCommand.NbData             = Size;
  sCommand.DummyCycles        = LatencyCode;
  sCommand.DQSMode            = HAL_OSPI_DQS_ENABLE;
  sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_OSPI_Command(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  return APS6408_OK;
}

/**
  * @brief  Reads an amount of data in DMA mode from the OSPI memory.
  * @param  Ctx Component object pointer
  * @param  pData Pointer to data to be read
  * @param  ReadAddr Read start address
  * @param  Size Size of data to read
  * @param  LatencyCode Latency used for the access
  * @param  BurstType Type of burst used for the access
  * @retval OSPI memory status
  */
int32_t APS6408_Read_DMA(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size,
                         uint32_t LatencyCode, uint32_t BurstType)
{
  OSPI_RegularCmdTypeDef sCommand;

  /* Initialize the read command */
  sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
  sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = ((BurstType == 0U) ? APS6408_READ_LINEAR_BURST_CMD : APS6408_READ_CMD);
  sCommand.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address            = ReadAddr;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode           = HAL_OSPI_DATA_8_LINES;
  sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  sCommand.NbData             = Size;
  sCommand.DummyCycles        = LatencyCode;
  sCommand.DQSMode            = HAL_OSPI_DQS_ENABLE;
  sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_OSPI_Command(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive_DMA(Ctx, pData) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  return APS6408_OK;
}

/**
  * @brief  Writes an amount of data to the OSPI memory.
  * @param  Ctx Component object pointer
  * @param  pData Pointer to data to be written
  * @param  WriteAddr Write start address
  * @param  Size Size of data to write
  * @param  LatencyCode Latency used for the access
  * @param  BurstType Type of burst used for the access
  * @retval OSPI memory status
  */
int32_t APS6408_Write(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t WriteAddr, uint32_t Size, uint32_t LatencyCode,
                      uint32_t BurstType)
{
  OSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the write command */
  sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
  sCommand.Instruction        = (BurstType == 0U) ? APS6408_WRITE_LINEAR_BURST_CMD : APS6408_WRITE_CMD;
  sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Address            = WriteAddr;
  sCommand.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode           = HAL_OSPI_DATA_8_LINES;
  sCommand.NbData             = Size;
  sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  sCommand.DummyCycles        = LatencyCode;
  sCommand.DQSMode            = HAL_OSPI_DQS_ENABLE;
  sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_OSPI_Command(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  /* Transmission of the data */
  if (HAL_OSPI_Transmit(Ctx, pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  return APS6408_OK;
}

/**
  * @brief  Writes an amount of data in DMA mode to the OSPI memory.
  * @param  Ctx Component object pointer
  * @param  pData Pointer to data to be written
  * @param  WriteAddr Write start address
  * @param  Size Size of data to write
  * @param  LatencyCode Latency used for the access
  * @param  BurstType Type of burst used for the access
  * @retval OSPI memory status
  */
int32_t APS6408_Write_DMA(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t WriteAddr, uint32_t Size,
                          uint32_t LatencyCode, uint32_t BurstType)
{
  OSPI_RegularCmdTypeDef sCommand;

  /* Initialize the write command */
  sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
  sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = ((BurstType == 0U) ? APS6408_WRITE_LINEAR_BURST_CMD : APS6408_WRITE_CMD);
  sCommand.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address            = WriteAddr;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode           = HAL_OSPI_DATA_8_LINES;
  sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  sCommand.NbData             = Size;
  sCommand.DummyCycles        = (LatencyCode - 1U);
  sCommand.DQSMode            = HAL_OSPI_DQS_ENABLE;
  sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_OSPI_Command(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  /* Transmission of the data */
  if (HAL_OSPI_Transmit_DMA(Ctx, pData) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  return APS6408_OK;
}

/**
  * @brief  Enable memory mapped mode for the OSPI memory.
  * @param  Ctx Component object pointer
  * @retval OSPI memory status
  */
int32_t APS6408_EnableMemoryMappedMode(OSPI_HandleTypeDef *Ctx, uint32_t ReadLatencyCode, uint32_t WriteLatencyCode,
                                       uint32_t BurstType)
{
  OSPI_RegularCmdTypeDef   sCommand;
  OSPI_MemoryMappedTypeDef sMemMappedCfg;

  /* Initialize the write command */
  sCommand.OperationType      = HAL_OSPI_OPTYPE_WRITE_CFG;
  sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
  sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = (BurstType == 0U) ? APS6408_WRITE_LINEAR_BURST_CMD : APS6408_WRITE_CMD;
  sCommand.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode           = HAL_OSPI_DATA_8_LINES;
  sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  sCommand.DummyCycles        = WriteLatencyCode;
  sCommand.DQSMode            = HAL_OSPI_DQS_ENABLE;
  sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  if (HAL_OSPI_Command(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  /* Initialize the read command */
  sCommand.OperationType = HAL_OSPI_OPTYPE_READ_CFG;
  sCommand.Instruction   = APS6408_READ_CMD;
  sCommand.DummyCycles   = ReadLatencyCode;

  if (HAL_OSPI_Command(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  /* OctoSPI activation of memory-mapped mode */
  sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_ENABLE;
  sMemMappedCfg.TimeOutPeriod     = 0x34U;

  if (HAL_OSPI_MemoryMapped(Ctx, &sMemMappedCfg) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  return APS6408_OK;
}

/* Register/Setting Commands **************************************************/
/**
  * @brief  Read mode register value
  * @param  Ctx Component object pointer
  * @param  Address Register address
  * @param  Value Register value pointer
  * @param  LatencyCode Latency used for the access
  * @retval error status
  */
int32_t APS6408_ReadReg(OSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *Value, uint32_t LatencyCode)
{
  OSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the read register command */
  sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = APS6408_READ_REG_CMD;
  sCommand.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address            = Address;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode           = HAL_OSPI_DATA_8_LINES;
  sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  sCommand.NbData             = 2;
  sCommand.DummyCycles        = LatencyCode;
  sCommand.DQSMode            = HAL_OSPI_DQS_ENABLE;
  sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_OSPI_Command(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(Ctx, (uint8_t *)Value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  return APS6408_OK;
}

/**
  * @brief  Write mode register
  * @param  Ctx Component object pointer
  * @param  Address Register address
  * @param  Value Value to write to register
  * @retval error status
  */
int32_t APS6408_WriteReg(OSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t Value)
{
  OSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the write register command */
  sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = APS6408_WRITE_REG_CMD;
  sCommand.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address            = Address;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode           = HAL_OSPI_DATA_8_LINES;
  sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  sCommand.NbData             = 2;
  sCommand.DummyCycles        = 0;
  sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_OSPI_Command(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  /* Transmission of the data */
  if (HAL_OSPI_Transmit(Ctx, (uint8_t *)(&Value), HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  return APS6408_OK;
}

/* ID Commands ****************************************************************/
/**
  * @brief  Read Flash IDs.
  *         Vendor ID, Device ID, Device Density
  * @param  Ctx Component object pointer
  * @param  ID IDs pointer (2 * 8-bits value)
  * @param  LatencyCode Latency used for the access
  * @retval error status
  */
int32_t APS6408_ReadID(OSPI_HandleTypeDef *Ctx, uint8_t *ID, uint32_t LatencyCode)
{
  /* Read the Mode Register 1 and 2 */
  if (APS6408_ReadReg(Ctx, APS6408_MR1_ADDRESS, ID, LatencyCode) != APS6408_OK)
  {
    return APS6408_ERROR;
  }

  /* Keep only Vendor ID from Mode Register 1 */
  *ID &= (APS6408_MR1_VENDOR_ID);

  /* Keep only Device ID and Device Density from Mode Register 2 */
  *(ID + 1) &= (APS6408_MR2_DEVICE_ID | APS6408_MR2_DENSITY);

  return APS6408_OK;
}

/* Power down Commands ********************************************************/
/**
  * @brief  Memory enter deep power-down command
  * @param  Ctx Component object pointer
  * @retval error status
  */
int32_t APS6408_EnterPowerDown(OSPI_HandleTypeDef *Ctx)
{
  /* Update the deep power down value of the MR6 register */
  if (APS6408_WriteReg(Ctx, APS6408_MR6_ADDRESS, APS6408_MR6_HS_DEEP_POWER_DOWN) != APS6408_OK)
  {
    return APS6408_ERROR;
  }

  /* ---         Memory enter deep power down as soon nCS goes high        --- */
  /* --- At least 500us should be respected before leaving deep power down --- */

  return APS6408_OK;
}

/**
  * @brief  Flash leave deep power-down command
  * @param  Ctx Component object pointer
  * @retval error status
  */
int32_t APS6408_LeavePowerDown(OSPI_HandleTypeDef *Ctx)
{
  /* --- A dummy command is sent to the memory, as the nCS should be low for at least 60 ns  --- */
  /* ---                  Memory takes 150us max to leave deep power down                    --- */

  OSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the command */
  sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
  sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = APS6408_READ_CMD;
  sCommand.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  sCommand.Address            = 0;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode           = HAL_OSPI_DATA_NONE;
  sCommand.NbData             = 0;
  sCommand.DummyCycles        = 0;
  sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_OSPI_Command(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  return APS6408_OK;
}

/* Reset Command **************************************************************/
/**
  * @brief  Reset the memory
  * @param  Ctx Component object pointer
  * @retval error status
  */
int32_t APS6408_Reset(OSPI_HandleTypeDef *Ctx)
{
  OSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the command */
  sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
  sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = APS6408_RESET_CMD;
  sCommand.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  sCommand.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  sCommand.Address            = 0;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode           = HAL_OSPI_DATA_NONE;
  sCommand.NbData             = 0;
  sCommand.DummyCycles        = 0;
  sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_OSPI_Command(Ctx, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return APS6408_ERROR;
  }

  /* Need to wait tRST */
  HAL_Delay(1);

  return APS6408_OK;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
