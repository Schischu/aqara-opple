/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/
/*
 * How to set up clock using clock driver functions:
 *
 * 1. Setup clock sources.
 *
 * 2. Set up all dividers.
 *
 * 3. Set up all selectors to provide selected clocks.
 */

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Clocks v6.0
processor: JN5189
package_id: JN5189HN
mcu_data: ksdk2_0
processor_version: 0.0.0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

#include "clock_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* System clock frequency. */
extern uint32_t SystemCoreClock;

/*******************************************************************************
 ************************ BOARD_InitBootClocks function ************************
 ******************************************************************************/
void BOARD_InitBootClocks(void)
{
    BOARD_BootClockRUN();
}

/*******************************************************************************
 ********************** Configuration BOARD_BootClockRUN ***********************
 ******************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!Configuration
name: BOARD_BootClockRUN
called_from_default_init: true
outputs:
- {id: ASYNCAPB_clock.outFreq, value: 48 MHz}
- {id: DMIC_clock.outFreq, value: 48 MHz}
- {id: I2C_clock.outFreq, value: 32 MHz}
- {id: SPIFI_clock.outFreq, value: 24 MHz}
- {id: SPI_clock.outFreq, value: 32 MHz}
- {id: SYSTICK_CLK.outFreq, value: 48 MHz}
- {id: System_clock.outFreq, value: 48 MHz}
- {id: TRACE_CLK.outFreq, value: 48 MHz}
- {id: USART_clock.outFreq, value: 32 MHz}
- {id: WDT_clock.outFreq, value: 32.768 kHz}
settings:
- {id: ASYNCAPBCTRL_EN_CFG, value: Enable}
- {id: ENABLE_XTAL32MHZ, value: Enabled}
- {id: PMC_PDRUNCFG_ENA_FRO32K_CFG, value: Enable}
- {id: SYSCON.DMICCLKSEL.sel, value: SYSCON.MAINCLKSEL}
- {id: SYSCON.I2CCLKSEL.sel, value: SYSCON.OSC32MCLKSEL}
- {id: SYSCON.MAINCLKSEL.sel, value: PMC.fro_48m_clk}
- {id: SYSCON.SPICLKSEL.sel, value: SYSCON.OSC32MCLKSEL}
- {id: SYSCON.SPIFICLKDIV.scale, value: '2', locked: true}
- {id: SYSCON.SPIFICLKSEL.sel, value: SYSCON.MAINCLKSEL}
- {id: SYSCON.USARTCLKSEL.sel, value: SYSCON.OSC32MCLKSEL}
- {id: SYSCON.WDTCLKSEL.sel, value: SYSCON.OSC32KCLKSEL}
sources:
- {id: PMC.fro_32k.outFreq, value: 32.768 kHz}
- {id: SYSCON.XTAL_32M.outFreq, value: 32 MHz, enabled: true}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/*******************************************************************************
 * Variables for BOARD_BootClockRUN configuration
 ******************************************************************************/
/*******************************************************************************
 * Code for BOARD_BootClockRUN configuration
 ******************************************************************************/
void BOARD_BootClockRUN(void)
{
    /*!< Set up the clock sources */
    CLOCK_EnableClock(kCLOCK_Fro12M);  /*!< Ensure FRO 12MHz is on */
    CLOCK_EnableClock(kCLOCK_Fro32M);  /*!< Ensure FRO 32MHz is on */
    CLOCK_EnableClock(kCLOCK_Fro48M);  /*!< Ensure FRO 48MHz is on */
    CLOCK_EnableAPBBridge();           /*!< The Async_APB clock is enabled. */
    CLOCK_EnableClock(kCLOCK_Xtal32M); /*!< Enable XTAL 32 MHz output */
    /*!< Configure RTC OSC */
    CLOCK_EnableClock(kCLOCK_Fro32k);       /*!< Enable RTC FRO 32 KHz output */
    CLOCK_AttachClk(kFRO32K_to_OSC32K_CLK); /*!< Switch OSC32K to FRO32K */
    /*!< Set up dividers */
    CLOCK_SetClkDiv(kCLOCK_DivRtcClk, 1U, false);     /*!< Set RTCCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 1U, false);     /*!< Set AHBCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivSystickClk, 1U, false); /*!< Set SYSTICKCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivTraceClk, 1U, false);   /*!< Set TRACECLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivSpifiClk, 2U, false);   /*!< Set SPIFICLKDIV divider to value 2 */
    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 1U, false);    /*!< Set DMICCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivWdtClk, 1U, true);      /*!< Set WDTCLKDIV divider to value 1 */

    /*!< Set up clock selectors - Attach clocks to the peripheries */
    CLOCK_AttachClk(kFRO48M_to_MAIN_CLK);    /*!< Switch MAIN_CLK to FRO48M */
    CLOCK_AttachClk(kMAIN_CLK_to_ASYNC_APB); /*!< Switch ASYNC_APB to MAIN_CLK */
    CLOCK_AttachClk(kFRO32M_to_OSC32M_CLK);  /*!< Switch OSC32M_CLK to FRO32M */
    CLOCK_AttachClk(kFRO32K_to_OSC32K_CLK);  /*!< Switch OSC32K_CLK to FRO32K */
    CLOCK_AttachClk(kOSC32M_to_USART_CLK);   /*!< Switch USART_CLK to OSC32M */
    CLOCK_AttachClk(kMAIN_CLK_to_SPIFI);     /*!< Switch SPIFI to MAIN_CLK */
    CLOCK_AttachClk(kMAIN_CLK_to_DMI_CLK);   /*!< Switch DMI_CLK to MAIN_CLK */
    CLOCK_AttachClk(kOSC32K_to_WDT_CLK);     /*!< Switch WDT_CLK to OSC32K */
    CLOCK_AttachClk(kOSC32M_to_SPI_CLK);     /*!< Switch SPI_CLK to OSC32M */
    CLOCK_AttachClk(kOSC32M_to_I2C_CLK);     /*!< Switch I2C_CLK to OSC32M */

    /*!< Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;
}
