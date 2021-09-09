/*
 * Copyright (c) 2013-2016 ARM Limited. All rights reserved.
 * Copyright (c) 2016, Freescale Semiconductor, Inc. Not a Contribution.
 * Copyright 2016-2017,2020 NXP. Not a Contribution.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FSL_LPI2C_CMSIS_H_
#define _FSL_LPI2C_CMSIS_H_

#include "fsl_common.h"
#include "Driver_I2C.h"
#include "RTE_Device.h"
#include "fsl_lpi2c.h"
#if (defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT)
#include "fsl_dmamux.h"
#endif
#if (defined(FSL_FEATURE_SOC_DMA_COUNT) && FSL_FEATURE_SOC_DMA_COUNT)
#include "fsl_lpi2c_dma.h"
#endif
#if (defined(FSL_FEATURE_SOC_EDMA_COUNT) && FSL_FEATURE_SOC_EDMA_COUNT)
#include "fsl_lpi2c_edma.h"
#endif

#if defined(LPI2C0) && defined(RTE_I2C0) && RTE_I2C0
extern ARM_DRIVER_I2C Driver_I2C0;
#endif /* LPI2C0 */

#if defined(LPI2C1) && defined(RTE_I2C1) && RTE_I2C1
extern ARM_DRIVER_I2C Driver_I2C1;
#endif /* LPI2C1 */

#if defined(LPI2C2) && defined(RTE_I2C2) && RTE_I2C2
extern ARM_DRIVER_I2C Driver_I2C2;
#endif /* LPI2C2 */

#if defined(LPI2C3) && defined(RTE_I2C3) && RTE_I2C3
extern ARM_DRIVER_I2C Driver_I2C3;
#endif /* LPI2C3 */

#if defined(LPI2C4) && defined(RTE_I2C4) && RTE_I2C4
extern ARM_DRIVER_I2C Driver_I2C4;
#endif /* LPI2C4 */

#if defined(LPI2C5) && defined(RTE_I2C5) && RTE_I2C5
extern ARM_DRIVER_I2C Driver_I2C5;
#endif /* LPI2C5*/

#if defined(LPI2C6) && defined(RTE_I2C6) && RTE_I2C6
extern ARM_DRIVER_I2C Driver_I2C6;
#endif /* LPI2C6 */

/* I2C Driver state flags */
#define I2C_FLAG_UNINIT (0UL)
#define I2C_FLAG_INIT   (1UL << 0)
#define I2C_FLAG_POWER  (1UL << 1)

#endif /* _FSL_LPI2C_CMSIS_H_ */
