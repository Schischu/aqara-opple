/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2019 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fsl_xcvr.h"
#include "fsl_reset.h"
#include "radio.h"
#include "FunctionLib.h"

#if gRadioUsePdm_d
#include "PDM.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if gRadioUsePdm_d
#if defined PDM_ID_RADIO_SETTINGS
#define PDM_RADIO_KMOD_TAG_OFFSET PDM_ID_RADIO_SETTINGS
#else
#define PDM_RADIO_KMOD_TAG_OFFSET (0xff00)
#endif
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
static bool cold_init = true;
static radio_mode_t current_radio_mode = NUM_RADIO_MODES;
static data_rate_t current_data_rate = DR_UNASSIGNED;

/*******************************************************************************
 * Code
 ******************************************************************************/


void XCVR_RadioBleConfigure(data_rate_t data_rate)
{

    assert(data_rate == DR_2MBPS || data_rate == DR_1MBPS);

    vRadio_Standard_Init(RADIO_STANDARD_BLE_1MB);
    if (data_rate == DR_2MBPS)
    {
        vRadio_Standard_Init(RADIO_STANDARD_BLE_2MB);
    }
    if (current_radio_mode == ZIGBEE_MODE)
    {
        vRadio_ZBtoBLE();
    }
    current_radio_mode = BLE_MODE;
    current_data_rate = data_rate;
}

void XCVR_RadioZigbeeConfigure(void)
{
    vRadio_InitialiseRadioStandard();
    /* invokes vRadio_Standard_Init with the right ZIGBEE mode */
    if (current_radio_mode == BLE_MODE)
    {
        vRadio_BLEtoZB();
    }
    current_radio_mode = ZIGBEE_MODE;
    current_data_rate = DR_UNASSIGNED;

}


static int XCVR_ModeCfg(radio_mode_t radio_mode, data_rate_t data_rate, bool wake)
{
    int res = -1;
    if (!wake && (current_radio_mode == radio_mode))
    {
        res = 1;
    }
    else switch (radio_mode) {
	    case BLE_MODE:
	        if (data_rate == DR_2MBPS || data_rate == DR_1MBPS)
	        {
	            XCVR_RadioBleConfigure(data_rate);
	            res = 0;
	        }
	        break;
	    case ZIGBEE_MODE:
	        XCVR_RadioZigbeeConfigure();
	        res = 0;
	        break;
	    default:
	        assert(0);
	        break;
	}
	return res;
}

xcvrStatus_t XCVR_Init(radio_mode_t radio_mode, data_rate_t data_rate)
{
    xcvrStatus_t res = gXcvrSuccess_c;
    if (cold_init)
    {
        /* BLE Reset on Cold init only but not on wakeup
         * It is not required to reset the BLE again if called several times
         * */
#if gSupportBle
        RESET_SetPeripheralReset(kBLE_RST_SHIFT_RSTn);
        RESET_ClearPeripheralReset(kBLE_RST_SHIFT_RSTn);
#endif
        cold_init = false;
    }
    /* XCVR_Init does not repeat the RadioInit if already done and we have kept the power */
    vRadio_RadioInit(RADIO_MODE_STD_USE_INITCAL);
    if (XCVR_ModeCfg(radio_mode, data_rate, false) < 0)
        res = gXcvrInvalidParameters_c;

    return res;
}

xcvrStatus_t XCVR_WakeUpInit(void)
{
    xcvrStatus_t res = gXcvrSuccess_c;

    /* Do RadioInit on each wakup */
    vRadio_RadioInit(RADIO_MODE_STD_USE_INITCAL);
    if (XCVR_ModeCfg(current_radio_mode, current_data_rate, true) < 0)
    	res = gXcvrInvalidParameters_c;
    return res;
}

#if gRadioUsePdm_d
bool bRadioCB_WriteNVM(uint8_t *pu8DataBlock, uint16_t u16Len)
{
    bool st = false;
    st = (PDM_eSaveRecordData(PDM_RADIO_KMOD_TAG_OFFSET, pu8DataBlock, (uint16_t)u16Len) ==  PDM_E_STATUS_OK);
    return st;
}

uint16_t u16RadioCB_ReadNVM(uint8_t *pu8DataBlock , uint16_t u16MaxLen)
{
    uint16_t lengthPtr = 0;
    if (PDM_bDoesDataExist(PDM_RADIO_KMOD_TAG_OFFSET, &lengthPtr))
    {
        if (PDM_eReadDataFromRecord(PDM_RADIO_KMOD_TAG_OFFSET, pu8DataBlock, u16MaxLen, &lengthPtr ) != PDM_E_STATUS_OK)
        {
            lengthPtr = 0;
        }
    }
    return lengthPtr;
}

#endif

void XCVR_Deinit(void)
{
    vRadio_RadioDeInit();
}

void XCVR_TemperatureUpdate(int32_t temperature)
{
    /* temperature expected by Radio should be expressed in half of degree C */
    vRadio_Temp_Update((int16_t) (2*temperature));
}

void XCVR_Recalibrate(void)
{
    vRadio_Recal();
}

uint32_t XCVR_GetRecalDuration(void)
{
    return u32Radio_Get_Next_Recal_Duration();
}

void XCVR_LockupCheckAndAbortRadio(void)
{
    vRadio_LockupCheckAndAbortRadio();
}

void XCVR_EnableBLEFastTX(bool_t keepPll)
{
    vRadio_EnableBLEFastTX(keepPll);
}

void XCVR_DisableBLEFastTX(void)
{
    vRadio_DisableBLEFastTX();
}

int16_t XCVR_ReadRSSI(data_rate_t rate)
{
    return i16Radio_GetNbRSSISyncCor(rate);
}

void XCVR_SetBLEdpTopEmAddr(uint32_t em_addr)
{
    vRadio_SetBLEdpTopEmAddr(em_addr);
}
