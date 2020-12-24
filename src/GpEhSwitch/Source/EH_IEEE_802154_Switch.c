/*****************************************************************************
 *
 * MODULE:            JN-AN-1245
 *
 * COMPONENT:          EH_IEEE_802154_Switch.c
 *
 * DESCRIPTION:        ZigBee GP switch main file
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
#include "jendefs.h"
#include "string.h"
#include <dbg.h>
#include "MMAC.h"
#include "EH_IEEE_802154_Switch.h"
#include "EH_Switch_Configurations.h"
#include "EH_IEEE_Features.h"
#include "EH_IEEE_Commands.h"
#include "EH_Button.h"
#include "app_nvm.h"
#include "fsl_power.h"
#include "fsl_gpio.h"
#include "DebugExceptionHandlers_jn518x.h"
#include "radio.h"
#ifdef OM15082
#include "app_main.h"
#include "ZTimer.h"
#include "fsl_os_abstraction.h"
#include "fsl_os_abstraction_bm.h"
#endif

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Required to set BrownOut */
#if 0
#define VBO_MASK            0x0000000eUL
#define SYSCON_SYS_IM_ADDR  0x0200000cUL
#define SYSCON_VBOCTRL_ADDR 0x02000044UL
#endif

/* DIO Used for Cap Switching */
#define DIO_1 (0x00000002)
#define DIO_2 (0x00000004)
#define DIO_3 (0x00000008)
/* Mask for all Cap switching buttons */
#define DIO_CAP_MASK (DIO_1|DIO_2|DIO_3)

#define BUTTON_PRESSED       0x00               /*No DIO set */
#define BUTTON_RELEASED      DIO_8              /*DIO 8 set when button released*/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/* Brown Out */
/*
typedef enum
{
    E_BO_TRIP_1V95 = 0,
    E_BO_TRIP_2V0  = 2,
    E_BO_TRIP_2V1  = 4,
    E_BO_TRIP_2V2  = 6,
    E_BO_TRIP_2V3  = 8,
    E_BO_TRIP_2V4  = 10,
    E_BO_TRIP_2V7  = 12,
    E_BO_TRIP_3V0  = 14
} teBrownOutTripVoltage;
*/
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
#ifdef OM15082
PUBLIC void APP_vInitResources(void);
#endif
volatile bool_t bEnableSleep = TRUE;
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

gpio_pin_config_t led_config = {
    kGPIO_DigitalOutput, 0,
};

/* bTxComplete is used to flag TX completion from ISR to main code */
volatile bool_t bTxComplete;

/*bRxComplete is used to flag RX completion from ISR to main code */
volatile bool_t bRxComplete;

extern const uint8_t gUseRtos_c;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
extern void OSA_TimeInit(void);

/****************************************************************************
 *
 * NAME: vSendMACFrame
 *
 * DESCRIPTION:
 * Transmits MAC frame and updates persistent data
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/

PUBLIC void vSendMACFrame( bool bUpdatePersistentData)
{
    uint8 u8TxRepeats = 0;
    if(bUpdatePersistentData)
    {
        /* update persistence data and button press count*/
#ifdef GPD_SUPPORT_PERSISTENT_DATA
        bAPP_NVMWriteBlock(  sizeof(tGPD_PersistantData),
                             (uint8 *)&sGPDPersistentData);
#endif
    }

    /* Set Dedicated single channel */
    vMMAC_SetChannelAndPower(sGPDPersistentData.u8Channel, 127);
    do
    {
        DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\n vSendMACFrame channel = %d  ", sGPDPersistentData.u8Channel );

        /* Set transmit flag to false */
        bTxComplete = FALSE;
        /* Start Transmit with no CCA or ACK */
        vMMAC_StartMacTransmit(&sGPD_MACFrame, E_MMAC_TX_START_NOW
                                           | E_MMAC_TX_NO_AUTO_ACK
                                           | E_MMAC_TX_NO_CCA);
        /* While call-back not reset flag */
        while (bTxComplete != TRUE)
        {
            /*
             * suspends CPU operation when the system is idle or puts the device to
             * sleep if there are no activities in progress
             */
            //POWER_SetLowPower( PM_SLEEP_0 , &pmconfig);
        POWER_EnterSleep();
        }
        /* add one to repeat value */
        u8TxRepeats++;
    /* Keep transmitting until counter reached or power runs out
     * or if powered by mains for a max of 20 packets */
    } while (u8TxRepeats < sGPDPersistentData.u8NoOfTransPerChannel);

}
/****************************************************************************
 *
 * NAME: vEnableRxAndReceive
 *
 * DESCRIPTION:
 * Turns on Rx and receives data
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
#if defined( GPD_RX_AFTER_TX) || defined(GPD_RX_ENABLE)
void vEnableRxAndReceive(void)
{
#ifdef OM15082
    uint32 u32StartTime;
#endif

    /* MAC clock gives time in 16us resolution */
    /* keep time as 10 milli seconds  */

    bRxComplete = FALSE;
    /* Set time corresponding to MAC clock */
    vMMAC_RadioToOffAndWait();
    vMMAC_StartMacReceive(&sGPD_MACReceivedFrame, E_MMAC_RX_START_NOW
                            |E_MMAC_RX_NO_AUTO_ACK
                            );
#ifndef OM15082
/* While call-back not reset flag */
    while( bRxComplete == FALSE)
    {
        //POWER_SetLowPower( PM_SLEEP_0 , &pmconfig);
    POWER_EnterSleep();
    }
    vHandleRxMacFrame();

#else
    /* Don't use ZTIMER or call into ZTIMER_vTask(). We are already in a
     * callback from ZTIMER_vTask() and calling it from here messes up the
     * timing spectacularly. So use OSA time function directly. */
    u32StartTime = OSA_TimeGetMsec();

    while (   (FALSE == bRxComplete)
           && ((OSA_TimeGetMsec() - u32StartTime) < 250)
          )
    {
        /* Tight loop */
    }

    if(bRxComplete)
    {
        /* Process Received frame */
        DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\n bRxComplete  vHandleRxMacFrame " );
        vHandleRxMacFrame();
    }
    else
    {
        DBG_vPrintf(DBG_DEVICE_SWITCH, "\r\n MAC receive Timer Expired " );
    }

    vMMAC_RadioOff();
#endif
}
#endif
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vInterruptFired
 *
 * DESCRIPTION:
 * ISR for MAC RX/TX interrupts
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vInterruptFired(uint32 u32InterruptBitmap)
{
    /* Is the interrupt for Transmit completed */
    if (u32InterruptBitmap & E_MMAC_INT_TX_COMPLETE)
    {
        bTxComplete = TRUE;
    }
    else
    {
        /* Is the interrupt for Rx completed */
        if((u32InterruptBitmap & E_MMAC_INT_RX_COMPLETE))
        {
            bRxComplete = TRUE;
        }
    }
}


/****************************************************************************
 *
 * NAME: AppWarmStart
 *
 * DESCRIPTION:
 * Entry point for application from boot loader. Simply jumps to AppColdStart
 * as, in this instance, application will never warm start.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppMain(void)
{

#ifdef OM15082
    bool bClearPDM;
#endif

    /* Initialise exception handlers for debugging */
    vDebugExceptionHandlersInitialise();

#ifdef OM15082
    /* Initialize DIO on OM15082 switch*/
    APP_vInitResources();
    bClearPDM =
#endif

    bButtonInitialize();

    /* Provide fake temperature to radio to ensure calibration settings are saved */
    vRadio_Temp_Update(40);
    /* Enable MAC, and enable interrupts */
    vMMAC_Enable();
    vMMAC_ConfigureRadio();
    vMMAC_EnableInterrupts(vInterruptFired);

#ifdef OM15082
    if(bClearPDM)
    {
        vClearPersistentData(TRUE);
    }
#else

    #ifdef GP_LEVEL_CONTROL
        vHandleLevelControlButtonPress();
    #else
        vHandleEHButtonPress();
    #endif
#endif

}


/****************************************************************************
 *
 * NAME: APP_vSetUpHardware
 *
 * DESCRIPTION:
 * Set up interrupts
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vSetUpHardware(void)
{
    /* Enable DMA access to RAM (assuming default configuration and MAC
     * buffers not in first block of RAM) */
    *(volatile uint32 *)0x40001000 = 0xE000F733;
    *(volatile uint32 *)0x40001004 = 0x109;
    /* Board pin, clock, debug console init */
    BOARD_InitHardware();

    /* Initialise output LED GPIOs */
    GPIO_PinInit(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED1_PIN, &led_config);
    GPIO_PinInit(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED1_PIN, &led_config);

    SYSCON -> MAINCLKSEL       = 3;  /* 32 M FRO */
    SystemCoreClockUpdate();
    OSA_TimeInit();
    /* Initialise buttons;
     */
    bButtonInitialize();

}

/****************************************************************************
 *
 * NAME: APP_vMainLoop
 *
 * DESCRIPTION:
 * Main application loop
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void main_task (uint32_t parameter)
{
    /* e.g. osaEventFlags_t ev; */
    static uint8_t initialized = FALSE;

    if(!initialized)
    {
        /* place initialization code here... */
        /*myEventId = OSA_EventCreate(TRUE);*/
        initialized = TRUE;
        vAppMain();

    }

    while (TRUE)
    {

        ZTIMER_vTask();

        /*
         * suspends CPU operation when the system is idle or puts the device to
         * sleep if there are no activities in progress
         */
        //POWER_SetLowPower( PM_SLEEP_0 , &pmconfig);
    POWER_EnterSleep();
        if(!gUseRtos_c)
        {
            break;
        }
    }
}

/****************************************************************************
 *
 * NAME: hardware_init
 *
 * DESCRIPTION:
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void hardware_init(void)
{
    static bool_t bColdStarted = FALSE;

    if (FALSE == bColdStarted)
    {
        APP_vSetUpHardware();
        bColdStarted = TRUE;
    }
}
