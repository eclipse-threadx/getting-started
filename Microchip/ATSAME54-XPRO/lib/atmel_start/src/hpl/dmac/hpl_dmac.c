
/**
 * \file
 *
 * \brief Generic DMAC related functionality.
 *
 * Copyright (c) 2016-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
#include <hpl_dma.h>
#include <utils_assert.h>
#include <utils.h>
#include <hpl_dmac_config.h>
#include <utils_repeat_macro.h>

#if CONF_DMAC_ENABLE
/* Section containing first descriptors for all DMAC channels */
COMPILER_ALIGNED(16)
DmacDescriptor _descriptor_section[DMAC_CH_NUM];

/* Section containing current descriptors for all DMAC channels */
COMPILER_ALIGNED(16)
DmacDescriptor _write_back_section[DMAC_CH_NUM];

/* Array containing callbacks for DMAC channels */
static struct _dma_resource _resources[DMAC_CH_NUM];

/* DMAC interrupt handler */
static void _dmac_handler(void);

/* This macro DMAC configuration */
#define DMAC_CHANNEL_CFG(i, n)                                                                                         \
	{(CONF_DMAC_RUNSTDBY_##n << DMAC_CHCTRLA_RUNSTDBY_Pos) | DMAC_CHCTRLA_TRIGACT(CONF_DMAC_TRIGACT_##n)               \
	     | DMAC_CHCTRLA_TRIGSRC(CONF_DMAC_TRIGSRC_##n),                                                                \
	 DMAC_CHPRILVL_PRILVL(CONF_DMAC_LVL_##n),                                                                          \
	 (CONF_DMAC_EVIE_##n << DMAC_CHEVCTRL_EVIE_Pos) | (CONF_DMAC_EVOE_##n << DMAC_CHEVCTRL_EVOE_Pos)                   \
	     | (CONF_DMAC_EVACT_##n << DMAC_CHEVCTRL_EVACT_Pos),                                                           \
	 DMAC_BTCTRL_STEPSIZE(CONF_DMAC_STEPSIZE_##n) | (CONF_DMAC_STEPSEL_##n << DMAC_BTCTRL_STEPSEL_Pos)                 \
	     | (CONF_DMAC_DSTINC_##n << DMAC_BTCTRL_DSTINC_Pos) | (CONF_DMAC_SRCINC_##n << DMAC_BTCTRL_SRCINC_Pos)         \
	     | DMAC_BTCTRL_BEATSIZE(CONF_DMAC_BEATSIZE_##n) | DMAC_BTCTRL_BLOCKACT(CONF_DMAC_BLOCKACT_##n)                 \
	     | DMAC_BTCTRL_EVOSEL(CONF_DMAC_EVOSEL_##n)},

/* DMAC channel configuration */
struct dmac_channel_cfg {
	uint32_t ctrla;
	uint8_t  prilvl;
	uint8_t  evctrl;
	uint16_t btctrl;
};

/* DMAC channel configurations */
const static struct dmac_channel_cfg _cfgs[] = {REPEAT_MACRO(DMAC_CHANNEL_CFG, i, DMAC_CH_NUM)};

/**
 * \brief Initialize DMAC
 */
int32_t _dma_init(void)
{
	uint8_t i;

	hri_dmac_clear_CTRL_DMAENABLE_bit(DMAC);
	hri_dmac_clear_CRCCTRL_reg(DMAC, DMAC_CRCCTRL_CRCSRC_Msk);
	hri_dmac_set_CTRL_SWRST_bit(DMAC);
	while (hri_dmac_get_CTRL_SWRST_bit(DMAC))
		;

	hri_dmac_write_CTRL_reg(DMAC,
	                        (CONF_DMAC_LVLEN0 << DMAC_CTRL_LVLEN0_Pos) | (CONF_DMAC_LVLEN1 << DMAC_CTRL_LVLEN1_Pos)
	                            | (CONF_DMAC_LVLEN2 << DMAC_CTRL_LVLEN2_Pos)
	                            | (CONF_DMAC_LVLEN3 << DMAC_CTRL_LVLEN3_Pos));
	hri_dmac_write_DBGCTRL_DBGRUN_bit(DMAC, CONF_DMAC_DBGRUN);

	hri_dmac_write_PRICTRL0_reg(
	    DMAC,
	    DMAC_PRICTRL0_LVLPRI0(CONF_DMAC_LVLPRI0) | DMAC_PRICTRL0_LVLPRI1(CONF_DMAC_LVLPRI1)
	        | DMAC_PRICTRL0_LVLPRI2(CONF_DMAC_LVLPRI2) | DMAC_PRICTRL0_LVLPRI3(CONF_DMAC_LVLPRI3)
	        | (CONF_DMAC_RRLVLEN0 << DMAC_PRICTRL0_RRLVLEN0_Pos) | (CONF_DMAC_RRLVLEN1 << DMAC_PRICTRL0_RRLVLEN1_Pos)
	        | (CONF_DMAC_RRLVLEN2 << DMAC_PRICTRL0_RRLVLEN2_Pos) | (CONF_DMAC_RRLVLEN3 << DMAC_PRICTRL0_RRLVLEN3_Pos));
	hri_dmac_write_BASEADDR_reg(DMAC, (uint32_t)_descriptor_section);
	hri_dmac_write_WRBADDR_reg(DMAC, (uint32_t)_write_back_section);

	for (i = 0; i < DMAC_CH_NUM; i++) {
		hri_dmac_write_CHCTRLA_reg(DMAC, i, _cfgs[i].ctrla);
		hri_dmac_write_CHPRILVL_reg(DMAC, i, _cfgs[i].prilvl);
		hri_dmac_write_CHEVCTRL_reg(DMAC, i, _cfgs[i].evctrl);
		hri_dmacdescriptor_write_BTCTRL_reg(&_descriptor_section[i], _cfgs[i].btctrl);
		hri_dmacdescriptor_write_DESCADDR_reg(&_descriptor_section[i], 0x0);
	}

	for (i = 0; i < 5; i++) {
		NVIC_DisableIRQ(DMAC_0_IRQn + i);
		NVIC_ClearPendingIRQ(DMAC_0_IRQn + i);
		NVIC_EnableIRQ(DMAC_0_IRQn + i);
	}

	hri_dmac_set_CTRL_DMAENABLE_bit(DMAC);

	return ERR_NONE;
}

/**
 * \brief Enable/disable DMA interrupt
 */
void _dma_set_irq_state(const uint8_t channel, const enum _dma_callback_type type, const bool state)
{
	if (DMA_TRANSFER_COMPLETE_CB == type) {
		hri_dmac_write_CHINTEN_TCMPL_bit(DMAC, channel, state);
	} else if (DMA_TRANSFER_ERROR_CB == type) {
		hri_dmac_write_CHINTEN_TERR_bit(DMAC, channel, state);
	}
}

int32_t _dma_set_destination_address(const uint8_t channel, const void *const dst)
{
	hri_dmacdescriptor_write_DSTADDR_reg(&_descriptor_section[channel], (uint32_t)dst);

	return ERR_NONE;
}

int32_t _dma_set_source_address(const uint8_t channel, const void *const src)
{
	hri_dmacdescriptor_write_SRCADDR_reg(&_descriptor_section[channel], (uint32_t)src);

	return ERR_NONE;
}

int32_t _dma_set_next_descriptor(const uint8_t current_channel, const uint8_t next_channel)
{
	hri_dmacdescriptor_write_DESCADDR_reg(&_descriptor_section[current_channel],
	                                      (uint32_t)&_descriptor_section[next_channel]);

	return ERR_NONE;
}

int32_t _dma_srcinc_enable(const uint8_t channel, const bool enable)
{
	hri_dmacdescriptor_write_BTCTRL_SRCINC_bit(&_descriptor_section[channel], enable);

	return ERR_NONE;
}

int32_t _dma_set_data_amount(const uint8_t channel, const uint32_t amount)
{
	uint32_t address   = hri_dmacdescriptor_read_DSTADDR_reg(&_descriptor_section[channel]);
	uint8_t  beat_size = hri_dmacdescriptor_read_BTCTRL_BEATSIZE_bf(&_descriptor_section[channel]);

	if (hri_dmacdescriptor_get_BTCTRL_DSTINC_bit(&_descriptor_section[channel])) {
		hri_dmacdescriptor_write_DSTADDR_reg(&_descriptor_section[channel], address + amount * (1 << beat_size));
	}

	address = hri_dmacdescriptor_read_SRCADDR_reg(&_descriptor_section[channel]);

	if (hri_dmacdescriptor_get_BTCTRL_SRCINC_bit(&_descriptor_section[channel])) {
		hri_dmacdescriptor_write_SRCADDR_reg(&_descriptor_section[channel], address + amount * (1 << beat_size));
	}

	hri_dmacdescriptor_write_BTCNT_reg(&_descriptor_section[channel], amount);

	return ERR_NONE;
}

int32_t _dma_enable_transaction(const uint8_t channel, const bool software_trigger)
{
	hri_dmacdescriptor_set_BTCTRL_VALID_bit(&_descriptor_section[channel]);
	hri_dmac_set_CHCTRLA_ENABLE_bit(DMAC, channel);

	if (software_trigger) {
		hri_dmac_set_SWTRIGCTRL_reg(DMAC, 1 << channel);
	}

	return ERR_NONE;
}

int32_t _dma_get_channel_resource(struct _dma_resource **resource, const uint8_t channel)
{
	*resource = &_resources[channel];

	return ERR_NONE;
}

int32_t _dma_dstinc_enable(const uint8_t channel, const bool enable)
{
	hri_dmacdescriptor_write_BTCTRL_DSTINC_bit(&_descriptor_section[channel], enable);

	return ERR_NONE;
}
/**
 * \internal DMAC interrupt handler
 */
static void _dmac_handler(void)
{
	uint8_t               channel      = hri_dmac_get_INTPEND_reg(DMAC, DMAC_INTPEND_ID_Msk);
	struct _dma_resource *tmp_resource = &_resources[channel];

	if (hri_dmac_get_INTPEND_TERR_bit(DMAC)) {
		hri_dmac_clear_CHINTFLAG_TERR_bit(DMAC, channel);
		tmp_resource->dma_cb.error(tmp_resource);
	} else if (hri_dmac_get_INTPEND_TCMPL_bit(DMAC)) {
		hri_dmac_clear_CHINTFLAG_TCMPL_bit(DMAC, channel);
		tmp_resource->dma_cb.transfer_done(tmp_resource);
	}
}
/**
 * \brief DMAC interrupt handler
 */
void DMAC_0_Handler(void)
{
	_dmac_handler();
}
/**
 * \brief DMAC interrupt handler
 */
void DMAC_1_Handler(void)
{
	_dmac_handler();
}
/**
 * \brief DMAC interrupt handler
 */
void DMAC_2_Handler(void)
{
	_dmac_handler();
}
/**
 * \brief DMAC interrupt handler
 */
void DMAC_3_Handler(void)
{
	_dmac_handler();
}
/**
 * \brief DMAC interrupt handler
 */
void DMAC_4_Handler(void)
{
	_dmac_handler();
}

#endif /* CONF_DMAC_ENABLE */
