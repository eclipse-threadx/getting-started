/*
 * Copyright 2017, 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_CSI_CAMERA_ADAPTER_H_
#define _FSL_CSI_CAMERA_ADAPTER_H_

#include "fsl_common.h"
#include "fsl_csi.h"

/*
 * Change log:
 *
 *   1.0.2
 *     - Fixed MISRA-C 2012 issues.
 *
 *   1.0.1
 *     - Add data bus width to csi_resource_t.
 *
 *   1.0.0
 *     - Initial version
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief The private data used by the CSI camera receiver. */
typedef struct _csi_private_data
{
    csi_handle_t csiHandle;              /*!< CSI driver handle. */
    camera_receiver_callback_t callback; /*!< Save the callback. */
    void *userData;                      /*!< Parameter for the callback. */
} csi_private_data_t;

/*!
 * @brief The resources used by the CSI camera receiver.
 *
 * Don't need to initialize the resource before CSI initialization.
 */
typedef struct _csi_resource
{
    CSI_Type *csiBase;      /*!< CSI register base. */
    csi_data_bus_t dataBus; /*!< Data bus width. */
} csi_resource_t;

/*! @brief CSI camera receiver operations structure. */
extern const camera_receiver_operations_t csi_ops;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_CSI_CAMERA_ADAPTER_H_ */
