/*
 * Copyright 2018, 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MEM_MANAGER_H__
#define __MEM_MANAGER_H__

#include "fsl_common.h"
/*!
 * @addtogroup MemManager
 * @{
 */

/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/
/*
 * @brief Configures the memory manager light enable.
 */
#ifndef gMemManagerLight
#define gMemManagerLight (1)
#endif

/*
 * @brief Configures the memory manager trace debug enable.
 */
#ifndef MEM_MANAGER_ENABLE_TRACE
#define MEM_MANAGER_ENABLE_TRACE (0)
#endif

/*
 * @brief Configures the memory manager remove memory buffer.
 */
#ifndef MEM_MANAGER_BUFFER_REMOVE
#define MEM_MANAGER_BUFFER_REMOVE (0)
#endif

/*
 * @brief Configures the memory manager pre configure.
 */
#ifndef MEM_MANAGER_PRE_CONFIGURE
#define MEM_MANAGER_PRE_CONFIGURE (1)
#endif

#if (defined(MEM_MANAGER_ENABLE_TRACE) && (MEM_MANAGER_ENABLE_TRACE > 0U))
#ifndef MEM_POOL_SIZE
#define MEM_POOL_SIZE (32U)
#endif
#ifndef MEM_BLOCK_SIZE
#define MEM_BLOCK_SIZE (12U)
#endif
#else
#ifndef MEM_POOL_SIZE
#define MEM_POOL_SIZE (20U)
#endif
#ifndef MEM_BLOCK_SIZE
#define MEM_BLOCK_SIZE (4U)
#endif
#endif

#define MAX_POOL_ID 3U

/* Debug Macros - stub if not defined */
#ifndef MEM_DBG_LOG
#define MEM_DBG_LOG(...)
#endif

/* Default memory allocator */
#ifndef MEM_BufferAlloc
#define MEM_BufferAlloc(numBytes) MEM_BufferAllocWithId(numBytes, 0)
#endif

#if (defined(MEM_MANAGER_PRE_CONFIGURE) && (MEM_MANAGER_PRE_CONFIGURE > 0U))
/*
 * Defines pools by block size and number of blocks. Must be aligned to 4 bytes.
 * Defines block as  (blockSize ,numberOfBlocks,  id), id must be keep here,
 * even id is 0, will be _block_set_(64, 8, 0) _eol_
 * and _block_set_(64, 8) _eol_\ could not supported
 */
#ifndef PoolsDetails_c
#define PoolsDetails_c _block_set_(64, 8, 0) _eol_ _block_set_(128, 2, 1) _eol_ _block_set_(256, 6, 1) _eol_
#endif /* PoolsDetails_c */

#define MEM_BLOCK_DATA_BUFFER_NONAME_DEFINE(blockSize, numberOfBlocks, id)                                        \
    uint32_t g_poolBuffer##blockSize##_##numberOfBlocks##_##id[(MEM_POOL_SIZE + (numberOfBlocks)*MEM_BLOCK_SIZE + \
                                                                ((numberOfBlocks) * (blockSize)) + 3U) >>         \
                                                               2U];

#define MEM_BLOCK_BUFFER_NONAME_DEFINE(blockSize, numberOfBlocks, id)                   \
    MEM_BLOCK_DATA_BUFFER_NONAME_DEFINE(blockSize, numberOfBlocks, id)                  \
    const static mem_config_t g_poolHeadBuffer##blockSize##_##numberOfBlocks##_##id = { \
        (blockSize), (numberOfBlocks), (id), (0), (uint8_t *)&g_poolBuffer####blockSize##_##numberOfBlocks##_##id[0]}
#define MEM_BLOCK_NONAME_BUFFER(blockSize, numberOfBlocks, id) \
    (uint8_t *)&g_poolHeadBuffer##blockSize##_##numberOfBlocks##_##id
#endif /* MEM_MANAGER_PRE_CONFIGURE */

/*!
 * @brief Defines the memory buffer
 *
 * This macro is used to define the shell memory buffer for memory manager.
 * And then uses the macro MEM_BLOCK_BUFFER to get the memory buffer pointer.
 * The macro should not be used in any function.
 *
 * This is a example,
 * @code
 * MEM_BLOCK_BUFFER_DEFINE(app64, 5, 64,0);
 * MEM_BLOCK_BUFFER_DEFINE(app128, 6, 128,0);
 * MEM_BLOCK_BUFFER_DEFINE(app256, 7, 256,0);
 * @endcode
 *
 * @param name The name string of the memory buffer.
 * @param numberOfBlocks The number Of Blocks.
 * @param blockSize The memory block size.
 * @param id The id Of memory buffer.
 */
#define MEM_BLOCK_DATA_BUFFER_DEFINE(name, numberOfBlocks, blockSize, id) \
    uint32_t                                                              \
        g_poolBuffer##name[(MEM_POOL_SIZE + numberOfBlocks * MEM_BLOCK_SIZE + numberOfBlocks * blockSize + 3U) >> 2U];

#define MEM_BLOCK_BUFFER_DEFINE(name, numberOfBlocks, blockSize, id)  \
    MEM_BLOCK_DATA_BUFFER_DEFINE(name, numberOfBlocks, blockSize, id) \
    mem_config_t g_poolHeadBuffer##name = {(blockSize), (numberOfBlocks), (id), (0), (uint8_t *)&g_poolBuffer##name[0]}

/*!                                                                     \
 * @brief Gets the memory buffer pointer                                 \
 *                                                                       \
 * This macro is used to get the memory buffer pointer. The macro should \
 * not be used before the macro MEM_BLOCK_BUFFER_DEFINE is used.         \
 *                                                                       \
 * @param name The memory name string of the buffer.                     \
 */
#define MEM_BLOCK_BUFFER(name) (uint8_t *)&g_poolHeadBuffer##name

/*****************************************************************************
******************************************************************************
* Public type definitions
******************************************************************************
*****************************************************************************/

/**@brief Memory status. */
typedef enum _mem_status
{
    kStatus_MemSuccess      = kStatus_Success,                          /* No error occurred */
    kStatus_MemInitError    = MAKE_STATUS(kStatusGroup_MEM_MANAGER, 1), /* Memory initialization error */
    kStatus_MemAllocError   = MAKE_STATUS(kStatusGroup_MEM_MANAGER, 2), /* Memory allocation error */
    kStatus_MemFreeError    = MAKE_STATUS(kStatusGroup_MEM_MANAGER, 3), /* Memory free error */
    kStatus_MemUnknownError = MAKE_STATUS(kStatusGroup_MEM_MANAGER, 4), /* something bad has happened... */
} mem_status_t;

/**@brief Memory user config. */
typedef struct _mem_config
{
    uint16_t blockSize;      /*< The memory block size. */
    uint16_t numberOfBlocks; /*< The number Of Blocks. */
    uint16_t poolId;         /*< The pool id Of Blocks. */
    uint16_t reserved;       /*< reserved. */
    uint8_t *pbuffer;        /*< buffer. */
} mem_config_t;

/*****************************************************************************
******************************************************************************
* Public memory declarations
******************************************************************************
*****************************************************************************/
/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */
#if (defined(MEM_MANAGER_PRE_CONFIGURE) && (MEM_MANAGER_PRE_CONFIGURE > 0U))
/*!
 * @brief  Initialises the Memory Manager.
 *
 */
mem_status_t MEM_Init(void);

#endif

#if !defined(gMemManagerLight) || (gMemManagerLight == 0)
/*!
 * @brief Add memory buffer to memory manager buffer list.
 *
 * @note This API should be called when need add memory buffer to memory manager buffer list. First use
 * MEM_BLOCK_BUFFER_DEFINE to
 *        define memory buffer, then call MEM_AddBuffer function with MEM_BLOCK_BUFFER Macro as the input parameter.
 *  @code
 * MEM_BLOCK_BUFFER_DEFINE(app64, 5, 64,0);
 * MEM_BLOCK_BUFFER_DEFINE(app128, 6, 128,0);
 * MEM_BLOCK_BUFFER_DEFINE(app256, 7, 256,0);
 *
 * MEM_AddBuffer(MEM_BLOCK_BUFFER(app64));
 * MEM_AddBuffer(MEM_BLOCK_BUFFER(app128));
 * MEM_AddBuffer(MEM_BLOCK_BUFFER(app256));
 * @endcode
 *
 * @param buffer                     Pointer the memory pool buffer, use MEM_BLOCK_BUFFER Macro as the input parameter.
 *
 * @retval kStatus_MemSuccess        Memory manager add buffer succeed.
 * @retval kStatus_MemUnknownError   Memory manager add buffer error occurred.
 */
mem_status_t MEM_AddBuffer(const uint8_t *buffer);
#endif /* gMemManagerLight */

#if !defined(gMemManagerLight) || (gMemManagerLight == 0)
#if (defined(MEM_MANAGER_BUFFER_REMOVE) && (MEM_MANAGER_BUFFER_REMOVE > 0U))
/*!
 * @brief Remove memory buffer from memory manager buffer list.
 *
 * @note This API should be called when need remove memory buffer from memory manager buffer list. Use MEM_BLOCK_BUFFER
 * Macro as the input parameter.
 *
 * @param buffer                     Pointer the memory pool buffer, use MEM_BLOCK_BUFFER Macro as the input parameter.
 *
 * @retval kStatus_MemSuccess        Memory manager remove buffer succeed.
 * @retval kStatus_MemUnknownError    Memory manager remove buffer error occurred.
 */
mem_status_t MEM_RemoveBuffer(uint8_t *buffer);
#endif /* MEM_MANAGER_BUFFER_REMOVE */
#endif /* gMemManagerLight */
/*!
 * @brief Allocate a block from the memory pools. The function uses the
 *        numBytes argument to look up a pool with adequate block sizes.
 *
 * @param numBytes           The number of bytes will be allocated.
 * @param poolId             The ID of the pool where to search for a free buffer.
 * @retval Memory buffer address when allocate success, NULL when allocate fail.
 */
void *MEM_BufferAllocWithId(uint32_t numBytes, uint8_t poolId);

/*!
 * @brief Memory buffer free .
 *
 * @param buffer                     The memory buffer address will be free.
 * @retval kStatus_MemSuccess        Memory free succeed.
 * @retval kStatus_MemFreeError      Memory free error occurred.
 */
mem_status_t MEM_BufferFree(void *buffer);

/*!
 * @brief Returns the size of a given buffer.
 *
 * @param buffer  The memory buffer address will be get size.
 * @retval The size of a given buffer.
 */
uint16_t MEM_BufferGetSize(void *buffer);

/*!
 * @brief Frees all allocated blocks by selected source and in selected pool.
 *
 * @param poolId                     Selected pool Id (4 LSBs of poolId parameter) and selected
 *                                   source Id (4 MSBs of poolId parameter).
 * @retval kStatus_MemSuccess        Memory free succeed.
 * @retval kStatus_MemFreeError      Memory free error occurred.
 */
mem_status_t MEM_BufferFreeAllWithId(uint8_t poolId);

/*!
 * @brief Memory buffer realloc.
 *
 * @param buffer                     The memory buffer address will be reallocated.
 * @param new_size                   The number of bytes will be reallocated
 * @retval kStatus_MemSuccess        Memory free succeed.
 * @retval kStatus_MemFreeError      Memory free error occurred.
 */
void *MEM_BufferRealloc(void *buffer, uint32_t new_size);

/*!
 * @brief Get the address after the last allocated block if MemManagerLight is used.
 *
 * @retval UpperLimit  Return the address after the last allocated block if MemManagerLight is used.
 * @retval 0           Return 0 in case of the legacy MemManager.
 */
uint32_t MEM_GetHeapUpperLimit(void);

#if !defined(gMemManagerLight) || (gMemManagerLight == 0)
#if (defined(MEM_MANAGER_ENABLE_TRACE) && (MEM_MANAGER_ENABLE_TRACE > 0U))
/*!
 * @brief Function to print statistics related to memory blocks managed by memory manager. Like bellow:
 * allocationFailures: 241  freeFailures:0
 * POOL: ID 0  status:
 * numBlocks allocatedBlocks    allocatedBlocksPeak  poolFragmentWaste poolFragmentWastePeak poolFragmentMinWaste
 * poolTotalFragmentWaste
 *     5            5                 5                  59                  63                       59 305
 * Currently pool meory block allocate status:
 * Block 0 Allocated    bytes: 1
 * Block 1 Allocated    bytes: 2
 * Block 2 Allocated    bytes: 3
 * Block 3 Allocated    bytes: 4
 * Block 4 Allocated    bytes: 5
 *
 * @details This API prints information with respects to each pool and block, including Allocated size,
 *          total block count, number of blocks in use at the time of printing, The API is intended to
 *          help developers tune the block sizes to make optimal use of memory for the application.
 *
 * @note This API should be disable by configure MEM_MANAGER_ENABLE_TRACE to 0
 *
 */
void MEM_Trace(void);

#endif /* MEM_MANAGER_ENABLE_TRACE */
#endif /* gMemManagerLight */

#if defined(gMemManagerLight) && (gMemManagerLight == 1)
void *MEM_CallocAlt(size_t len, size_t val);
#endif /*gMemManagerLight == 1*/

#if defined(__cplusplus)
}
#endif
/*! @}*/
#endif /* #ifndef __MEM_MANAGER_H__ */
