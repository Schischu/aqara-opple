/*****************************************************************************
 *
 * MODULE:          JN-AN-1245
 *
 * COMPONENT:       app_switch_state_machine.h
 *
 * DESCRIPTION:     ZLO Controller & Switch Demo: Remote Key Press Behaviour (Interface)
 *
 *****************************************************************************
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
 * Copyright NXP B.V. 2016-2018. All rights reserved
 *
 ****************************************************************************/
#ifndef APP_SWITCH_STATE_MACHINE_H_
#define APP_SWITCH_STATE_MACHINE_H_

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include "zcl.h"
#include "app.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define BUTTON_DELAY_TIME_IN_MS 250

#if 0
#define SW_ON   (1 << APP_BOARD_SW1_PIN)
#define SW_OFF  (1 << APP_BOARD_SW2_PIN )
#define SW_UP   (1 << APP_BOARD_SW3_PIN )
#define SW_DOWN (1 << APP_BOARD_SW4_PIN )
//#define NTAG_FD (1 << APP_BUTTONS_NFC_FD)
#define SW_COMM (1 << APP_BOARD_SW0_PIN)
#endif

#define SW_COMM (1 << APP_BOARD_SW0_PIN)

#define SW_TL   (1 << APP_BOARD_SW1_PIN)
#define SW_TR   (1 << APP_BOARD_SW2_PIN)

#define SW_ML   (1 << APP_BOARD_SW3_PIN)
#define SW_MR   (1 << APP_BOARD_SW4_PIN)

#define SW_BL   (1 << APP_BOARD_SW5_PIN)
#define SW_BR   (1 << APP_BOARD_SW6_PIN)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef enum{
    SCENE_1=1,
    SCENE_2
}te_ScenesId;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC void vApp_ProcessKeyCombination(APP_tsEvent sButton);
PUBLIC void APP_cbTimerButtonDelay(void *pvParam);
PUBLIC void vStopStartModeChangeTimer( uint32 u32Ticks );
PUBLIC void APP_cbTimerChangeMode(void *pvParam);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#endif /* APP_SWITCH_STATE_MACHINE_H_ */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
