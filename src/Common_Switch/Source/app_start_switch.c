/****************************************************************************
 *
 * MODULE:             JN-AN-1245
 *
 * COMPONENT:          app_start_remote.c
 *
 * DESCRIPTION:        ZLO Dimmmer Switch Application Initialisation and Startup
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
#include "app_events.h"
#include "app_zlo_switch_node.h"
#include "app_buttons.h"
#include <string.h>
#include "ZTimer.h"
#include "app_main.h"
#include "zps_gen.h"
#include "DebugExceptionHandlers_jn518x.h"
#ifdef APP_NTAG_ICODE
#include "ntag_nwk.h"
#include "app_ntag_icode.h"
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

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef DEBUG_START_UP
    #define TRACE_START FALSE
#else
    #define TRACE_START TRUE
#endif

#ifndef DEBUG_APP
#define TRACE_APP_MAIN   TRUE
#else
#define TRACE_APP_MAIN   TRUE
#endif

#define RAM_HELD 2

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PUBLIC void vAppMain(bool_t bColdStart);
PRIVATE void APP_vInitialise(bool_t bColdStart);
#ifdef SLEEP_ENABLE
    PRIVATE void vSetUpWakeUpConditions(bool_t bDeepSleep);
#endif

void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

static uint8 u8PowerMode = RAM_HELD;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
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
/***        Exported Functions                                            ***/
/****************************************************************************/
extern void OSA_TimeInit(void);

/****************************************************************************
 *
 * NAME: vAppMain
 *
 * DESCRIPTION:
 * Entry point for application from a cold start.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppMain(bool_t bColdStart)
{

    wwdt_config_t  config;

#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_RED, 0);
#endif

    /* Initialise exception handlers for debugging */
    vDebugExceptionHandlersInitialise();

    /* Catch resets due to watchdog timer expiry. Comment out to harden code. */
    WWDT_GetDefaultConfig(&config);

    if (((PMC->RESETCAUSE) & PMC_RESETCAUSE_WDTRESET_MASK) == PMC_RESETCAUSE_WDTRESET_MASK)
    {
        /* Enable the WWDT clock */
        CLOCK_EnableClock(kCLOCK_WdtOsc);
        RESET_PeripheralReset(kWWDT_RST_SHIFT_RSTn);
        WWDT_Deinit(WWDT);
        DBG_vPrintf(TRACE_APP_MAIN, "APP: Watchdog timer has reset device!\r\n");
        POWER_ClearResetCause();
        while(1)
        {
            volatile uint32 u32Delay;


#if 0
#if  (defined AQARA_OPPLE)
            for (u32Delay = 0; u32Delay < 100000; u32Delay++);
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_BOARD_LED1_PIN, 0);
            for (u32Delay = 0; u32Delay < 100000; u32Delay++);
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_BOARD_LED1_PIN, 1);

#else
            for (u32Delay = 0; u32Delay < 100000; u32Delay++);
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED1_PIN, 0);
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED2_PIN, 0);
            for (u32Delay = 0; u32Delay < 100000; u32Delay++);
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED1_PIN, 1);
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED2_PIN, 1);
#endif
#endif
        }
    }

    if(bColdStart)
    {
        // TODO /* idle task commences here */
        DBG_vPrintf(TRACE_APP_MAIN, "\r***********************************************\r\n");
        DBG_vPrintf(TRACE_APP_MAIN, "\rSWITCH NODE RESET                              \r\n");
        DBG_vPrintf(TRACE_APP_MAIN, "\r***********************************************\r\n");
    }

#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_RED, 1);
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_ORANGE, 0);
#endif
//    DBG_vPrintf(TRACE_START, "APP: Entering APP_vInitResources()\r\n");
    APP_vInitResources();

    /* Set IIC DIO lines to outputs */
    // TODO vAHI_DioSetDirection(0, IIC_MASK);
    // TODO vAHI_DioSetOutput(IIC_MASK, 0);

#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_ORANGE, 1);
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_RED, 0);
#endif
//    DBG_vPrintf(TRACE_START, "APP: Entering APP_vInitialise()\r\n");
    APP_vInitialise(bColdStart);

#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_RED, 1);
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_ORANGE, 0);
#endif

    /* Didn't start BDB using PDM data ? */
    if(bColdStart)
    {
#if (defined APP_NTAG_ICODE)
        DBG_vPrintf(TRACE_START, "\nAPP: Entering APP_vNtagPdmLoad()");
        if (FALSE == APP_bNtagPdmLoad())
#endif
        {
            DBG_vPrintf(TRACE_START, "APP: Entering BDB_vStart()\n");
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
            APP_vNtagStart(DIMMERSWITCH_SWITCH_ENDPOINT);
        }
    }
#else
    /* Not waking from deep sleep ? */
    if (0 == (u16AHI_PowerStatus() & (1 << 11)))
    {
        DBG_vPrintf(TRACE_START, "\nAPP: Entering APP_vNtagStart()");
        APP_vNtagStart(DIMMERSWITCH_SWITCH_ENDPOINT);
    }
#endif

#endif
#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_ORANGE, 1);
#endif

     /*switch to XTAL 32M for better timing accuracy*/
    SYSCON -> MAINCLKSEL    =  2;  /* 32 M XTAL */
    SystemCoreClockUpdate();
    OSA_TimeInit();
}


/****************************************************************************
 *
 * NAME: vAppRegisterPWRCallbacks
 *
 * DESCRIPTION:
 * Power manager callback.
 * Called to allow the application to register  sleep and wake callbacks.
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
 * NAME: APP_vInitialise
 *
 * DESCRIPTION:
 * Initialises Zigbee stack, hardware and application.
 *
 *
 * RETURNS:
 * void
 ****************************************************************************/
PRIVATE void APP_vInitialise(bool_t bColdStart)
{

    if(bColdStart)
    {
        #ifdef APP_ANALYZE
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_YELLOW, 0);
        #endif
        /* Initialise Power Manager even on non-sleeping nodes as it allows the
         * device to doze when in the idle task */
        #ifdef SLEEP_ENABLE
             u8PowerMode = RAM_HELD;
        #endif
#ifdef APP_LOW_POWER_API
            (void) PWR_ChangeDeepSleepMode(PWR_E_SLEEP_OSCON_RAMON);
            PWR_Init();
            PWR_vForceRadioRetention(TRUE);
#else
            PWRM_vInit(E_AHI_SLEEP_OSCON_RAMON);
            PWRM_vForceRadioRetention(TRUE);
#endif
        #ifdef APP_ANALYZE
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_YELLOW, 1);
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_GREEN, 0);
        #endif
        /* Initialise the Persistent Data Manager */
        PDM_eInitialise(1200, 63, NULL);
    }

#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_GREEN, 1);
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_YELLOW, 0);
#endif
    /* Initialise Protocol Data Unit Manager */
    PDUM_vInit();
    ZPS_vExtendedStatusSetCallback(vfExtendedStatusCallBack);

#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_YELLOW, 1);
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_GREEN, 0);
#endif
    /* Initialise application */
    APP_vInitialiseNode(bColdStart);
#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_GREEN, 1);
#endif
}


#ifdef SLEEP_ENABLE
/****************************************************************************
 *
 * NAME: vSetUpWakeUpConditions
 *
 * DESCRIPTION:
 *
 * Set up the wake up inputs while going to sleep.
 *
 * PARAMETERS:      Name            RW  Usage
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vSetUpWakeUpConditions(bool_t bDeepSleep)
{
   /*
    * Set the DIO with the right edges for wake up
    * */

    /*Set the LED to inputs to reduce power consumption */
    /*the following pins are connected to LEDs hence drive them low*/
    APP_vSetLED(LED1, 0);
#if  (defined AQARA_OPPLE)
#else
    APP_vSetLED(LED2, 0);
    APP_vSetLED(LED3, 0);
#endif

    DBG_vPrintf(TRACE_START, "\rSet wake conditions: Buttons:%08x Mask:%08x\n", APP_BUTTONS_DIO_MASK, APP_BUTTONS_DIO_MASK_FOR_DEEP_SLEEP);

    #ifdef DEEP_SLEEP_ENABLE
    if(bDeepSleep)
    {
#ifdef APP_LOW_POWER_API
        PWR_vWakeUpConfig(APP_BUTTONS_DIO_MASK_FOR_DEEP_SLEEP);
#else
        PWRM_vWakeUpConfig(APP_BUTTONS_DIO_MASK_FOR_DEEP_SLEEP);
#endif
    }
    else
    #endif
    {
#ifdef APP_LOW_POWER_API
        PWR_vWakeUpConfig(APP_BUTTONS_DIO_MASK);
#else
        PWRM_vWakeUpConfig(APP_BUTTONS_DIO_MASK);
#endif
    }
}


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
     DBG_vPrintf(TRUE,"\rGoing to sleep..\n");
     #ifdef DEEP_SLEEP_ENABLE
         //DBG_vPrintf(TRACE_START,"Sleeping...bGoingDeepSleep = %d\n", bGoingDeepSleep());
         if(bGoingDeepSleep())
         {
        	 DBG_vPrintf(TRUE,"\rGoing to DEEP sleep..\n");
             u8PowerMode = 0;
         }
         else
         {
             u8PowerMode = RAM_HELD;
#if (defined SLEEP_MIN_RETENTION) && (defined CLD_OTA) && (defined OTA_CLIENT)
             vSetOTAPersistedDatForMinRetention();
#endif
         }
    #else
         u8PowerMode = RAM_HELD
    #endif

    /* If the power mode is with RAM held do the following
     * else not required as the entry point will init everything*/
    if(u8PowerMode == RAM_HELD)
    {
       vAppApiSaveMacSettings();
    }

    /* Set up wake up input */
    #ifdef DEEP_SLEEP_ENABLE
        vSetUpWakeUpConditions(bGoingDeepSleep());
    #else
        vSetUpWakeUpConditions(FALSE);
    #endif

    //DBG_vPrintf(TRUE,"\r\nSTART: PreSleep() EXIT, Deep = %d\r\n\r\n", bGoingDeepSleep());
   DBG_vPrintf(TRUE,"\rSTART: PreSleep() EXIT, Deep = %d\n");

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
 * Wakeup call back by  power manager after the controller wakes up from sleep.
 *
 ****************************************************************************/
#ifdef APP_LOW_POWER_API
static void Wakeup(void)
#else
PWRM_CALLBACK(Wakeup)
#endif
{
    /* If the power status is OK and RAM held while sleeping
     * restore the MAC settings
     * */
    if( (PMC->RESETCAUSE & ( PMC_RESETCAUSE_WAKEUPIORESET_MASK |
            PMC_RESETCAUSE_WAKEUPPWDNRESET_MASK ) ) && (u8PowerMode == RAM_HELD) )
    {

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
//        DBG_vPrintf(TRACE_START,"Waking..\n");
       /* Restore Mac settings (turns radio on) */
        vAppApiRestoreMacSettings();
//        DBG_vPrintf(TRACE_START, "\nAPP: MAC settings restored");

        /* Define HIGH_POWER_ENABLE to enable high power module */
        #ifdef HIGH_POWER_ENABLE
           //TODO vAHI_HighPowerModuleEnable(TRUE, TRUE);
        #endif

        APP_vInitResources();

        /* Activate the SleepTask, that would start the SW timer and polling would continue */
        APP_vStartUpHW();
        /* ToDO: why to Initialize buttons again as after waking up ,
         * as the button press is not detected
         */
        APP_bButtonInitialise();
#endif
    }
    else
    {
#ifdef APP_ANALYZE
        /* Analyze */
        GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_ORANGE, 0);
#endif
    }
    //DBG_vPrintf(TRUE, "APP: Woken up (CB)\n");
    DBG_vPrintf(TRUE, "\rAPP: Warm Waking powerStatus = 0x%x", PMC->RESETCAUSE);
}
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
            u8PowerMode = 0;
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
    DBG_vPrintf(TRACE_START,"ERROR: Extended status 0x%x\r\n", eExtendedStatus);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
