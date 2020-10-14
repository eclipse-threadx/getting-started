/*
 * Copyright  2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_codec_adapter.h"
#include "fsl_codec_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief module capability definition */
#define HAL_SGTL_MODULE_CAPABILITY                                                                                \
    kCODEC_SupportModuleADC | kCODEC_SupportModuleDAC | kCODEC_SupportModulePGA | kCODEC_SupportModuleHeadphone | \
        kCODEC_SupportModuleLinein | kCODEC_SupportModuleI2SIn | kCODEC_SupportModuleI2SOut | kCODEC_SupportModuleMic
#define HAL_SGTL_PLAY_CAPABILITY                                                                        \
    kCODEC_SupportPlayChannelLeft0 | kCODEC_SupportPlayChannelRight0 | kCODEC_SupportPlayChannelLeft1 | \
        kCODEC_SupportPlayChannelRight1 | kCODEC_SupportPlaySourceInput | kCODEC_SupportPlaySourceDAC | \
        kCODEC_SupportPlayChannelLeft2 | kCODEC_SupportPlayChannelRight2
#define HAL_SGTL_RECORD_CAPABILITY kCODEC_SupportRecordSourceLineInput | kCODEC_SupportRecordSourceSingleEndMic

/*! @brief sgtl map module */
#define HAL_SGTL_MAP_MODULE(module)                           \
    (module == kCODEC_ModuleADC ?                             \
         kSGTL_ModuleADC :                                    \
         module == kCODEC_ModuleDAC ?                         \
         kSGTL_ModuleDAC :                                    \
         module == kCODEC_ModuleHeadphone ?                   \
         kSGTL_ModuleHP :                                     \
         module == kCODEC_ModuleI2SIn ?                       \
         kSGTL_ModuleI2SIN :                                  \
         module == kCODEC_ModuleI2SOut ?                      \
         kSGTL_ModuleI2SOUT :                                 \
         module == kCODEC_ModuleLinein ? kSGTL_ModuleLineIn : \
                                         module == kCODEC_ModuleLineout ? kSGTL_ModuleLineOut : kSGTL_ModuleMicin)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const codec_capability_t s_sgtl5000_capability = {
    .codecPlayCapability   = HAL_SGTL_PLAY_CAPABILITY,
    .codecModuleCapability = HAL_SGTL_MODULE_CAPABILITY,
    .codecRecordCapability = HAL_SGTL_RECORD_CAPABILITY,
};
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * brief Codec initilization.
 *
 * param handle codec handle.
 * param config codec configuration.
 * return kStatus_Success is success, else initial failed.
 */
status_t HAL_CODEC_Init(void *handle, void *config)
{
    assert((config != NULL) && (handle != NULL));

    codec_config_t *codecConfig = (codec_config_t *)config;

    sgtl_config_t *sgtlConfig = (sgtl_config_t *)(codecConfig->codecDevConfig);
    sgtl_handle_t *sgtlHandle = (sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle));

    ((codec_handle_t *)handle)->codecCapability = &s_sgtl5000_capability;

    /* codec device initialization */
    return SGTL_Init(sgtlHandle, sgtlConfig);
}

/*!
 * brief Codec de-initilization.
 *
 * param handle codec handle.
 * return kStatus_Success is success, else de-initial failed.
 */
status_t HAL_CODEC_Deinit(void *handle)
{
    assert(handle != NULL);

    return SGTL_Deinit((sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle)));
}

/*!
 * brief set audio data format.
 *
 * param handle codec handle.
 * param mclk master clock frequency in HZ.
 * param sampleRate sample rate in HZ.
 * param bitWidth bit width.
 * return kStatus_Success is success, else configure failed.
 */
status_t HAL_CODEC_SetFormat(void *handle, uint32_t mclk, uint32_t sampleRate, uint32_t bitWidth)
{
    assert(handle != NULL);

    return SGTL_ConfigDataFormat((sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle)), mclk,
                                 sampleRate, bitWidth);
}

/*!
 * brief set audio codec module volume.
 *
 * param handle codec handle.
 * param channel audio codec play channel, can be a value or combine value of _codec_play_channel.
 * param volume volume value, support 0 ~ 100, 0 is mute, 100 is the maximum volume value.
 * return kStatus_Success is success, else configure failed.
 */
status_t HAL_CODEC_SetVolume(void *handle, uint32_t playChannel, uint32_t volume)
{
    assert(handle != NULL);

    if (playChannel & (kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight))
    {
        return SGTL_SetVolume((sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle)), kSGTL_ModuleHP,
                              volume);
    }

    if (playChannel & (kCODEC_PlayChannelLineOutLeft | kCODEC_PlayChannelLineOutRight))
    {
        return SGTL_SetVolume((sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle)),
                              kSGTL_ModuleLineOut, volume);
    }

    return kStatus_CODEC_NotSupport;
}

/*!
 * brief set audio codec module mute.
 *
 * param handle codec handle.
 * param channel audio codec play channel, can be a value or combine value of _codec_play_channel.
 * param isMute true is mute, false is unmute.
 * return kStatus_Success is success, else configure failed.
 */
status_t HAL_CODEC_SetMute(void *handle, uint32_t playChannel, bool isMute)
{
    assert(handle != NULL);

    status_t retVal = kStatus_Success;

    if (playChannel & (kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight))
    {
        retVal = SGTL_SetMute((sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle)), kSGTL_ModuleHP,
                              isMute);
    }

    if ((playChannel & (kCODEC_PlayChannelLineOutLeft | kCODEC_PlayChannelLineOutRight)) && (retVal == kStatus_Success))
    {
        retVal = SGTL_SetMute((sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle)),
                              kSGTL_ModuleLineOut, isMute);
    }

    return retVal;
}

/*!
 * brief set audio codec module power.
 *
 * param handle codec handle.
 * param module audio codec module.
 * param powerOn true is power on, false is power down.
 * return kStatus_Success is success, else configure failed.
 */
status_t HAL_CODEC_SetPower(void *handle, uint32_t module, bool powerOn)
{
    assert(handle != NULL);

    if (powerOn)
    {
        return SGTL_EnableModule((sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle)),
                                 HAL_SGTL_MAP_MODULE(module));
    }
    else
    {
        return SGTL_DisableModule((sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle)),
                                  HAL_SGTL_MAP_MODULE(module));
    }
}

/*!
 * brief codec set record source.
 *
 * param handle codec handle.
 * param source audio codec record source, can be a value or combine value of _codec_record_source.
 *
 * return kStatus_Success is success, else configure failed.
 */
status_t HAL_CODEC_SetRecord(void *handle, uint32_t recordSource)
{
    assert(handle != NULL);

    if (recordSource == kCODEC_RecordSourceLineInput)
    {
        return SGTL_SetRecord((sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle)),
                              kSGTL_RecordSourceLineIn);
    }

    if (recordSource == kCODEC_RecordSourceSingleEndMic)
    {
        return SGTL_SetRecord((sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle)),
                              kSGTL_RecordSourceMic);
    }

    return kStatus_CODEC_NotSupport;
}

/*!
 * brief codec set record channel.
 *
 * param handle codec handle.
 * param leftRecordChannel audio codec record channel, reference _codec_record_channel, can be a value or combine value
 of member in _codec_record_channel.
 * param rightRecordChannel audio codec record channel, reference _codec_record_channel, can be a value combine of
 member in _codec_record_channel.

 * return kStatus_Success is success, else configure failed.
 */
status_t HAL_CODEC_SetRecordChannel(void *handle, uint32_t leftRecordChannel, uint32_t rightRecordChannel)
{
    return kStatus_CODEC_NotSupport;
}

/*!
 * brief codec set play source.
 *
 * param handle codec handle.
 * param playSource audio codec play source, can be a value or combine value of _codec_play_source.
 *
 * return kStatus_Success is success, else configure failed.
 */
status_t HAL_CODEC_SetPlay(void *handle, uint32_t playSource)
{
    assert(handle != NULL);

    if (playSource == kCODEC_PlaySourceInput)
    {
        return SGTL_SetPlay((sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle)),
                            kSGTL_PlaySourceLineIn);
    }

    if (playSource == kCODEC_PlaySourceDAC)
    {
        return SGTL_SetPlay((sgtl_handle_t *)((uint32_t)(((codec_handle_t *)handle)->codecDevHandle)),
                            kSGTL_PlaySourceDAC);
    }

    return kStatus_CODEC_NotSupport;
}

/*!
 * brief codec module control.
 *
 * param handle codec handle.
 * param cmd module control cmd, reference _codec_module_ctrl_cmd.
 * param data value to write, when cmd is kCODEC_ModuleRecordSourceChannel, the data should be a value combine
 *  of channel and source, please reference macro CODEC_MODULE_RECORD_SOURCE_CHANNEL(source, LP, LN, RP, RN), reference
 *  codec specific driver for detail configurations.
 * return kStatus_Success is success, else configure failed.
 */
status_t HAL_CODEC_ModuleControl(void *handle, uint32_t cmd, uint32_t data)
{
    return kStatus_CODEC_NotSupport;
}
