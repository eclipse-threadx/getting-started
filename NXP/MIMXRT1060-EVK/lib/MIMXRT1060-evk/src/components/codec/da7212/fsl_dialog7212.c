/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_dialog7212.h"

/*******************************************************************************
 * Definitations
 ******************************************************************************/
/*! @brief da7212 reigster structure */
typedef struct _da7212_register_value
{
    uint8_t addr;
    uint8_t value;
} da7212_register_value_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const da7212_register_value_t kInputRegisterSequence[kDA7212_Input_MAX][17] = {
    /* DA7212_Input_AUX */
    {
        {DIALOG7212_MIXIN_L_SELECT, 0x01},
        {DIALOG7212_MIXIN_R_SELECT, 0x01},
        {DIALOG7212_CP_CTRL, 0xFD},
        {DIALOG7212_AUX_L_CTRL, 0xB4},
        {DIALOG7212_AUX_R_CTRL, 0xB0},
        {DIALOG7212_MIC_1_CTRL, 0x04},
        {DIALOG7212_MIC_2_CTRL, 0x04},
        {DIALOG7212_MIXIN_L_CTRL, 0x88},
        {DIALOG7212_MIXIN_R_CTRL, 0x88},
        {DIALOG7212_ADC_L_CTRL, 0xA0},
        {DIALOG7212_GAIN_RAMP_CTRL, 0x02},
        {DIALOG7212_PC_COUNT, 0x02},
        {DIALOG7212_CP_DELAY, 0x95},
    },
    /* DA7212_Input_MIC1_Dig */
    {
        {DIALOG7212_MICBIAS_CTRL, 0xA9},
        {DIALOG7212_CP_CTRL, 0xF1},
        {DIALOG7212_MIXIN_L_SELECT, 0x80},
        {DIALOG7212_MIXIN_R_SELECT, 0x80},
        {DIALOG7212_SYSTEM_MODES_INPUT, 0xFE},
        {DIALOG7212_SYSTEM_MODES_OUTPUT, 0xF7},
        {DIALOG7212_MIC_CONFIG, 0x07},
        {DIALOG7212_MIC_2_GAIN, 0x04},
        {DIALOG7212_MIC_2_CTRL, 0x84},
        {DIALOG7212_MIC_1_GAIN, 0x01},
        {DIALOG7212_MIC_1_CTRL, 0x80},
        {DIALOG7212_ADC_FILTERS1, 0x08},
    },
    /* DA7212_Input_MIC1_An */
    {
        {DIALOG7212_MIXIN_L_SELECT, 0x02},
        {DIALOG7212_MIXIN_R_SELECT, 0x04},
        {DIALOG7212_MIC_1_GAIN, 0x03},
        {DIALOG7212_CP_CTRL, 0xFD},
        {DIALOG7212_MIXOUT_L_SELECT, 0x08},
        {DIALOG7212_MIXOUT_R_SELECT, 0x08},
        {DIALOG7212_AUX_R_CTRL, 0x40},
        {DIALOG7212_MICBIAS_CTRL, 0x19},
        {DIALOG7212_MIC_1_CTRL, 0x84},
        {DIALOG7212_MIC_2_CTRL, 0x04},
        {DIALOG7212_MIXIN_L_CTRL, 0x88},
        {DIALOG7212_MIXIN_R_CTRL, 0x88},
        {DIALOG7212_ADC_L_CTRL, 0xA0},
        {DIALOG7212_GAIN_RAMP_CTRL, 0x02},
        {DIALOG7212_PC_COUNT, 0x02},
        {DIALOG7212_CP_DELAY, 0x95},
    },
    /* DA7212_Input_MIC2 */
    {
        {DIALOG7212_MIXIN_L_SELECT, 0x04},
        {DIALOG7212_MIXIN_R_SELECT, 0x02},
        {DIALOG7212_MIC_2_GAIN, 0x04},
        {DIALOG7212_CP_CTRL, 0xFD},
        {DIALOG7212_AUX_R_CTRL, 0x40},
        {DIALOG7212_MICBIAS_CTRL, 0x91},
        {DIALOG7212_MIC_1_CTRL, 0x08},
        {DIALOG7212_MIC_2_CTRL, 0x84},
        {DIALOG7212_MIXIN_L_CTRL, 0x88},
        {DIALOG7212_MIXIN_R_CTRL, 0x88},
        {DIALOG7212_ADC_L_CTRL, 0xA0},
        {DIALOG7212_GAIN_RAMP_CTRL, 0x02},
        {DIALOG7212_PC_COUNT, 0x02},
        {DIALOG7212_CP_DELAY, 0x95},
    }};

static const da7212_register_value_t kOutputRegisterSequence[kDA7212_Output_MAX][4] = {
    /* DA7212_Output_HP */
    {
        {DIALOG7212_CP_CTRL, 0xF9},
        {DIALOG7212_LINE_CTRL, 0},
        {DIALOG7212_HP_L_CTRL, (DIALOG7212_HP_L_CTRL_AMP_EN_MASK | DIALOG7212_HP_L_CTRL_AMP_RAMP_EN_MASK |
                                DIALOG7212_HP_L_CTRL_AMP_ZC_EN_MASK | DIALOG7212_HP_L_CTRL_AMP_OE_MASK)},
        {DIALOG7212_HP_R_CTRL, (DIALOG7212_HP_R_CTRL_AMP_EN_MASK | DIALOG7212_HP_R_CTRL_AMP_RAMP_EN_MASK |
                                DIALOG7212_HP_R_CTRL_AMP_ZC_EN_MASK | DIALOG7212_HP_R_CTRL_AMP_OE_MASK)},
    },
    /* DA7212_Output_SP */
    {
        {DIALOG7212_CP_CTRL, 0x3D},
        {DIALOG7212_HP_L_CTRL, 0x40},
        {DIALOG7212_HP_R_CTRL, 0x40},
        {DIALOG7212_LINE_CTRL, 0xA8},
    }};

static const da7212_register_value_t kInitRegisterSequence[DA7212_INIT_SIZE] = {
    {
        DIALOG7212_CIF_CTRL,
        0x80,
    },
    {
        DIALOG7212_DIG_ROUTING_DAI,
        0x10,
    },
    {
        DIALOG7212_SR,
        DIALOG7212_SR_16KHZ,
    },
    {
        DIALOG7212_REFERENCES,
        DIALOG7212_REFERENCES_BIAS_EN_MASK,
    },
    {
        DIALOG7212_PLL_FRAC_TOP,
        CLEAR_REGISTER,
    },
    {
        DIALOG7212_PLL_FRAC_BOT,
        CLEAR_REGISTER,
    },
    {
        DIALOG7212_PLL_INTEGER,
        0x20,
    },
    {
        DIALOG7212_PLL_CTRL,
        0U,
    },
    {
        DIALOG7212_DAI_CLK_MODE,
        (DIALOG7212_DAI_BCLKS_PER_WCLK_BCLK64),
    },
    {
        DIALOG7212_DAI_CTRL,
        (DIALOG7212_DAI_EN_MASK | DIALOG7212_DAI_OE_MASK | DIALOG7212_DAI_WORD_LENGTH_16B |
         DIALOG7212_DAI_FORMAT_I2S_MODE),
    },
    {
        DIALOG7212_DIG_ROUTING_DAC,
        (DIALOG7212_DIG_ROUTING_DAC_R_RSC_DAC_R | DIALOG7212_DIG_ROUTING_DAC_L_RSC_DAC_L),
    },
    {
        DIALOG7212_CP_CTRL,
        (DIALOG7212_CP_CTRL_EN_MASK | DIALOG7212_CP_CTRL_SMALL_SWIT_CH_FREQ_EN_MASK |
         DIALOG7212_CP_CTRL_MCHANGE_OUTPUT | DIALOG7212_CP_CTRL_MOD_CPVDD_1 |
         DIALOG7212_CP_CTRL_ANALOG_VLL_LV_BOOSTS_CP),
    },
    {
        DIALOG7212_MIXOUT_L_SELECT,
        (DIALOG7212_MIXOUT_L_SELECT_DAC_L_MASK),
    },
    {
        DIALOG7212_MIXOUT_R_SELECT,
        (DIALOG7212_MIXOUT_R_SELECT_DAC_R_MASK),
    },
    {
        DIALOG7212_DAC_L_CTRL,
        (DIALOG7212_DAC_L_CTRL_ADC_EN_MASK | DIALOG7212_DAC_L_CTRL_ADC_RAMP_EN_MASK),
    },
    {
        DIALOG7212_DAC_R_CTRL,
        (DIALOG7212_DAC_R_CTRL_ADC_EN_MASK | DIALOG7212_DAC_R_CTRL_ADC_RAMP_EN_MASK),
    },
    {
        DIALOG7212_HP_L_CTRL,
        (DIALOG7212_HP_L_CTRL_AMP_EN_MASK | DIALOG7212_HP_L_CTRL_AMP_RAMP_EN_MASK |
         DIALOG7212_HP_L_CTRL_AMP_ZC_EN_MASK | DIALOG7212_HP_L_CTRL_AMP_OE_MASK),
    },
    {
        DIALOG7212_HP_R_CTRL,
        (DIALOG7212_HP_R_CTRL_AMP_EN_MASK | DIALOG7212_HP_R_CTRL_AMP_RAMP_EN_MASK |
         DIALOG7212_HP_R_CTRL_AMP_ZC_EN_MASK | DIALOG7212_HP_R_CTRL_AMP_OE_MASK),
    },
    {
        DIALOG7212_MIXOUT_L_CTRL,
        (DIALOG7212_MIXOUT_L_CTRL_AMP_EN_MASK | DIALOG7212_MIXOUT_L_CTRL_AMP_SOFT_MIX_EN_MASK |
         DIALOG7212_MIXOUT_L_CTRL_AMP_MIX_EN_MASK),
    },
    {
        DIALOG7212_MIXOUT_R_CTRL,
        (DIALOG7212_MIXOUT_R_CTRL_AMP_EN_MASK | DIALOG7212_MIXOUT_R_CTRL_AMP_SOFT_MIX_EN_MASK |
         DIALOG7212_MIXOUT_R_CTRL_AMP_MIX_EN_MASK),
    },
    {
        DIALOG7212_CP_VOL_THRESHOLD1,
        (DIALOG7212_CP_VOL_THRESHOLD1_VDD2(0x32)),
    },
    {
        DIALOG7212_SYSTEM_STATUS,
        CLEAR_REGISTER,
    },
    {
        DIALOG7212_DAC_L_GAIN,
        kDA7212_DACGainM6DB,
    },
    {
        DIALOG7212_DAC_R_GAIN,
        kDA7212_DACGainM6DB,
    },
    {
        DIALOG7212_MIXIN_L_SELECT,
        DIALOG7212_MIXIN_L_SELECT_AUX_L_SEL_MASK,
    },
    {
        DIALOG7212_MIXIN_R_SELECT,
        DIALOG7212_MIXIN_R_SELECT_AUX_R_SEL_MASK,
    },
    {
        DIALOG7212_MIXIN_L_GAIN,
        DIALOG7212_MIXIN_L_AMP_GAIN(0x03),
    },
    {
        DIALOG7212_MIXIN_R_GAIN,
        DIALOG7212_MIXIN_R_AMP_GAIN(0x03),
    },
    {
        DIALOG7212_ADC_L_GAIN,
        DIALOG7212_ADC_L_DIGITAL_GAIN(0x6F),
    },
    {
        DIALOG7212_ADC_R_GAIN,
        DIALOG7212_ADC_R_DIGITAL_GAIN(0x6F),
    },
    {
        DIALOG7212_AUX_L_CTRL,
        DIALOG7212_AUX_L_CTRL_AMP_EN_MASK | DIALOG7212_AUX_L_CTRL_AMP_RAMP_EN_MASK |
            DIALOG7212_AUX_L_CTRL_AMP_ZC_EN_MASK,
    },
    {
        DIALOG7212_AUX_R_CTRL,
        DIALOG7212_AUX_R_CTRL_AMP_EN_MASK | DIALOG7212_AUX_R_CTRL_AMP_RAMP_EN_MASK |
            DIALOG7212_AUX_R_CTRL_AMP_ZC_EN_MASK,
    },
    {
        DIALOG7212_MIXIN_L_CTRL,
        DIALOG7212_MIXIN_L_CTRL_AMP_EN_MASK | DIALOG7212_MIXIN_L_CTRL_AMP_MIX_EN_MASK,
    },
    {
        DIALOG7212_MIXIN_R_CTRL,
        DIALOG7212_MIXIN_R_CTRL_AMP_EN_MASK | DIALOG7212_MIXIN_R_CTRL_AMP_MIX_EN_MASK,
    },
    {
        DIALOG7212_ADC_L_CTRL,
        DIALOG7212_ADC_L_CTRL_ADC_EN_MASK | DIALOG7212_ADC_L_CTRL_ADC_RAMP_EN_MASK,
    },
    {
        DIALOG7212_ADC_R_CTRL,
        DIALOG7212_ADC_R_CTRL_ADC_EN_MASK | DIALOG7212_ADC_R_CTRL_ADC_RAMP_EN_MASK,
    },
};

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t DA7212_WriteRegister(da7212_handle_t *handle, uint8_t u8Register, uint8_t u8RegisterData)
{
    assert(handle->config);
    assert(handle->config->slaveAddress != 0U);

    uint8_t writeValue = u8RegisterData;

    return CODEC_I2C_Send(handle->i2cHandle, handle->config->slaveAddress, u8Register, 1U, (uint8_t *)&writeValue, 1U);
}

status_t DA7212_ReadRegister(da7212_handle_t *handle, uint8_t u8Register, uint8_t *pu8RegisterData)
{
    assert(handle->config);
    assert(handle->config->slaveAddress != 0U);

    return CODEC_I2C_Receive(handle->i2cHandle, handle->config->slaveAddress, u8Register, 1U,
                             (uint8_t *)pu8RegisterData, 1U);
}

status_t DA7212_ModifyRegister(da7212_handle_t *handle, uint8_t reg, uint8_t mask, uint8_t value)
{
    status_t result;
    uint8_t regValue;

    result = DA7212_ReadRegister(handle, reg, &regValue);
    if (result != kStatus_Success)
    {
        return result;
    }

    regValue &= (uint8_t)~mask;
    regValue |= value;

    return DA7212_WriteRegister(handle, reg, regValue);
}

status_t DA7212_Init(da7212_handle_t *handle, da7212_config_t *codecConfig)
{
    assert(codecConfig != NULL);
    assert(handle != NULL);

    uint32_t i              = 0;
    da7212_config_t *config = codecConfig;
    uint32_t sysClock       = config->format.mclk_HZ;
    handle->config          = config;

    /* i2c bus initialization */
    if (CODEC_I2C_Init(handle->i2cHandle, config->i2cConfig.codecI2CInstance, DA7212_I2C_BAUDRATE,
                       config->i2cConfig.codecI2CSourceClock) != kStatus_HAL_I2cSuccess)
    {
        return kStatus_Fail;
    }

    /* If no config structure, use default settings */
    for (i = 0; i < DA7212_INIT_SIZE; i++)
    {
        DA7212_WriteRegister(handle, kInitRegisterSequence[i].addr, kInitRegisterSequence[i].value);
    }

    if (config->isMaster)
    {
        /* clock configurations */
        DA7212_WriteRegister(handle, DIALOG7212_DAI_CLK_MODE,
                             (config->format.isBclkInvert ? 1U << 2U : 0U) | (1U << 7U));

        DA7212_SetMasterModeBits(handle, config->format.bitWidth);
    }

    /* Set DA7212 functionality */
    if (config->dacSource == kDA7212_DACSourceADC)
    {
        DA7212_WriteRegister(handle, DIALOG7212_DIG_ROUTING_DAC, 0x10);
    }
    else
    {
        DA7212_WriteRegister(handle, DIALOG7212_DIG_ROUTING_DAC, 0x32);
    }

    /* Set the audio protocol */
    DA7212_WriteRegister(handle, DIALOG7212_DAI_CTRL, DIALOG7212_DAI_EN_MASK | config->protocol);

    if (codecConfig->sysClkSource == kDA7212_SysClkSourcePLL)
    {
        if (DA7212_SetPLLConfig(handle, codecConfig->pll) != kStatus_Success)
        {
            return kStatus_Fail;
        }

        sysClock = codecConfig->pll->outputClock_HZ;
    }

    DA7212_ConfigAudioFormat(handle, sysClock, config->format.sampleRate, config->format.bitWidth);

    return kStatus_Success;
}

status_t DA7212_SetPLLConfig(da7212_handle_t *handle, da7212_pll_config_t *config)
{
    assert(config != NULL);

    uint8_t indiv = 0, inputDiv = 0, regVal = 0;
    uint64_t pllValue = 0;
    uint32_t pllFractional;
    uint8_t pllInteger;
    uint8_t pllFracTop;
    uint8_t pllFracBottom;
    uint8_t pllEnMask = DIALOG7212_PLL_EN_MASK;

    if (config->refClock_HZ == 32768U)
    {
        pllEnMask |= DIALOG7212_PLL_SRM_EN_MASK | DIALOG7212_PLL_32K_MODE_MASK;
        indiv    = DIALOG7212_PLL_INDIV_2_10MHZ;
        inputDiv = 1;
    }
    /* Compute the PLL_INDIV and DIV value for sysClock */
    else if ((config->refClock_HZ > 2000000) && (config->refClock_HZ <= 10000000))
    {
        indiv    = DIALOG7212_PLL_INDIV_2_10MHZ;
        inputDiv = 2;
    }
    else if ((config->refClock_HZ > 10000000) && (config->refClock_HZ <= 20000000))
    {
        indiv    = DIALOG7212_PLL_INDIV_10_20MHZ;
        inputDiv = 4;
    }
    else if ((config->refClock_HZ > 20000000) && (config->refClock_HZ <= 40000000))
    {
        indiv    = DIALOG7212_PLL_INDIV_20_40MHZ;
        inputDiv = 8;
    }
    else
    {
        indiv    = DIALOG7212_PLL_INDIV_40_80MHZ;
        inputDiv = 16;
    }

    /* PLL feedback divider is a Q13 value */
    pllValue =
        (uint64_t)(((uint64_t)((((uint64_t)config->outputClock_HZ * 8) * inputDiv) << 13)) / (config->refClock_HZ));

    /* extract integer and fractional */
    pllInteger    = pllValue >> 13;
    pllFractional = (pllValue - (pllInteger << 13));
    pllFracTop    = (pllFractional >> 8);
    pllFracBottom = (pllFractional & 0xFF);

    DA7212_WriteRegister(handle, DIALOG7212_PLL_FRAC_TOP, pllFracTop);

    DA7212_WriteRegister(handle, DIALOG7212_PLL_FRAC_BOT, pllFracBottom);

    DA7212_WriteRegister(handle, DIALOG7212_PLL_INTEGER, pllInteger);

    regVal = pllEnMask | indiv;

    DA7212_WriteRegister(handle, DIALOG7212_PLL_CTRL, regVal);

    /* wait for PLL lock bits */
    while ((regVal & 1U) == 0U)
    {
        DA7212_ReadRegister(handle, DIALOG7212_PLL_STATUS, &regVal);
    }

    return kStatus_Success;
}

status_t DA7212_SetProtocol(da7212_handle_t *handle, da7212_protocol_t protocol)
{
    return DA7212_WriteRegister(handle, DIALOG7212_DAI_CTRL, (DIALOG7212_DAI_EN_MASK | protocol));
}

status_t DA7212_ConfigAudioFormat(da7212_handle_t *handle,
                                  uint32_t masterClock_Hz,
                                  uint32_t sampleRate_Hz,
                                  uint32_t dataBits)
{
    uint8_t regVal = 0;

    switch (sampleRate_Hz)
    {
        case 8000:
            DA7212_WriteRegister(handle, DIALOG7212_SR, DIALOG7212_SR_8KHZ);
            break;
        case 11025:
            DA7212_WriteRegister(handle, DIALOG7212_SR, DIALOG7212_SR_11_025KHZ);
            break;
        case 12000:
            DA7212_WriteRegister(handle, DIALOG7212_SR, DIALOG7212_SR_12KHZ);
            break;
        case 16000:
            DA7212_WriteRegister(handle, DIALOG7212_SR, DIALOG7212_SR_16KHZ);
            break;
        case 22050:
            DA7212_WriteRegister(handle, DIALOG7212_SR, DIALOG7212_SR_22KHZ);
            break;
        case 24000:
            DA7212_WriteRegister(handle, DIALOG7212_SR, DIALOG7212_SR_24KHZ);
            break;
        case 32000:
            DA7212_WriteRegister(handle, DIALOG7212_SR, DIALOG7212_SR_32KHZ);
            break;
        case 44100:
            DA7212_WriteRegister(handle, DIALOG7212_SR, DIALOG7212_SR_44_1KHZ);
            break;
        case 48000:
            DA7212_WriteRegister(handle, DIALOG7212_SR, DIALOG7212_SR_48KHZ);
            break;
        case 88200:
            DA7212_WriteRegister(handle, DIALOG7212_SR, DIALOG7212_SR_88_2KHZ);
            break;
        case 96000:
            DA7212_WriteRegister(handle, DIALOG7212_SR, DIALOG7212_SR_96KHZ);
            break;
        default:
            break;
    }

    /* Set data bits of word */
    DA7212_ReadRegister(handle, DIALOG7212_DAI_CTRL, &regVal);
    regVal &= ~DIALOG7212_DAI_WORD_LENGTH_MASK;
    switch (dataBits)
    {
        case 16:
            regVal |= DIALOG7212_DAI_WORD_LENGTH_16B;
            break;
        case 20:
            regVal |= DIALOG7212_DAI_WORD_LENGTH_20B;
            break;
        case 24:
            regVal |= DIALOG7212_DAI_WORD_LENGTH_24B;
            break;
        case 32:
            regVal |= DIALOG7212_DAI_WORD_LENGTH_32B;
            break;
        default:
            break;
    }
    DA7212_WriteRegister(handle, DIALOG7212_DAI_CTRL, regVal);

    return kStatus_Success;
}

status_t DA7212_SetMasterModeBits(da7212_handle_t *handle, uint32_t bitWidth)
{
    uint8_t regVal = 0U;

    switch (bitWidth)
    {
        case 16:
            regVal = 0U;
            break;
        case 32:
            regVal = 1U;
            break;
        case 64:
            regVal = 2U;
            break;
        case 128:
            regVal = 3U;
            break;
        default:
            assert(false);
    }

    return DA7212_ModifyRegister(handle, DIALOG7212_DAI_CLK_MODE, 3, regVal);
}

void DA7212_ChangeInput(da7212_handle_t *handle, da7212_Input_t DA7212_Input)
{
    uint32_t i       = 0;
    uint32_t seqSize = sizeof(kInputRegisterSequence[DA7212_Input]) / sizeof(da7212_register_value_t);

    for (i = 0; i < seqSize; i++)
    {
        DA7212_WriteRegister(handle, kInputRegisterSequence[DA7212_Input][i].addr,
                             kInputRegisterSequence[DA7212_Input][i].value);
    }
}

void DA7212_ChangeOutput(da7212_handle_t *handle, da7212_Output_t DA7212_Output)
{
    uint32_t i       = 0;
    uint32_t seqSize = sizeof(kOutputRegisterSequence[DA7212_Output]) / sizeof(da7212_register_value_t);

    for (i = 0; i < seqSize; i++)
    {
        DA7212_WriteRegister(handle, kOutputRegisterSequence[DA7212_Output][i].addr,
                             kOutputRegisterSequence[DA7212_Output][i].value);
    }
}

void DA7212_ChangeHPVolume(da7212_handle_t *handle, da7212_volume_t volume)
{
    DA7212_WriteRegister(handle, DIALOG7212_DAC_L_GAIN, volume);
    DA7212_WriteRegister(handle, DIALOG7212_DAC_R_GAIN, volume);
}

void DA7212_Mute(da7212_handle_t *handle, bool isMuted)
{
    uint8_t val = 0;

    if (isMuted)
    {
        val = DA7212_DAC_MUTE_ENABLED;
    }
    else
    {
        val = DA7212_DAC_MUTE_DISABLED;
    }

    DA7212_WriteRegister(handle, DIALOG7212_DAC_L_CTRL, val);
    DA7212_WriteRegister(handle, DIALOG7212_DAC_R_CTRL, val);
}

status_t DA7212_SetChannelVolume(da7212_handle_t *handle, uint32_t channel, uint32_t volume)
{
    status_t retVal   = kStatus_Success;
    uint16_t muteCtrl = volume == 0U ? 0x40 : 0x80U;
    uint32_t vol      = volume == 100U ? 64U : volume;

    if (channel & kDA7212_HeadphoneLeft)
    {
        retVal = DA7212_WriteRegister(handle, DIALOG7212_HP_L_GAIN, vol - 1U);
        retVal = DA7212_ModifyRegister(handle, DIALOG7212_HP_L_CTRL, 0xC0U, muteCtrl);
    }

    if (channel & kDA7212_HeadphoneRight)
    {
        retVal = DA7212_WriteRegister(handle, DIALOG7212_HP_R_GAIN, vol - 1U);
        retVal = DA7212_ModifyRegister(handle, DIALOG7212_HP_R_CTRL, 0xC0U, muteCtrl);
    }

    if (channel & kDA7212_Speaker)
    {
        retVal = DA7212_WriteRegister(handle, DIALOG7212_LINE_GAIN, vol - 1U);
        retVal = DA7212_ModifyRegister(handle, DIALOG7212_LINE_CTRL, 0xC0U, muteCtrl);
    }

    return retVal;
}

status_t DA7212_SetChannelMute(da7212_handle_t *handle, uint32_t channel, bool isMute)
{
    uint8_t regValue = isMute == true ? 0x40U : 0x80U;
    status_t retVal  = kStatus_Success;

    if (channel & kDA7212_HeadphoneLeft)
    {
        retVal = DA7212_ModifyRegister(handle, DIALOG7212_HP_L_CTRL, 0xC0U, regValue);
    }

    if (channel & kDA7212_HeadphoneRight)
    {
        retVal = DA7212_ModifyRegister(handle, DIALOG7212_HP_R_CTRL, 0xC0U, regValue);
    }

    if (channel & kDA7212_Speaker)
    {
        retVal = DA7212_ModifyRegister(handle, DIALOG7212_LINE_CTRL, 0xC0U, regValue);
    }

    return retVal;
}

status_t DA7212_Deinit(da7212_handle_t *handle)
{
    return kStatus_Success;
}
