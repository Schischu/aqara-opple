/*****************************************************************************
 *
 * MODULE:             JN-AN-1245
 *
 * COMPONENT:          app_buttons.h
 *
 * DESCRIPTION:        Button Press detection (Interface)
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

#ifndef APP_BUTTONS_H
#define APP_BUTTONS_H

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Number of real GPIO lines */
#define APP_NUMBER_OF_GPIO 22

/* Dummy GPIO bit to use for internal NTAG FD line */
/* Use 22 as it has special purpose of waking from sleep when masks below are provided to PWRM */
#define APP_INTERNAL_NTAG_FD_BIT (22)
/* PIO pin to use for FD line */
/* 0xff = disabled, <=21 = GPIO, 22 = JN5189T Internal NTAG FD */
#define APP_BUTTONS_NFC_FD (APP_INTERNAL_NTAG_FD_BIT)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

//#define APP_BOARD_SW0_PIN   (14)   /* Pairing PIO14_PIN17 */

#if 0
00210c09 //_1
00214c09
4000

00204c09 //_SW1
00214c09
10000

00214c01 //_SW2
00214c09
8
#endif

#if (defined BUTTON_MAP_AQARA_OPPLE)
	typedef enum {
		APP_E_BUTTONS_BUTTON_1 = 0,
		APP_E_BUTTONS_BUTTON_SW1,
		APP_E_BUTTONS_BUTTON_SW2,
		APP_E_BUTTONS_BUTTON_SW3,
		APP_E_BUTTONS_BUTTON_SW4,
		APP_E_BUTTONS_BUTTON_SW5,
		APP_E_BUTTONS_BUTTON_SW6
	} APP_teButtons;

	#define APP_BUTTONS_NUM                     (7UL)

	#define APP_BUTTONS_DIO_MASK                ((1 << APP_BOARD_SW0_PIN)|(1 << APP_BOARD_SW1_PIN)|(1 << APP_BOARD_SW2_PIN) | (1 << APP_BOARD_SW3_PIN) | (1 << APP_BOARD_SW4_PIN) | (1 << APP_BOARD_SW5_PIN) | (1 << APP_BOARD_SW6_PIN))
	#define APP_BUTTONS_DIO_MASK_FOR_DEEP_SLEEP ((1 << APP_BOARD_SW0_PIN)|(1 << APP_BOARD_SW1_PIN)|(1 << APP_BOARD_SW2_PIN) | (1 << APP_BOARD_SW3_PIN) | (1 << APP_BOARD_SW4_PIN) | (1 << APP_BOARD_SW5_PIN) | (1 << APP_BOARD_SW6_PIN))
	/* OM15076-3 Carrier Board */
	#define APP_BUTTON_INVERT_MASK                0
	//#define APP_BUTTON_INVERT_MASK             ((1 << APP_BOARD_SW0_PIN)|(1 << APP_BOARD_SW1_PIN)|(1 << APP_BOARD_SW2_PIN) | (1 << APP_BOARD_SW3_PIN) | (1 << APP_BOARD_SW4_PIN) | (1 << APP_BOARD_SW5_PIN) | (1 << APP_BOARD_SW6_PIN))
	/* OM15076-1 Carrier Board */
	//#define APP_BUTTON_INVERT_MASK              (1 << APP_BOARD_SW0_PIN)
	#define DEMO_GINT0_POL_MASK         ((APP_BUTTON_INVERT_MASK) & 0x3FFFFF)
	#define DEMO_GINT0_ENA_MASK         ((APP_BUTTONS_DIO_MASK)   & 0x3FFFFF)
#else
#if (defined BUTTON_MAP_OM15082)
    #if ((defined APP_NTAG_ICODE) && (APP_BUTTONS_NFC_FD != (0xff)))
        typedef enum {
            APP_E_BUTTONS_BUTTON_1 = 0,
            APP_E_BUTTONS_BUTTON_SW1,
            APP_E_BUTTONS_BUTTON_SW2,
            APP_E_BUTTONS_BUTTON_SW3,
            APP_E_BUTTONS_BUTTON_SW4,
            APP_E_BUTTONS_NFC_FD
        } APP_teButtons;

        #define APP_BUTTONS_NUM                     (6UL)

        #define APP_BUTTONS_DIO_MASK                ((1 << APP_BOARD_SW0_PIN)|(1 << APP_BOARD_SW1_PIN)|(1 << APP_BOARD_SW2_PIN) | (1 << APP_BOARD_SW3_PIN) | (1 << APP_BOARD_SW4_PIN) | (1 << APP_BUTTONS_NFC_FD))
        #define APP_BUTTONS_DIO_MASK_FOR_DEEP_SLEEP (                         (1 << APP_BOARD_SW1_PIN)|(1 << APP_BOARD_SW2_PIN) | (1 << APP_BOARD_SW3_PIN) | (1 << APP_BOARD_SW4_PIN) | (1 << APP_BUTTONS_NFC_FD))
        /* OM15076-3 Carrier Board */
        #define APP_BUTTON_INVERT_MASK                0
        /* OM15076-1 Carrier Board */
        //#define APP_BUTTON_INVERT_MASK              (1 << APP_BOARD_SW0_PIN)
        #define DEMO_GINT0_POL_MASK         ((APP_BUTTON_INVERT_MASK) & 0x3FFFFF)
        #define DEMO_GINT0_ENA_MASK         ((APP_BUTTONS_DIO_MASK)   & 0x3FFFFF)
    #else
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
        #define DEMO_GINT0_POL_MASK         ((APP_BUTTON_INVERT_MASK) & 0x3FFFFF)
        #define DEMO_GINT0_ENA_MASK         ((APP_BUTTONS_DIO_MASK)   & 0x3FFFFF)
    #endif
#else
    typedef enum {
        APP_E_BUTTONS_BUTTON_1
    } APP_teButtons;

    #define APP_BUTTONS_NUM                     (1UL)
    #define APP_BUTTONS_BUTTON_1            (4)

    #define APP_BUTTONS_DIO_MASK                (1 << APP_BUTTONS_BUTTON_1)
#endif
#endif

typedef enum {
    E_INTERRUPT_UNKNOWN,
    E_INTERRUPT_BUTTON,
    E_INTERRUPT_WAKE_TIMER_EXPIRY
} teInterruptType;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC bool_t APP_bButtonInitialise(void);
extern PUBLIC void vManageWakeUponSysControlISR(teInterruptType eInterruptType);
PUBLIC uint32 APP_u32GetSwitchIOState(void);
PUBLIC void APP_cbTimerButtonScan(void *pvParam);
PUBLIC void APP_cbTimerButtonDelay(void *pvParam);



/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /*APP_BUTTONS_H*/
