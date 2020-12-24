/*****************************************************************************
 *
 * MODULE:             JN-AN-1245
 *
 * COMPONENT:          EH_Button.c
 *
 * DESCRIPTION:        EH switch button handling
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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "jendefs.h"
#include "string.h"
#include <dbg.h>
#include "fsl_iocon.h"
#include "fsl_gpio.h"
#include "fsl_gint.h"
#include "pin_mux.h"
#include "EH_Button.h"
#include "EH_IEEE_Features.h"
#include "EH_IEEE_Commands.h"
#include "app_nvm.h"
#include "ZTimer.h"
#include "app_main.h"
#include "app_events.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/** Time in msec to wait to enable button interrupts when a button interrupt is detected */
#define BUTTON_STABLE_DURATION                  (15)

#define NEXT_MOVE_CMD_DURATION                  (500) /* 500m sec */

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/****************************************************************************/
/***        Exported Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/* flag indicating whether button press detected, updated from ISR */
volatile bool bButtonPressed = 0;

/* The button which is pressed/released , updated from ISR */
volatile uint32 u32Button = 0;

/****************************************************************************/
/***        Local Function Prototypes                                               ***/
/****************************************************************************/
void gint_callback(void);
PUBLIC void APP_cbTimerButtonScan(void *pvParam);
PUBLIC uint32 APP_u32GetSwitchIOState(void);
/****************************************************************************/
/***        Public Functions                                              ***/
/****************************************************************************/
#if (defined BUTTON_MAP_OM15082) ||(defined GP_LEVEL_CONTROL)

        PRIVATE uint8 s_u8ButtonDebounce[APP_BUTTONS_NUM] = { 0xff,0xff,0xff,0xff,0xff };
        PRIVATE uint8 s_u8ButtonState[APP_BUTTONS_NUM] = { 0,0,0,0,0 };
        PRIVATE const uint8 s_u8ButtonDIOLine[APP_BUTTONS_NUM] =
        {
            APP_BOARD_SW0_PIN,
            APP_BOARD_SW1_PIN,
            APP_BOARD_SW2_PIN,
            APP_BOARD_SW3_PIN,
            APP_BOARD_SW4_PIN
        };

#else

        PRIVATE uint8 s_u8ButtonDebounce[APP_BUTTONS_NUM] = { 0xff };
        PRIVATE uint8 s_u8ButtonState[APP_BUTTONS_NUM] = { 0 };
        PRIVATE const uint8 s_u8ButtonDIOLine[APP_BUTTONS_NUM] =
        {
            APP_BOARD_SW0_PIN
        };
#endif

/****************************************************************************
 * NAME: vHandleButtonPress
 *
 * DESCRIPTION:
 * handles button press event on OM15082 and initiates appropriate commands
 *
 * RETURNS:
 * Return whether device can go to sleep
 *
 ****************************************************************************/
#ifdef OM15082
void vHandleButtonPress(te_TransitionCode  eCode )
{
    static bool bIsCommMode = 0;
    vLoadPersistentData();

    switch(eCode)
    {

    case COMM_BUTTON_PRESSED:
        /* if already in operational state, switch to commissioning state */
        if(sGPDPersistentData.b8CommissioiningStatusBits & COMM_COMP_SET_BIT)
        {
            sGPDPersistentData.b8CommissioiningStatusBitsPrevious =
                    sGPDPersistentData.b8CommissioiningStatusBits;
#ifdef GPD_SUPPORT_PERSISTENT_DATA
            bAPP_NVMWriteBlock(  sizeof(tGPD_PersistantData),
                                (uint8 *)&sGPDPersistentData);
            DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\n  reverted= %d, previous = %d",
                sGPDPersistentData.b8CommissioiningStatusBits ,
                sGPDPersistentData.b8CommissioiningStatusBitsPrevious
                );
#endif
            bIsCommMode = TRUE;
        }
        break;

    case ON_PRESSED:
        /* Switch 1: mapping
         * 1. Commissioning Commands
         * 2. On Or  Toggle Command
         * 3. De commissioning command
         * 4. Factory Reset
         * */
        if(bIsCommMode)
        {
            sGPDPersistentData.b8CommissioiningStatusBits = CHANNEL_SET_BIT;
            DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\n Commissioning ");
            bIsCommMode = FALSE;
        }
        DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\n ON_PRESSED HandleGPDStateMachine ");
        HandleGPDStateMachine(0);
        //tbd short press
        break;

    case OFF_PRESSED:
        DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\n APP_E_BUTTONS_BUTTON_SW2 ");
        /* Button 2 mapping :
         * Off command , Decommissioning command if Dio8 pressed */
        if(bIsCommMode)
        {
            if(sGPDPersistentData.b8CommissioiningStatusBits & COMM_COMP_SET_BIT)
            {
                vSendDecommissioingCmd();

                bIsCommMode = FALSE;
            }
        }
        else
        {
            HandleGPDStateMachine(E_GPD_OFF_CMD);
        }
        break;

    case UP_PRESSED:
        DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\n APP_E_BUTTONS_BUTTON_SW3 ");
        /* Button 3 mapping :
         * Move up with on/off( button press) ,
         * Level control stop (button release ) */

        HandleGPDStateMachine(E_GPD_MOVE_UP_CMD);

        break;

    case DOWN_PRESSED:
        HandleGPDStateMachine(E_GPD_MOVE_DOWN_CMD);
        break;

    case DOWN_RELEASED:
    case UP_RELEASED:
        HandleGPDStateMachine(E_GPD_LEVEL_CONTROL_STOP);
        break;

    case UP_AND_ON_TOGETHER_PRESSED:
        /* This is used in current commissioning failed case.
         * Revert state */
        DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\n UP_AND_ON_TOGETHER_PRESSED = %d, previous = %d",
                sGPDPersistentData.b8CommissioiningStatusBits ,
                sGPDPersistentData.b8CommissioiningStatusBitsPrevious
                );
        if((sGPDPersistentData.b8CommissioiningStatusBits == CHANNEL_SET_BIT) &&
        (sGPDPersistentData.b8CommissioiningStatusBitsPrevious & COMM_COMP_SET_BIT))
        {
            sGPDPersistentData.b8CommissioiningStatusBits =
                    sGPDPersistentData.b8CommissioiningStatusBitsPrevious;
            sGPDPersistentData.b8CommissioiningStatusBitsPrevious = 0;
#ifdef GPD_SUPPORT_PERSISTENT_DATA
            bAPP_NVMWriteBlock( sizeof(tGPD_PersistantData),
                                (uint8 *)&sGPDPersistentData);
            DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\n  reverted= %d, previous = %d",
                sGPDPersistentData.b8CommissioiningStatusBits ,
                sGPDPersistentData.b8CommissioiningStatusBitsPrevious
                );
#endif
        }
        break;

    default:
        //DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\n  Button Handler default= 0x%x", eCode);
        break;
    }

}
#endif
/****************************************************************************
 * NAME: bButtonInitialize
 *
 * DESCRIPTION:
 * Button Initialization function, enables interrupts only on OM15082
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
bool bButtonInitialize(void)
{
    /* Define the init structure for the input switch pin */
    gpio_pin_config_t switch_config = {
        kGPIO_DigitalInput,
    };
    uint8 u8Button;
    /* Loop through buttons */
    for (u8Button = 0; u8Button < APP_BUTTONS_NUM; u8Button++)
    {
        /* Valid pin ? */
        if (s_u8ButtonDIOLine[u8Button] < 22)
        {
#if APP_BUTTON_INVERT_MASK != 0
            /* Inverted operation ? */
            if (APP_BUTTON_INVERT_MASK & (1 << s_u8ButtonDIOLine[u8Button]))
            {
                /* Configure io mux for pull down operation */
                IOCON_PinMuxSet(IOCON, 0, s_u8ButtonDIOLine[u8Button], IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN);
            }
            /* Normal operation ? */
            else
#endif
            {
                /* Configure io mux for pull up operation */
                IOCON_PinMuxSet(IOCON, 0, s_u8ButtonDIOLine[u8Button], IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN);
            }
            /* Initialise GPIO use */
            GPIO_PinInit(GPIO, APP_BOARD_GPIO_PORT, s_u8ButtonDIOLine[u8Button], &switch_config);
        }
    }

    /* Initialize GINT0 */
    GINT_Init(GINT0);

    /* Setup GINT0 for edge trigger, "OR" mode */
    GINT_SetCtrl(GINT0, kGINT_CombineOr, kGINT_TrigEdge, gint_callback);

    /* Select pins & polarity for GINT0 */
    GINT_ConfigPins(GINT0, DEMO_GINT0_PORT, DEMO_GINT0_POL_MASK, DEMO_GINT0_ENA_MASK);

    /* Enable callback(s) */
    GINT_EnableCallback(GINT0);

    /* If we came out of deep sleep; perform appropriate action as well based
       on button press.*/
    APP_cbTimerButtonScan(NULL);

    uint32 u32Buttons = APP_u32GetSwitchIOState();
    if (u32Buttons != APP_BUTTONS_DIO_MASK)
    {
        return TRUE;
    }
    return FALSE;
}
#ifdef GP_LEVEL_CONTROL
void vHandleLevelControlButtonPress(void)
{
    static uint8 u8data;
    switch(APP_u32GetSwitchIOState()&APP_BUTTONS_DIO_MASK)
    {
        case BUTTON_ON:
            u8data=0x01;
            bAHI_WriteEEPROMBlock(60,sizeof(uint8),(uint8 *)&u8data);
            HandleGPDStateMachine(E_GPD_ON_CMD);
            break;

        case BUTTON_OFF:
            u8data=0x01;
            bAHI_WriteEEPROMBlock(60,sizeof(uint8),(uint8 *)&u8data);
            HandleGPDStateMachine(E_GPD_OFF_CMD);
            break;

        case BUTTON_INCREASE:
            HandleGPDStateMachine(E_GPD_MOVE_UP_CMD);
            break;

        case BUTTON_DECREASE:
            HandleGPDStateMachine(E_GPD_MOVE_DOWN_CMD);
            break;

        default:
            bAHI_ReadEEPROMBlock(60,sizeof(uint8),(uint8 *)&u8data);
            if (u8data == 0x01)
            {
                u8data=0x00;
                bAHI_WriteEEPROMBlock(60,sizeof(uint8),(uint8 *)&u8data);
            }
            else
            {
                HandleGPDStateMachine(E_GPD_LEVEL_CONTROL_STOP);
            }
            break;
     }
}
#endif
#ifdef GP_AUTO_SWITCH
void vHandleEHButtonPress(void)
{
    /* Read data from epprom */
    switch(APP_u32GetSwitchIOState()&0x100)
        {
        case BUTTON_PRESSED:
            DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\nButton Pressed");
            /* Send a packet */
            HandleGPDStateMachine(0);
            break;

        case BUTTON_RELEASED:
            DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\nButton Released");
            /* Do nothing */
            break;

        default:
            break;
        }
}
#endif
/****************************************************************************/
/***        Local Functions                                              ***/
/****************************************************************************/


/****************************************************************************
 *
 * NAME: APP_u32GetSwitchIOState
 *
 * DESCRIPTION:
 * reads the IO state of any switch inputs, flips the bits of any switches that are
 * active high
 *
 * PARAMETER:
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC uint32 APP_u32GetSwitchIOState(void)
{
    return (GPIO_PortRead(GPIO, APP_BOARD_GPIO_PORT) & APP_BUTTONS_DIO_MASK) ^ APP_BUTTON_INVERT_MASK;
}

/****************************************************************************
 *
 * NAME: gint_callback
 *
 * DESCRIPTION:
 *callback from the group io interrupt
 *
 * PARAMETER:
 *
 * RETURNS:
 *
 ****************************************************************************/

void gint_callback(void)
{
#ifdef OM15082
    bEnableSleep = FALSE;
#endif
    /* Take action for gint event */;
    uint32 u32IOStatus = (GPIO_PortRead(GPIO, APP_BOARD_GPIO_PORT) & APP_BUTTONS_DIO_MASK) ^ APP_BUTTON_INVERT_MASK;
    DBG_vPrintf(DBG_DEVICE_SWITCH, "GINT 0 Pin Interrupt event detected %x\r\r", u32IOStatus);

    if (u32IOStatus != APP_BUTTONS_DIO_MASK)
    {
        /* disable edge detection until scan complete */
        GINT_DisableCallback(GINT0);
        ZTIMER_eStop(u8TimerButtonScan);
        ZTIMER_eStart(u8TimerButtonScan, ZTIMER_TIME_MSEC(10));
    }

}

/****************************************************************************
 *
 * NAME: APP_cbTimerButtonScan
 *
 * DESCRIPTION:
 * Button scan task -only present on NON SSL builds
 *
 * PARAMETER:
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC void APP_cbTimerButtonScan(void *pvParam)
{
    /*
     * The DIO changed status register is reset here before the scan is performed.
     * This avoids a race condition between finishing a scan and re-enabling the
     * DIO to interrupt on a falling edge.
     */

    uint8 u8AllReleased = 0xff;
    unsigned int i;
    uint32 u32DIOState = 0;
    u32DIOState = APP_u32GetSwitchIOState();


    for (i = 0; i < APP_BUTTONS_NUM; i++)
    {
        uint8 u8Button = 0;
        u8Button = (uint8) ((u32DIOState >> s_u8ButtonDIOLine[i]) & 1);

        s_u8ButtonDebounce[i] <<= 1;
        s_u8ButtonDebounce[i] |= u8Button;
        u8AllReleased &= s_u8ButtonDebounce[i];

        if (0 == s_u8ButtonDebounce[i] && !s_u8ButtonState[i])
        {
            s_u8ButtonState[i] = TRUE;

            /*
             * button consistently depressed for 8 scan periods
             * so post message to application task to indicate
             * a button down event
             */
            APP_tsEvent sButtonEvent;
            sButtonEvent.eType = APP_E_EVENT_BUTTON_DOWN;
            sButtonEvent.uEvent.sButton.u8Button = i;
            sButtonEvent.uEvent.sButton.u32DIOState = u32DIOState;
            DBG_vPrintf(DBG_DEVICE_SWITCH, "Button DN=%d\r", i);

            bHandleEventSend(&APP_msgAppEvents, &sButtonEvent);
        }
        else if (0xff == s_u8ButtonDebounce[i] && s_u8ButtonState[i] != FALSE)
        {
            s_u8ButtonState[i] = FALSE;

            /*
             * button consistently released for 8 scan periods
             * so post message to application task to indicate
             * a button up event
             */
            APP_tsEvent sButtonEvent;
            sButtonEvent.eType = APP_E_EVENT_BUTTON_UP;
            sButtonEvent.uEvent.sButton.u8Button = i;
            sButtonEvent.uEvent.sButton.u32DIOState=u32DIOState;
            DBG_vPrintf(DBG_DEVICE_SWITCH, "Button UP=%i\r", i);

            bHandleEventSend(&APP_msgAppEvents, &sButtonEvent);
        }
    }

    if (0xff == u8AllReleased)
    {
        /*
         * all buttons high so set dio to interrupt on change
         */
        DBG_vPrintf(DBG_DEVICE_SWITCH, "ALL UP\r", i);
        GINT_EnableCallback(GINT0);
        ZTIMER_eStop(u8TimerButtonScan);
    }
    else
    {
        /*
         * one or more buttons is still depressed so continue scanning
         */
        ZTIMER_eStart(u8TimerButtonScan, ZTIMER_TIME_MSEC(10));
    }
}

