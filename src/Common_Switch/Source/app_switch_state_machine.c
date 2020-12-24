/*****************************************************************************
 *
 * MODULE:          JN-AN-1245
 *
 * COMPONENT:       app_switch_state_machine.c
 *
 * DESCRIPTION:     ZLO Controller & Switch Demo: Remote Key Press Behaviour (Implementation)
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
 * Copyright NXP B.V. 2016-2018. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "dbg.h"
#include "app_events.h"
#include "app_buttons.h"
#include "app_zlo_switch_node.h"
#include "ZTimer.h"
#include "app_switch_state_machine.h"
#include "App_DimmerSwitch.h"
#include "app_main.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifdef DEBUG_SWITCH_STATE
    #define TRACE_SWITCH_STATE   TRUE
#else
    #define TRACE_SWITCH_STATE   FALSE
#endif

#define HAVE_HUE_SUPPORT_MAX_KEYS_4

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC void vApp_ProcessKeyCombination(APP_tsEvent sButton)
{
    DBG_vPrintf(TRACE_SWITCH_STATE, "\nButton Event = %d - ",sButton.eType);
    switch(sButton.eType)
    {
        case APP_E_EVENT_BUTTON_DOWN:
            DBG_vPrintf(TRACE_SWITCH_STATE, "Button Number = %d - ",sButton.uEvent.sButton.u8Button);
            DBG_vPrintf(TRACE_SWITCH_STATE, "DIO State    = %08x\n",sButton.uEvent.sButton.u32DIOState);

            switch (sButton.uEvent.sButton.u8Button)
            {
                case APP_E_BUTTONS_BUTTON_1:

                    DBG_vPrintf(TRACE_SWITCH_STATE," Network Steering \n");
                    sBDB.sAttrib.u32bdbPrimaryChannelSet = BDB_PRIMARY_CHANNEL_SET;
                    sBDB.sAttrib.u32bdbSecondaryChannelSet = BDB_SECONDARY_CHANNEL_SET;
                    BDB_eNsStartNwkSteering();
                    break;

#ifdef HAVE_HUE_SUPPORT_MAX_KEYS_4
                case APP_E_BUTTONS_BUTTON_SW1:
                	if (sButton.uEvent.sButton.u8Hold == 0)
                	{
                        vAppKeyPress(APP_E_BUTTONS_BUTTON_SW1, 0);
                	}
                	break;
                case APP_E_BUTTONS_BUTTON_SW2:
                	if (sButton.uEvent.sButton.u8Hold == 0)
                	{
                        vAppKeyPress(APP_E_BUTTONS_BUTTON_SW4, 0);
                	}
                	break;

                case APP_E_BUTTONS_BUTTON_SW3:
                	if (sButton.uEvent.sButton.u8Hold == 0)
                	{
                		vAppKeyPress(APP_E_BUTTONS_BUTTON_SW1, 0);
                		vAppKeyPress(APP_E_BUTTONS_BUTTON_SW1, 1 /*HOLD*/);
                	}
                    break;
                case APP_E_BUTTONS_BUTTON_SW4:
                	if (sButton.uEvent.sButton.u8Hold == 0)
                	{
                		vAppKeyPress(APP_E_BUTTONS_BUTTON_SW4, 0);
                		vAppKeyPress(APP_E_BUTTONS_BUTTON_SW4, 1 /*HOLD*/);
                	}
                    break;

                case APP_E_BUTTONS_BUTTON_SW5:
                    vAppKeyPress(APP_E_BUTTONS_BUTTON_SW2, sButton.uEvent.sButton.u8Hold);
                    break;
                case APP_E_BUTTONS_BUTTON_SW6:
                    vAppKeyPress(APP_E_BUTTONS_BUTTON_SW3, sButton.uEvent.sButton.u8Hold);
                    break;
#else
                case APP_E_BUTTONS_BUTTON_SW1:
                case APP_E_BUTTONS_BUTTON_SW2:
                case APP_E_BUTTONS_BUTTON_SW3:
                case APP_E_BUTTONS_BUTTON_SW4:
                case APP_E_BUTTONS_BUTTON_SW5:
                case APP_E_BUTTONS_BUTTON_SW6:
                    vAppKeyPress(sButton.uEvent.sButton.u8Button, sButton.uEvent.sButton.u8Hold);
                    break;
#endif

                default :
                    break;
            }

            break;

        case APP_E_EVENT_BUTTON_UP:
            DBG_vPrintf(TRACE_SWITCH_STATE, "Button Number= %d - ",sButton.uEvent.sButton.u8Button);
            DBG_vPrintf(TRACE_SWITCH_STATE, "DIO State    = %08x\n",sButton.uEvent.sButton.u32DIOState);

            switch (sButton.uEvent.sButton.u8Button)
            {
                case APP_E_BUTTONS_BUTTON_1:
                    //vAPP_ZCL_DeviceSpecific_IdentifyOff();
                    //BDB_vFbExitAsInitiator();
                    break;

#ifdef HAVE_HUE_SUPPORT_MAX_KEYS_4
                case APP_E_BUTTONS_BUTTON_SW1:
                    vAppKeyRelease(APP_E_BUTTONS_BUTTON_SW1, 0 /*SHORT*/);
                    break;
                case APP_E_BUTTONS_BUTTON_SW2:
                    vAppKeyRelease(APP_E_BUTTONS_BUTTON_SW4, 0 /*SHORT*/);
                    break;

                case APP_E_BUTTONS_BUTTON_SW3:
                    vAppKeyRelease(APP_E_BUTTONS_BUTTON_SW1, 2 /*LONG*/);
                    break;
                case APP_E_BUTTONS_BUTTON_SW4:
                    vAppKeyRelease(APP_E_BUTTONS_BUTTON_SW4, 2 /*LONG*/);
                    break;

                case APP_E_BUTTONS_BUTTON_SW5:
                    vAppKeyRelease(APP_E_BUTTONS_BUTTON_SW2, sButton.uEvent.sButton.u8Hold);
                    break;
                case APP_E_BUTTONS_BUTTON_SW6:
                    vAppKeyRelease(APP_E_BUTTONS_BUTTON_SW3, sButton.uEvent.sButton.u8Hold);
                    break;
#else
                case APP_E_BUTTONS_BUTTON_SW1:
                case APP_E_BUTTONS_BUTTON_SW2:
                case APP_E_BUTTONS_BUTTON_SW3:
                case APP_E_BUTTONS_BUTTON_SW4:
                case APP_E_BUTTONS_BUTTON_SW5:
                case APP_E_BUTTONS_BUTTON_SW6:
                    vAppKeyRelease(sButton.uEvent.sButton.u8Button, sButton.uEvent.sButton.u8Hold);
                    break;
#endif

                default :
                    break;
            }
            break;
        default :
            break;
    }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
