/**
  ******************************************************************************
  * @file    sx8651_conf_template.h
  * @author  MCD Application Team
  * @brief   This file contains specific configuration for the
  *          sx8651.c that can be modified by user.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SX8651_CONF_H
#define SX8651_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Macros --------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define SX8651_AUTO_CALIBRATION_ENABLED      0U
#define SX8651_MAX_X_LENGTH                  4095U
#define SX8651_MAX_Y_LENGTH                  4095U

#ifdef __cplusplus
}
#endif
#endif /* SX8651_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
