/*****************************************************************************
 *
 * MODULE:             JN-AN-1245
 *
 * COMPONENT:          PDM_IDs.h
 *
 * DESCRIPTION:        Persistance Data Manager ID definitions
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5169,
 * JN5179, JN5189].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2016-2019. All rights reserved
 *
 ***************************************************************************/

#ifndef  PDMIDS_H_INCLUDED
#define  PDMIDS_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif


/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define PDM_ID_APP_ZLO_CONTROLLER                 (0x2)
#define PDM_ID_APP_END_P_TABLE                    (0x3)
#define PDM_ID_APP_GROUP_TABLE                    (0x4)
#define PDM_ID_APP_ZLO_SWITCH                     (0x5)
#define PDM_ID_OTA_APP                            (0x9) /* Application OTA data (server) */
#define PDM_ID_OTA_DATA                           (0xA) /* Stack OTA data */
#if (defined CoinCellSwitch)
#define PDM_ID_APP_ZLO_COINCELL_SWITCH			  (0x2)
#define PDM_ID_APP_REPORTS						  (0x3)
#define PDM_ID_APP_POLL_CONTROL         		  (0x4)
#define PDM_ID_APP_POWER_CONFIGURATION            (0x5)
#define PDM_ID_APP_SAVE_EVENT			          (0x6)
#define PDM_ID_APP_SAVE_SCAN_CHANNELS             (0x7)
#endif

/* APP_NTAG_ICODE */
#define PDM_ID_APP_NFC_NWK_NCI                    (0xA130)
#define PDM_ID_APP_NFC_ICODE                      (0xA131)

#ifndef PDM_ID_BASE_RADIO
#define PDM_ID_BASE_RADIO                         (0xff00) /* 0xff00�0xffff: Radio driver records */
#define PDM_ID_RADIO_SETTINGS (PDM_ID_BASE_RADIO + 0x0000) /* Holds radio KMOD calibration data */
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif /* PDMIDS_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
