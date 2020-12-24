/*****************************************************************************
 *
 * MODULE:             JN-AN-1245
 *
 * COMPONENT:          app_start_remote.c
 *
 * DESCRIPTION:        ZLO Demo: System Initialisation - Implementation
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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include "fsl_wwdt.h"
#include "fsl_iocon.h"
#include "fsl_gpio.h"
#include "fsl_power.h"
#include "pin_mux.h"
#include "app.h"

#ifdef APP_LOW_POWER_API
#include "PWR_interface.h"
#else
#include "pwrm.h"
#endif

#include "pdum_nwk.h"
#include "pdum_apl.h"
#include "pdum_gen.h"
#include "PDM.h"
#include "dbg.h"
#include "zps_apl_af.h"
#include "AppApi.h"
#include "ZTimer.h"
#include "zlo_controller_node.h"
#include "app_buttons.h"
// TODO #include "low_bat_indicator.h"
#include "app_led_control.h"
#include "app_main.h"
#include <string.h>
#include "bdb_api.h"
#include "app_uart.h"
#include "fsl_power.h"
#include "DebugExceptionHandlers_jn518x.h"
#ifdef APP_NTAG_ICODE
#include "ntag_nwk.h"
#include "app_ntag_icode.h"
#include "zps_gen.h"
#if (JENNIC_CHIP_FAMILY == JN518x)
#if (defined SLEEP_MIN_RETENTION) && (defined CLD_OTA) && (defined OTA_CLIENT)
#include "app_ota_client.h"
#endif
#endif
#endif
#if (ZIGBEE_USE_FRAMEWORK != 0) && (defined SLEEP_MIN_RETENTION)
#include "SecLib.h"
#include "RNG_Interface.h"
#include "MemManager.h"
#include "TimersManager.h"
#endif
extern void OSA_TimeInit(void);
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define RAM_HELD 2

#ifndef DEBUG_APP
#define TRACE_APP   FALSE
#else
#define TRACE_APP   TRUE
#endif

#ifndef DEBUG_START_UP
#define TRACE_START FALSE
#else
#define TRACE_START TRUE
#endif

#ifndef DEBUG_SLEEP
#define TRACE_SLEEP FALSE
#else
#define TRACE_SLEEP TRUE
#endif

#define HALT_ON_EXCEPTION   FALSE

#define POWER_BTN (1)
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PUBLIC void vAppMain(bool_t bColdStart);
PRIVATE void APP_vInitialise(bool_t bColdStart);
PRIVATE void vSetUpWakeUpConditions(void);

#if (defined PDM_EEPROM)
#if TRACE_APP
PRIVATE void vPdmEventHandlerCallback(uint32 u32EventNumber, PDM_eSystemEventCode eSystemEventCode);
#endif
#endif
void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus);
/**
 * Power manager Callbacks
 */
#ifdef APP_LOW_POWER_API
static void PreSleep(void);
static void Wakeup(void);
#else
static PWRM_DECLARE_CALLBACK_DESCRIPTOR(PreSleep);
static PWRM_DECLARE_CALLBACK_DESCRIPTOR(Wakeup);
#endif

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

bool_t bDeepSleep = FALSE;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: PreSleep
 *
 * DESCRIPTION:
 *
 * PreSleep call back by the power manager before the controller put into sleep.
 *
 * PARAMETERS:      Name            RW  Usage
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
#ifdef APP_LOW_POWER_API
static void PreSleep(void)
#else
PWRM_CALLBACK(PreSleep)
#endif
{
    DBG_vPrintf(TRACE_SLEEP,"Sleeping mode %d...\n", bDeepSleep);
    /* If the power mode is with RAM held do the following
     * else not required as the entry point will init everything*/
    if(!bDeepSleep)
    {
        /* sleep memory held */

        vAppApiSaveMacSettings();

        #if (defined SLEEP_MIN_RETENTION) && (defined CLD_OTA) && (defined OTA_CLIENT)
             vSetOTAPersistedDatForMinRetention();
        #endif
    }

    /* Set up wake up dio input */
    vSetUpWakeUpConditions();

    /* Disable debug */
    DbgConsole_Deinit();

    /* Minimize GPIO power consumption */
    BOARD_SetPinsForPowerMode();
}

/****************************************************************************
 *
 * NAME: Wakeup
 *
 * DESCRIPTION:
 *
 * Wakeup call back by the power manager after the controller wakes up from sleep.
 *
 * PARAMETERS:      Name            RW  Usage
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
#ifdef APP_LOW_POWER_API
static void Wakeup(void)
#else
PWRM_CALLBACK(Wakeup)
#endif
{
    /*Stabilise the oscillator*/


    if( (PMC->RESETCAUSE & ( PMC_RESETCAUSE_WAKEUPIORESET_MASK |
            PMC_RESETCAUSE_WAKEUPPWDNRESET_MASK ) ) && ( !bDeepSleep) )
    {
        /* Don't use RTS/CTS pins on UART0 as they are used for buttons */
        APP_vSetUpHardware();

#ifdef SLEEP_MIN_RETENTION
#if (ZIGBEE_USE_FRAMEWORK != 0)
        RNG_Init();
        SecLib_Init();
        MEM_Init();
        TMR_Init();
#endif
        vAppMain(FALSE);
        APP_vStartUpHW();
#else
//        DBG_vPrintf(TRACE_APP, "\nAPP: Warm Waking powerStatus = 0x%x", PMC->RESETCAUSE);
        /* If the power status is OK and RAM held while sleeping
         * restore the MAC settings
         * */

        // Restore Mac settings (turns radio on)
        vAppApiRestoreMacSettings();
//        DBG_vPrintf(TRACE_APP, "\nAPP: MAC settings restored");

        APP_vInitResources();

        /* Activate the SleepTask, that would start the SW timer and polling would continue
         * */
        APP_vStartUpHW();
        /* ToDO: why to Initialize buttons again as after waking up ,
         * as the button press is not detected
         */
        APP_bButtonInitialise();

    #ifdef OM15082
        UART_vInit();
        UART_vRtsStartFlow();
    #endif
#endif
    }

//    DBG_vPrintf(TRACE_SLEEP, "\n\nAPP: Woken up (CB)");
}

/****************************************************************************
 *
 * NAME: vAppMain
 *
 * DESCRIPTION:
 *
 * Entry point for application from a cold start.
 *
 * PARAMETERS:      Name            RW  Usage
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppMain(bool_t bColdStart)
{

    wwdt_config_t  config;
    uint32       u32ResetCause;

    /* Initialise exception handlers for debugging */
    vDebugExceptionHandlersInitialise();

    /* Catch resets due to watchdog timer expiry */
    WWDT_GetDefaultConfig(&config);
    u32ResetCause = PMC->RESETCAUSE;
    if ((u32ResetCause & PMC_RESETCAUSE_WDTRESET_MASK) == PMC_RESETCAUSE_WDTRESET_MASK)
    {
        /* Enable the WWDT clock */
        CLOCK_EnableClock(kCLOCK_WdtOsc);
        RESET_PeripheralReset(kWWDT_RST_SHIFT_RSTn);
        WWDT_Deinit(WWDT);
        DBG_vPrintf(TRACE_APP, "APP: Watchdog timer has reset device!\r\n");
        POWER_ClearResetCause();
        while(1)
        {
            volatile uint32 u32Delay;

            for (u32Delay = 0; u32Delay < 100000; u32Delay++);
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED1_PIN, 0);
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED2_PIN, 0);
            for (u32Delay = 0; u32Delay < 100000; u32Delay++);
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED1_PIN, 1);
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED2_PIN, 1);
        }
    }

    /* idle task commences here */
    DBG_vPrintf(TRACE_APP, "***********************************************\n");
    DBG_vPrintf(TRACE_APP, "REMOTE NODE RESET                              \n");
    DBG_vPrintf(TRACE_APP, "***********************************************\n");
    DBG_vPrintf(TRACE_APP, "u32ResetCause = 0x%x\n", u32ResetCause);

//    DBG_vPrintf(TRACE_APP, "APP: Entering APP_vInitResources()\n");
    APP_vInitResources();

//    DBG_vPrintf(TRACE_APP, "APP: Entering APP_vInitialise()\n");
    APP_vInitialise(bColdStart);

    /* Didn't start BDB using PDM data ? */
    if(bColdStart)
    {
#if (defined APP_NTAG_ICODE)
        DBG_vPrintf(TRACE_APP, "\nAPP: Entering APP_vNtagPdmLoad()");
        if (FALSE == APP_bNtagPdmLoad())
#endif
        {
            DBG_vPrintf(TRACE_APP, "APP: Entering BDB_vStart()\n");
            BDB_vStart();
        }
    }
    else
    {
        BDB_vRestart();
    }

#ifdef APP_NTAG_ICODE
#if (JENNIC_CHIP_FAMILY == JN518x)
        if(bColdStart)
        {
            /* Not waking from deep sleep ? */
            if (0 == (POWER_GetResetCause() & (RESET_WAKE_DEEP_PD | RESET_WAKE_PD)))
            {
                DBG_vPrintf(TRACE_START, "\nAPP: Entering APP_vNtagStart()");
                APP_vNtagStart(COLORSCENECONTROLLER_REMOTE_ENDPOINT);
            }
        }
#else
        /* Not waking from deep sleep ? */
        if (0 == (u16AHI_PowerStatus() & (1 << 11)))
        {
            DBG_vPrintf(TRACE_START, "\nAPP: Entering APP_vNtagStart()");
            APP_vNtagStart(COLORSCENECONTROLLER_REMOTE_ENDPOINT);
        }
#endif

#endif

    /* Update LED's to reflect shift level */
     APP_vSetLedsMask(0);
     APP_vBlinkLeds(0);

    /*switch to XTAL 32M for better timing accuracy*/
    SYSCON -> MAINCLKSEL    =  2;  /* 32 M XTAL */
    SystemCoreClockUpdate();
    OSA_TimeInit();
}

/****************************************************************************
 *
 * NAME: vAppMain
 *
 * DESCRIPTION:
 *
 * Power manager callback.
 * Called to allow the application to register
 * sleep and wake callbacks.
 *
 * PARAMETERS:      Name            RW  Usage
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void vAppRegisterPWRCallbacks(void)
{
#ifdef APP_LOW_POWER_API
    PWR_RegisterLowPowerEnterCallback(PreSleep);
    PWR_RegisterLowPowerExitCallback(Wakeup);
#else
    PWRM_vRegisterPreSleepCallback(PreSleep);
    PWRM_vRegisterWakeupCallback(Wakeup);
#endif
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vSetUpWakeUpConditions
 *
 * DESCRIPTION:
 *
 * Set up the wake up inputs while going to sleep  or preserve as an output
 * to drive the LHS led indicator if never sleeping
 *
 ****************************************************************************/

PRIVATE void vSetUpWakeUpConditions(void)
{
    APP_vSetLedsMask(0);                    /* ensure leds are off */
    DBG_vPrintf(TRACE_START, "Going to sleep: Buttons:%08x Mask:%08x\n", APP_BUTTONS_DIO_MASK & (~(1 << APP_BOARD_SW4_PIN)), APP_BUTTONS_DIO_MASK_FOR_DEEP_SLEEP & (~(1 << APP_BOARD_SW4_PIN)));

    if(bDeepSleep)
    {
#ifdef APP_LOW_POWER_API
        PWR_vWakeUpConfig(APP_BUTTONS_DIO_MASK_FOR_DEEP_SLEEP & (~(1 << APP_BOARD_SW4_PIN)));
#else
        PWRM_vWakeUpConfig(APP_BUTTONS_DIO_MASK_FOR_DEEP_SLEEP & (~(1 << APP_BOARD_SW4_PIN)));
#endif
    }
    else
    {
#ifdef APP_LOW_POWER_API
        PWR_vWakeUpConfig(APP_BUTTONS_DIO_MASK & (~(1 << APP_BOARD_SW4_PIN)));
#else
        PWRM_vWakeUpConfig(APP_BUTTONS_DIO_MASK & (~(1 << APP_BOARD_SW4_PIN)));
#endif
    }
}


/****************************************************************************
 *
 * NAME: APP_vInitialise
 *
 * DESCRIPTION:
 *
 * Initialises Zigbee stack, hardware and application.
 *
 * PARAMETERS:      Name            RW  Usage
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void APP_vInitialise(bool_t bColdStart)
{
if(bColdStart)
{
    /* Initialise Power Manager even on non-sleeping nodes as it allows the
     * device to doze when in the idle task */
#ifdef APP_LOW_POWER_API
    (void) PWR_ChangeDeepSleepMode(PWR_E_SLEEP_OSCON_RAMON);
    PWR_Init();
    PWR_vForceRadioRetention(TRUE);
#else
    PWRM_vInit(E_AHI_SLEEP_OSCON_RAMON);
    PWRM_vForceRadioRetention(TRUE);
#endif
    /* Initialise the Persistent Data Manager */
    PDM_eInitialise(1200, 63, NULL);
}

#if (defined PDM_EEPROM)
#if TRACE_APP
    PDM_vRegisterSystemCallback(vPdmEventHandlerCallback);
#endif
#endif

    /* Initialise Protocol Data Unit Manager */
    PDUM_vInit();

    ZPS_vExtendedStatusSetCallback(vfExtendedStatusCallBack);

#ifdef OM15082
    UART_vInit();
    UART_vRtsStartFlow();
#endif

    /* initialise application */
    APP_vInitialiseNode(bColdStart);

}

#if (defined PDM_EEPROM)
#if TRACE_APP
PRIVATE void vPdmEventHandlerCallback(uint32 u32EventNumber, PDM_eSystemEventCode eSystemEventCode)
{
    DBG_vPrintf(TRUE, "\nPDM: Event %x:%d", u32EventNumber, eSystemEventCode);
}
#endif
#endif

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
        if( PMC->RESETCAUSE & ( PMC_RESETCAUSE_WAKEUPIORESET_MASK |
                         PMC_RESETCAUSE_WAKEUPPWDNRESET_MASK ))
        {
            bDeepSleep = TRUE;
        }
        APP_vSetUpHardware();
#ifdef APP_LOW_POWER_API
        PWR_vColdStart();
#else
        PWRM_vColdStart();
#endif
        bColdStarted = TRUE;
    }
}


/****************************************************************************
 *
 * NAME: vfExtendedStatusCallBack
 *
 * DESCRIPTION:
 *
 * ZPS extended error callback .
 *
 * PARAMETERS:      Name            RW  Usage
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus)
{
    DBG_vPrintf(TRACE_START,"ERROR: Extended status 0x%x\n", eExtendedStatus);
}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
