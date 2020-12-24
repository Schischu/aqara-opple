
/*****************************************************************************

 *
 * MODULE:             JN-AN-1245
 *
 * COMPONENT:          EH_Button.h
 *
 * DESCRIPTION:        ZigBee EH Button
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

#ifndef EH_BUTTON_H_
#define EH_BUTTON_H_
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include "jendefs.h"
#include "app.h"


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

 /* DIO mapping for buttons on OM15082 and EH switch */

/** Maximum time between two short press in  msec */
#define SHORT_PRESS_TIME                        (1000 *10)    /* 10 sec */

#if (defined BUTTON_MAP_OM15082)

        typedef enum {
            APP_E_BUTTONS_BUTTON_1 = 0,
            APP_E_BUTTONS_BUTTON_SW1,
            APP_E_BUTTONS_BUTTON_SW2,
            APP_E_BUTTONS_BUTTON_SW3,
            APP_E_BUTTONS_BUTTON_SW4
        } APP_teButtons;

        #define APP_BUTTONS_NUM                     (5UL)

        #define APP_BUTTONS_DIO_MASK                ((1 << APP_BOARD_SW0_PIN)|(1 << APP_BOARD_SW1_PIN)|(1 << APP_BOARD_SW2_PIN) | (1 << APP_BOARD_SW3_PIN) | (1 << APP_BOARD_SW4_PIN))
        #define APP_BUTTONS_DIO_MASK_FOR_DEEP_SLEEP (                         (1 << APP_BOARD_SW1_PIN)|(1 << APP_BOARD_SW2_PIN) | (1 << APP_BOARD_SW3_PIN) | (1 << APP_BOARD_SW4_PIN))
        /* OM15076-3 Carrier Board */
        #define APP_BUTTON_INVERT_MASK                0
        /* OM15076-1 Carrier Board */
        //#define APP_BUTTON_INVERT_MASK              (1 << APP_BOARD_SW0_PIN)
        #define DEMO_GINT0_POL_MASK                   APP_BUTTON_INVERT_MASK
        #define DEMO_GINT0_ENA_MASK                   APP_BUTTONS_DIO_MASK
#elif (defined GP_LEVEL_CONTROL )

    /* DIO used for GP_LEVEL_CONTROL switch */
    #define DIO_11 (1 << APP_BOARD_SW1_PIN)
    #define DIO_15 (1 << APP_BOARD_SW2_PIN)
    #define DIO_12 (1 << APP_BOARD_SW3_PIN)
    #define DIO_13 (1 << APP_BOARD_SW4_PIN)

    /* Mask for all buttons */
    #define APP_BUTTONS_DIO_MASK (DIO_11|DIO_12|DIO_13|DIO_15)

    /* Button defines */
    #define BUTTON_ON       DIO_13     //DIO 11 for On button
    #define BUTTON_OFF      DIO_15     //DIO 15 for off button
    #define BUTTON_INCREASE DIO_12     //DIO 12 for light increase button
    #define BUTTON_DECREASE DIO_11     //DIO 13 for light decrease button

#elif  (defined GP_AUTO_SWITCH)
    #define DIO_8 ((1 << APP_BOARD_SW0_PIN))
    #define DIO_BUTTON_MASK (DIO_8)
    #define BUTTON_PRESSED       0x00               /*No DIO set */
    #define BUTTON_RELEASED      DIO_8              /*DIO 8 set when button released*/
    #define APP_BUTTONS_DIO_MASK DIO_8
#else
    typedef enum {
        APP_E_BUTTONS_BUTTON_1
    } APP_teButtons;

    #define APP_BUTTONS_NUM                     (1UL)
    #define APP_BUTTONS_BUTTON_1                (APP_BOARD_SW0_PIN)
    /* OM15076-3 Carrier Board */
    #define APP_BUTTON_INVERT_MASK                0
    /* OM15076-1 Carrier Board */
    //#define APP_BUTTON_INVERT_MASK              (1 << APP_BOARD_SW0_PIN)
    #define APP_BUTTONS_DIO_MASK                (1 << APP_BOARD_SW0_PIN)
    #define DEMO_GINT0_POL_MASK                   APP_BUTTON_INVERT_MASK
    #define DEMO_GINT0_ENA_MASK                   APP_BUTTONS_DIO_MASK
#endif

/* Kindly Maintain the order as the button numbers are assigned directly */

typedef enum{
    COMM_BUTTON_PRESSED,
    ON_PRESSED,
    OFF_PRESSED,
    UP_PRESSED,
    DOWN_PRESSED,
    UP_AND_ON_TOGETHER_PRESSED,
    UP_AND_OFF_TOGETHER_PRESSED,
    DOWN_AND_OFF_TOGETHER_PRESSED,
    DOWN_AND_ON_TOGETHER_PRESSED,
    COMM_AND_ON_TOGETHER_PRESSED,
    COMM_AND_OFF_TOGETHER_PRESSED,
    COMM_AND_UP_TOGETHER_PRESSED,
    COMM_AND_DOWN_TOGETHER_PRESSED,
    COMM_BUTTON_RELEASED,
    UP_RELEASED,
    DOWN_RELEASED,
    NUMBER_OF_TRANSITION_CODE
}te_TransitionCode;


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported variables                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Public Functions                                              ***/
/****************************************************************************/
//bool bHandleButtonPress(bool bIsPowerUp);
void vHandleButtonPress(te_TransitionCode  eCode );
bool bProcessGPDEvent(void);
bool bButtonInitialize(void);
void vHandleLevelControlButtonPress(void);
void vHandleEHButtonPress(void);
PUBLIC void APP_cbTimerButtonScan(void *pvParam);
#endif /* EH_BUTTON_H_ */
