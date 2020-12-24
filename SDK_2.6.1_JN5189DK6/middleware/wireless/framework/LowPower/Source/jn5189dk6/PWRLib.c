/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2017, 2019-2020 NXP
* All rights reserved.
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/*****************************************************************************
 *                               INCLUDED HEADERS                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the headers that this module needs to include.    *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
#include "EmbeddedTypes.h"
#include "fsl_device_registers.h"
#include "board.h"
#include "PWRLib.h"
#include "PWR_Configuration.h"
#include "TimersManager.h"
#include "fsl_os_abstraction.h"
#include "GPIO_Adapter.h"
#include "fsl_power.h"
#include "rom_lowpower.h"
#include "fsl_flash.h"
#if gSupportBle
#include "ble_general.h"
#include "controller_interface.h"
#endif

#include "rom_api.h"
#include "fsl_debug_console.h"

extern uint32_t                   __base_RAM0;
extern uint32_t                   _end_fw_retention;
extern uint32_t                   _end_boot_resume_stack;

#define RESUME_STACK_POINTER      ((uint32_t)&_end_boot_resume_stack)
#define PWR_JUMP_BUF_SIZE         10

typedef unsigned long PWR_Register;
typedef PWR_Register PWR_jmp_buf[PWR_JUMP_BUF_SIZE];

void PWR_longjmp(PWR_jmp_buf __jmpb, int __retval);
int PWR_setjmp(PWR_jmp_buf __jmpb);

PWR_jmp_buf                   pwr_CPUContext;


#if defined(__IAR_SYSTEMS_ICC__) || defined(__GNUC__)
    #define GET_MSP(x) asm volatile("MRS %0, MSP" : "=r" (x))
#else
    #error "must define get_MSP"
#endif

/*****************************************************************************
 *                             PRIVATE MACROS                                *
 *---------------------------------------------------------------------------*
 * Add to this section all the access macros, registers mappings, bit access *
 * macros, masks, flags etc ...                                              *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
#ifndef BIT
#define BIT(x)                        (1 << (x))
#endif

/* The RAM starts at 0x04000000 but for our purposes we only care
   about it from __base_RAM0 onwards, as the area before this is used by boot
   code data that does not have to be preserved through sleep. Thus the start
   address for bank 0 is taken as __base_RAM0 */
#define PWR_SRAM_BANK0_BASE          (0x04000000)
#define PWR_SRAM_BANK0_START_ADDR    ((uint32_t)&__base_RAM0)
#define PWR_SRAM_BANK1_START_ADDR    (PWR_SRAM_BANK0_BASE + ( 16 << 10 ))
#define PWR_SRAM_BANK2_START_ADDR    (PWR_SRAM_BANK1_START_ADDR + ( 16 << 10 ))
#define PWR_SRAM_BANK3_START_ADDR    (PWR_SRAM_BANK2_START_ADDR + ( 16 << 10 ))
#define PWR_SRAM_BANK4_START_ADDR    (PWR_SRAM_BANK3_START_ADDR + ( 16 << 10 ))
#define PWR_SRAM_BANK5_START_ADDR    (PWR_SRAM_BANK4_START_ADDR + (  8 << 10 ))
#define PWR_SRAM_BANK6_START_ADDR    (PWR_SRAM_BANK5_START_ADDR + (  8 << 10 ))
#define PWR_SRAM_BANK7_START_ADDR    (PWR_SRAM_BANK6_START_ADDR + (  4 << 10 ))
#define PWR_SRAM_BANK7_END_ADDR      (PWR_SRAM_BANK7_START_ADDR + (  4 << 10 ))

const uint32_t pm_ram_bank_addr[] =
{
    PWR_SRAM_BANK0_START_ADDR,
    PWR_SRAM_BANK1_START_ADDR,
    PWR_SRAM_BANK2_START_ADDR,
    PWR_SRAM_BANK3_START_ADDR,
    PWR_SRAM_BANK4_START_ADDR,
    PWR_SRAM_BANK5_START_ADDR,
    PWR_SRAM_BANK6_START_ADDR,
    PWR_SRAM_BANK7_START_ADDR,
};


/*****************************************************************************
 *                               PRIVATE VARIABLES                           *
 *---------------------------------------------------------------------------*
 * Add to this section all the variables and constants that have local       *
 * (file) scope.                                                             *
 * Each of this declarations shall be preceded by the 'static' keyword.      *
 * These variables / constants cannot be accessed outside this module.       *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
static uint8_t mPWRLib_DeepSleepMode = 0;

/*****************************************************************************
 *                               PUBLIC VARIABLES                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the variables and constants that have global      *
 * (project) scope.                                                          *
 * These variables / constants can be accessed outside this module.          *
 * These variables / constants shall be preceded by the 'extern' keyword in  *
 * the interface header.                                                     *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

volatile PWR_WakeupReason_t PWRLib_MCU_WakeupReason;


/*****************************************************************************
 *                           PRIVATE FUNCTIONS PROTOTYPES                    *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions prototypes that have local (file)   *
 * scope.                                                                    *
 * These functions cannot be accessed outside this module.                   *
 * These declarations shall be preceded by the 'static' keyword.             *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

/*****************************************************************************
 *                                PRIVATE FUNCTIONS                          *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions that have local (file) scope.       *
 * These functions cannot be accessed outside this module.                   *
 * These definitions shall be preceded by the 'static' keyword.              *
 *---------------------------------------------------------------------------*
*****************************************************************************/
#if ROM_API_CALL
#include "lowpower_jn5180.h"
extern void Chip_LOWPOWER_PowerCycleCpuAndFlash(LPC_LOWPOWER_T *p_lowpower_cfg);
LPC_LOWPOWER_T p_lowpower_cfg;
#endif

#if defined(__IAR_SYSTEMS_ICC__)
__ramfunc
#elif  defined(__GNUC__)
__attribute__ ((section (".ramfunc")))
#endif
void PWRLib_EnterSleep_WithFlashControllerPowerDown(void)
{
#if (gPWR_FlashControllerPowerDownInWFI == 2)
    uint32_t ldoflashcore_prev_voltage = 0;
#endif

    /* Clear the DONE status bit */
    FLASH->INT_CLR_STATUS = FLASH_DONE;

    /* Power down the flash */
    FLASH->CMD = FLASH_CMD_POWERDOWN;

    /* Wait for flash power down is done before turning OFF LDO flash */
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");

#if (gPWR_FlashControllerPowerDownInWFI == 2)
    {
       uint32_t ldo_flash_core_reg       = PMC -> LDOFLASHCORE;
       uint32_t ldoflashcore_new_voltage = (p_lowpower_cfg->VOLTAGE & LOWPOWER_VOLTAGE_LDO_FLASH_CORE_MASK) >> LOWPOWER_VOLTAGE_LDO_FLASH_CORE_INDEX;

       /* Switch OFF the clock first */
       SYSCON -> CLOCK_CTRL &= ~SYSCON_CLOCK_CTRL_FLASH48MHZ_ENA_MASK;
       /* Save and set new LDO Flash Core voltage */
       if ( PMC -> CTRL & PMC_CTRL_SELLDOVOLTAGE_MASK )
       {
           ldoflashcore_prev_voltage  = (ldo_flash_core_reg & PMC_LDOFLASHCORE_VADJ_2_MASK) >> PMC_LDOFLASHCORE_VADJ_2_SHIFT;
           PMC -> LDOFLASHCORE = (ldo_flash_core_reg & (~PMC_LDOFLASHCORE_VADJ_2_MASK)) | ( ldoflashcore_new_voltage << PMC_LDOFLASHCORE_VADJ_2_SHIFT );
       }
       else
       {
           ldoflashcore_prev_voltage  = (ldo_flash_core_reg & PMC_LDOFLASHCORE_VADJ_MASK) >> PMC_LDOFLASHCORE_VADJ_SHIFT;
           PMC -> LDOFLASHCORE = (ldo_flash_core_reg & (~PMC_LDOFLASHCORE_VADJ_MASK))   | ( ldoflashcore_new_voltage << PMC_LDOFLASHCORE_VADJ_SHIFT );
       }
    }
#else
    /* switch OFF the Flash controller switch to reduce power */
      SYSCON->CLOCK_CTRL &= ~SYSCON_CLOCK_CTRL_FLASH48MHZ_ENA_MASK;
#endif

    POWER_ENTER_SLEEP();

#if (gPWR_FlashControllerPowerDownInWFI == 2)
    {
       uint32_t ldo_flash_core_reg     = PMC -> LDOFLASHCORE;
       uint32_t delay_5us              = ((p_lowpower_cfg->CFG & LOWPOWER_CFG_LDOFLASHCORE_DELAY_MASK) >> LOWPOWER_CFG_LDOFLASHCORE_DELAY_INDEX) + 4 /* 19us */;   /* 4.75 us per unit */

        /* restore LDO FLASH CORE Voltage */
       if ( PMC -> CTRL & PMC_CTRL_SELLDOVOLTAGE_MASK )
           PMC -> LDOFLASHCORE = (ldo_flash_core_reg & (~PMC_LDOFLASHCORE_VADJ_2_MASK)) | ( ldoflashcore_prev_voltage << PMC_LDOFLASHCORE_VADJ_2_SHIFT );
       else
           PMC -> LDOFLASHCORE = (ldo_flash_core_reg & (~PMC_LDOFLASHCORE_VADJ_MASK))   | ( ldoflashcore_prev_voltage << PMC_LDOFLASHCORE_VADJ_SHIFT );

       for ( int i = delay_5us*11; i!=0 ; i--)
            __asm("nop");

       /* switch ON the clock back */
       SYSCON -> CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FLASH48MHZ_ENA_MASK;
    }
#else
    /* Restore Flash controller clock */
    SYSCON -> CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FLASH48MHZ_ENA_MASK;
#endif



    /* Power Up the flash (WAKE UP event) */
    FLASH->EVENT = FLASH_EVENT_WAKEUP(1);

#if 0
    __ISB();
    __DSB();
#endif

   /* Wait until Flash is powered up */
   while(!(FLASH-> INT_STATUS & FLASH_DONE ));

#if 0
   __asm("nop");
   __asm("nop");
   __asm("nop");
   __asm("nop");
#endif
}


/*****************************************************************************
 *                             PUBLIC FUNCTIONS                              *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions that have global (project) scope.   *
 * These functions can be accessed outside this module.                      *
 * These functions shall have their declarations (prototypes) within the     *
 * interface header file and shall be preceded by the 'extern' keyword.      *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
void PWRLib_Init(void)
{
}

/*---------------------------------------------------------------------------
 * Name: PWRLib_SetDeepSleepMode
 * Description:
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PWRLib_SetDeepSleepMode(uint8_t lpMode)
{
    mPWRLib_DeepSleepMode = lpMode;
#if gSupportBle
    if (lpMode == cPWR_DeepSleep_RamOffOsc32kOn ||
         lpMode == cPWR_PowerDown_RamRet ||
         lpMode == cPWR_DeepSleep_RamOffOsc32kOff)
        BLE_enable_sleep();
    else
        BLE_disable_sleep();
#endif
}

/*---------------------------------------------------------------------------
 * Name: PWRLib_GetDeepSleepMode
 * Description:
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PWRLib_GetDeepSleepMode(void)
{
    return mPWRLib_DeepSleepMode;
}


/*---------------------------------------------------------------------------
 * Name: PWRLib_MCUEnter_Sleep
 * Description: Puts the processor into Sleep .

                Mode of operation details:
                 - ARM core enters Sleep Mode
                 - ARM core is clock gated (HCLK = OFF)
                 - peripherals are functional

 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/

extern uint8_t mLPMFlag;
void PWRLib_MCU_Enter_Sleep(void)
{
    PWR_DBG_LOG("");
    OSA_DisableIRQGlobal();
#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
    BOARD_DbgIoSet(0, 0);
#endif

#if gPWR_FreqScalingWFI
    /* FRO 12MHz */
    SYSCON -> MAINCLKSEL = SYSCON_MAINCLKSEL_SEL(BOARD_MAINCLK_FRO12M);
#endif

#if gPWR_FlashControllerPowerDownInWFI
    if ( mLPMFlag==0 )
    {
#if ROM_API_CALL
        memset (&p_lowpower_cfg, 0, sizeof(LPC_LOWPOWER_T));

        p_lowpower_cfg.CFG   |= LOWPOWER_CFG_WFI_NOT_WFE_MASK;
#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
        BOARD_DbgIoSet(3, 1);
#endif
        Chip_LOWPOWER_PowerCycleCpuAndFlash(&p_lowpower_cfg);
#else
#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
        BOARD_DbgIoSet(3, 1);
#endif

        PWRLib_EnterSleep_WithFlashControllerPowerDown();
#endif
#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
        BOARD_DbgIoSet(3, 0);
#endif
    }
    else
    {
        POWER_EnterSleep();
    }

#else
    POWER_EnterSleep();
#endif


#if gPWR_FreqScalingWFI
#if gPWR_LdoVoltDynamic
    /* Need to increase LDO voltage before increasing CPU clock */
    POWER_ApplyLdoActiveVoltage(PM_LDO_VOLT_1_1V_DEFAULT);
    /* wait for LDO voltage to reach the voltage setting */
    CLOCK_uDelay(10U);
#endif

    /* FRO 48MHz or 32MHz - the flash wait states were not changed during WFI */
    SYSCON->MAINCLKSEL       = SYSCON_MAINCLKSEL_SEL(BOARD_TARGET_CPU_FREQ);

#if gPWR_LdoVoltDynamic
    POWER_ApplyLdoActiveVoltage(PM_LDO_VOLT_1_0V);
#endif
#endif

#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
    BOARD_DbgIoSet(0, 1);
#endif

    OSA_EnableIRQGlobal();
}


/*---------------------------------------------------------------------------
 * Name: PWRLib_UpdateWakeupReason
 * Description: Updates wakeup reason when exiting deep sleep
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PWR_UpdateWakeupReason(void)
{
    /* GPIO */
    uint32_t wakeup_io_source = POWER_GetIoWakeStatus();
    if (wakeup_io_source)
    {
        PWRLib_MCU_WakeupReason.Bits.FromKeyBoard = 1U;
        /* notify the GPIO adaptor so the user callback could be called */
        GpioNotifyIoWakeupSource(wakeup_io_source);
        PWR_DBG_LOG("WakeIOSource=%x", wakeup_io_source);
    }

    /* BLE */
    if (NVIC_GetPendingIRQ( BLE_WAKE_UP_TIMER_IRQn))
    {
        PWR_DBG_LOG("BLE_WAKE_UP_TIMER_IRQn");
        PWRLib_MCU_WakeupReason.Bits.FromBLE_LLTimer = 1U;
    }

    /* Timer - RTC */
    if (NVIC_GetPendingIRQ(WAKE_UP_TIMER0_IRQn) ||
        NVIC_GetPendingIRQ(WAKE_UP_TIMER1_IRQn))
    {
        PWR_DBG_LOG("WAKE_UP_TIMERx_IRQn");
        PWRLib_MCU_WakeupReason.Bits.FromTMR = 1U;
    }

    /* RTC Second Interrupt */
    if (NVIC_GetPendingIRQ(RTC_IRQn))
    {
        PWR_DBG_LOG("RTC_IRQn");
        PWRLib_MCU_WakeupReason.Bits.FromRTC_Sec = 1U;
    }

    /* Analog comparator 0 */
    if (NVIC_GetPendingIRQ(ANA_COMP_IRQn))
    {
        PWR_DBG_LOG("ANA_COMP_IRQn");
        PWRLib_MCU_WakeupReason.Bits.FromACmp0 = 1U;
    }

    /* USART 0 */
    if (NVIC_GetPendingIRQ(FLEXCOMM0_IRQn))
    {
        PWR_DBG_LOG("FLEXCOMM0_IRQn");
        PWRLib_MCU_WakeupReason.Bits.FromUSART0 = 1U;
    }

    /* USART 1 */
    if (NVIC_GetPendingIRQ(FLEXCOMM1_IRQn))
    {
        PWR_DBG_LOG("FLEXCOMM1_IRQn");
        PWRLib_MCU_WakeupReason.Bits.FromUSART1 = 1U;
    }

    /* BOD */
    if (NVIC_GetPendingIRQ(WDT_BOD_IRQn))
    {
        PWR_DBG_LOG("WDT_BOD_IRQn");
        PWRLib_MCU_WakeupReason.Bits.FromBOD = 1U;
    }
}

void PWR_ClearWakeupReason(void)
{
    PWRLib_MCU_WakeupReason.AllBits = 0UL;
}


PWR_WakeupReason_t PWR_GetWakeupReason(void)
{
    return PWRLib_MCU_WakeupReason;
}


void WarmMain(void)
{
    /* Interrupts should be disabled */
    __disable_irq();
#if defined(USE_RTOS) && (USE_RTOS)
    /* Switch to the psp stack */
    asm volatile("movs r0, #2");
    asm volatile("msr CONTROL, r0");
 #endif
    // Restore context
    PWR_longjmp(pwr_CPUContext, TRUE);
}


void PWRLib_EnterPowerDownMode(pwrlib_pd_cfg_t *pd_cfg)
{
    PWR_DBG_LOG("");
    register uint32_t stack_top;
    pm_power_config_t power_config;
    uint32_t pwrlib_pm_ram_config;
    int i;
    GET_MSP(stack_top);

    power_config.pm_wakeup_src = pd_cfg->wakeup_src;
#if gSupportBle
    power_config.pm_wakeup_src |= (POWER_WAKEUPSRC_BLE_OSC_EN
                                   | POWER_WAKEUPSRC_RTC ); /* POWER_WAKEUPSRC_BLE_WAKE_TIMER */
    /* BLE always assumes RFP retention registers retained */
    power_config.pm_config = pd_cfg->sleep_cfg | PM_CFG_XTAL32M_AUTOSTART  | PM_CFG_RADIO_RET;
#else
    power_config.pm_config = pd_cfg->sleep_cfg | PM_CFG_XTAL32M_AUTOSTART;
#endif

    /* Assess the RAM bank to be hold in sleep mode based on _end_noinit linker script symbol and SP register */
    i = 0;
    pwrlib_pm_ram_config = 0U;

    while ( pm_ram_bank_addr[i] < (uint32_t)&_end_fw_retention )
    {
        pwrlib_pm_ram_config |= ((1 << PM_CFG_SRAM_BANK_BIT_BASE) << i) ;
        i++;
    }

    for (i = sizeof(pm_ram_bank_addr) / sizeof(uint32_t) - 1; i >= 0; i--)
    {
        power_config.pm_config |= ((1 << PM_CFG_SRAM_BANK_BIT_BASE) << i);
        if ( pm_ram_bank_addr[i] <= stack_top )
            break;
    }
    power_config.pm_config |= pwrlib_pm_ram_config;
#if gLoggingActive_d
    power_config.pm_config |= PM_CFG_SRAM_ALL_RETENTION;
#endif

    power_config.pm_wakeup_io           = pd_cfg->wakeup_io & (BIT(22)-1) ;
    if (power_config.pm_wakeup_io)
    {
        power_config.pm_wakeup_src        |=  POWER_WAKEUPSRC_IO;
    }

    BOOT_SetResumeStackPointer(RESUME_STACK_POINTER);

    /* on application warm start, prevent the application from corrupting the current stack */
    SYSCON->CPSTACK = RESUME_STACK_POINTER;

    if ( 0 == PWR_setjmp(pwr_CPUContext))
    {
        POWER_EnterPowerMode( PM_POWER_DOWN, &power_config );
    }
}


void PWRLib_EnterPowerDownModeRamOff(pwrlib_pd_cfg_t *pd_cfg)
{
    PWR_DBG_LOG("");
    pm_power_config_t power_config;

    power_config.pm_wakeup_src = pd_cfg->wakeup_src;
    power_config.pm_config = pd_cfg->sleep_cfg | PM_CFG_XTAL32M_AUTOSTART;
    power_config.pm_wakeup_io           = pd_cfg->wakeup_io & (BIT(22)-1) ;
    if (power_config.pm_wakeup_io)
    {
        power_config.pm_wakeup_src         |=  POWER_WAKEUPSRC_IO;
    }

    POWER_EnterPowerMode( PM_POWER_DOWN, &power_config );
}

void PWRLib_EnterDeepDownMode(pwrlib_pd_cfg_t *pd_cfg)
{
    PWR_DBG_LOG("");
    pm_power_config_t power_config;

    power_config.pm_wakeup_src = pd_cfg->wakeup_src;
    power_config.pm_config = pd_cfg->sleep_cfg | PM_CFG_XTAL32M_AUTOSTART;
    power_config.pm_wakeup_io           = pd_cfg->wakeup_io & (BIT(22)-1) ;
    if (power_config.pm_wakeup_io)
    {
        power_config.pm_wakeup_src         |=  POWER_WAKEUPSRC_IO;
    }

    POWER_EnterPowerMode( PM_DEEP_DOWN, &power_config );
}

uint32_t PWRLib_u32RamBanksSpanned(uint32_t u32Start, uint32_t u32Length)
{
    uint32_t u32RamBanks = 0;
    uint32_t u32End = u32Start + u32Length - 1U;
    int iStartBank;
    int iEndBank;

    /* Bounds checking: returns 0 if any of these tests fail */
    if (   (u32Start  >= PWR_SRAM_BANK0_START_ADDR)
        && (u32Start  <  PWR_SRAM_BANK7_END_ADDR)
        && (u32End    >= PWR_SRAM_BANK0_START_ADDR)
        && (u32End    <  PWR_SRAM_BANK7_END_ADDR)
        && (u32Length >  0U)
       )
    {
        /* Assess the RAM bank(s) spanned by the specified RAM range.
           Loops result in iStartBank and iEndBank having values between
           1 and 8, for banks 0 to 7 */
        iStartBank = 1U;

        while ((iStartBank < 8U) && (pm_ram_bank_addr[iStartBank] <= u32Start))
        {
            iStartBank++;
        }

        iEndBank = iStartBank;
        while ((iEndBank < 8U) && (pm_ram_bank_addr[iEndBank] <= u32End))
        {
            iEndBank++;
        }

        /* Create bitmask with bits (iEndBank-1):(iStartBank-1) set */
        u32RamBanks = (1U << iEndBank) - 1U;
        u32RamBanks ^= (1U << (iStartBank - 1U)) - 1U;
    }

    return u32RamBanks;
}
