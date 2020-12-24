/*****************************************************************************
 *
 * MODULE:             JN-AN-1245
 *
 * COMPONENT:          app_led_control.c
 *
 * DESCRIPTION:        ZLO Demo - Led Control Implementation
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
#include <jendefs.h>
#include "dbg.h"
#include "app.h"
#include "app_led_control.h"

#include "ZTimer.h"
#include "app_main.h"

#include "app_serial_interface.h"



/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DIO_LED_MASK  0x03UL
#define LED_BLINK_TIME ZTIMER_TIME_MSEC(50)    /*  0.05sec blink time */

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


PRIVATE teShiftLevel eAppShiftLevel;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC void APP_vInitLeds(void)
{
#if (defined OM15082)
    /* Define the init structure for the output LED pin*/
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput, OFF,
    };

    GPIO_PinInit(GPIO, APP_BOARD_GPIO_PORT, APP_BOARD_LED1_PIN, &led_config);
    GPIO_PinInit(GPIO, APP_BOARD_GPIO_PORT, APP_BOARD_LED2_PIN, &led_config);
    GPIO_PinInit(GPIO, APP_BOARD_GPIO_PORT, APP_BOARD_LED3_PIN, &led_config);

    GPIO_PinInit(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED1_PIN, &led_config);
    GPIO_PinInit(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED2_PIN, &led_config);
#endif

}


PUBLIC void APP_vBlinkLeds(teShiftLevel eShiftLevel)
{
    eAppShiftLevel = eShiftLevel;
    APP_vSetLedsMask((~eShiftLevel) & DIO_LED_MASK);
    ZTIMER_eStop(u8TimerLedBlink);
    ZTIMER_eStart(u8TimerLedBlink, LED_BLINK_TIME);
}

PUBLIC void APP_vSetLedsMask(teShiftLevel eShiftLevel)
{

    (eShiftLevel & 0x01) ? APP_vSetLed(LED2, ON): APP_vSetLed(LED2, OFF);
    (eShiftLevel & 0x02) ? APP_vSetLed(LED1, ON): APP_vSetLed(LED1, OFF);

    uint8 u8Leds = (uint8)eShiftLevel;
    /* this serial protocol message will cause what looks like garbage debug output on a terminal */
    vSL_WriteMessage(SERIAL_MSG_TYPE_LEDS, SERIAL_MSG_LENGTH_LEDS, &u8Leds);

}

/****************************************************************************
 *
 * NAME: APP_vSetLed
 *
 * DESCRIPTION:
 * set the state ofthe given application led
 *
 * PARAMETER:
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC void APP_vSetLed(uint u8Led, bool_t bState)
{
    uint32 u32LedPin = 0;

#if (defined OM15082)
    switch (u8Led)
    {
    case LED1:
        u32LedPin = APP_BOARD_LED1_PIN;
        break;
    case LED2:
        u32LedPin = APP_BOARD_LED2_PIN;
        break;
    case LED3:
        u32LedPin = APP_BOARD_LED3_PIN;
        break;
    case LED4:
            u32LedPin = APP_BASE_BOARD_LED1_PIN;
            bState = ( bState==OFF );
            break;
    case LED5:
            u32LedPin = APP_BASE_BOARD_LED1_PIN;
            bState = ( bState==OFF );
            break;
    default:
        u32LedPin -= 1;
        break;
    }

    if (u32LedPin != 0xffffffff)
    {
        GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, u32LedPin, bState);
    }
#endif

}


PUBLIC void APP_cbTimerLedBlink(void *pvParam)
{
    APP_vSetLedsMask(eAppShiftLevel);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
