/*
* Copyright 2019 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


#include "app.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_power.h"
#include "DebugExceptionHandlers_jn518x.h"
#include "fsl_iocon.h"
#include "fsl_gpio.h"
#include <stdbool.h>


gpio_pin_config_t led_config = {
    kGPIO_DigitalOutput, 0,
};

extern void OSA_TimeInit(void);

void BOARD_InitClocks(void)
{
    /* what is it for exactly */
    CLOCK_EnableAPBBridge();

    /* 32MHz clock */
    CLOCK_EnableClock(kCLOCK_Fro32M);       // derived from 198MHz FRO
    CLOCK_EnableClock(kCLOCK_Xtal32M);

    /* 32KHz clock */
    CLOCK_EnableClock(kCLOCK_Fro32k);      // isnt' strictly required but put for clarity
    //CLOCK_EnableClock(kCLOCK_Xtal32k);

    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_InputMux);

    /* Enable GPIO for LED controls */
    CLOCK_EnableClock(kCLOCK_Gpio0);

    /* Enable ADC clock */
    CLOCK_EnableClock(kCLOCK_Adc0);
    /* Power on the ADC converter. */
    POWER_EnablePD(kPDRUNCFG_PD_LDO_ADC_EN);

    /* INMUX and IOCON are used by many apps, enable both INMUX and IOCON clock bits here. */
    CLOCK_AttachClk(kOSC32M_to_USART_CLK);

    CLOCK_EnableClock(kCLOCK_Aes);
    CLOCK_EnableClock(kCLOCK_I2c0) ;
    CLOCK_AttachClk(kOSC32M_to_I2C_CLK);

    /* WWDT clock config (32k oscillator, no division) */
    CLOCK_AttachClk(kOSC32K_to_WDT_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivWdtClk, 1, true);

    /* Attach the ADC clock. */
    CLOCK_AttachClk(kXTAL32M_to_ADC_CLK);
    /* Enable LDO ADC 1v1 */
    PMC -> PDRUNCFG |= PMC_PDRUNCFG_ENA_LDO_ADC_MASK;
}

void BOARD_SetClockForPowerMode(void)
{
    CLOCK_AttachClk(kNONE_to_USART_CLK);
    CLOCK_DisableClock(kCLOCK_Iocon);
    CLOCK_DisableClock(kCLOCK_InputMux);
    CLOCK_DisableClock(kCLOCK_Gpio0);
    CLOCK_DisableClock(kCLOCK_Aes);
    CLOCK_DisableClock(kCLOCK_Xtal32M);
    CLOCK_DisableClock(kCLOCK_Fro48M);
    CLOCK_DisableClock(kCLOCK_I2c0);
    /* Disable LDO ADC 1v1 */
    PMC -> PDRUNCFG &= ~PMC_PDRUNCFG_ENA_LDO_ADC_MASK;
    POWER_DisablePD(kPDRUNCFG_PD_LDO_ADC_EN);
    CLOCK_DisableClock(kCLOCK_Adc0);
}


void BOARD_InitHardware(void)
{
    /* Security code to allow debug access */
    SYSCON->CODESECURITYPROT = 0x87654320;

    /* Configure the optimized active voltages, set up the BODs */
    POWER_Init();

    /* reset FLEXCOMM for USART - mandatory from power down 4 at least*/
    RESET_PeripheralReset(kUSART0_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kGPIO0_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kI2C0_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kADC0_RST_SHIFT_RSTn);
    /* For power optimization, we need a limited clock enabling specific to this application
     * This replaces BOARD_BootClockRUN() usually called in other application    */
    BOARD_InitClocks();
    BOARD_InitDebugConsole();
    BOARD_InitPins();

    /* Enable DMA access to RAM (assuming default configuration and MAC
     * buffers not in first block of RAM) */
    *(volatile uint32_t *)0x40001000 = 0xE000F733;
    *(volatile uint32_t *)0x40001004 = 0x109;

    /* Initialise output LED GPIOs */
    GPIO_PinInit(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED1_PIN, &led_config);
    GPIO_PinInit(GPIO, APP_BOARD_GPIO_PORT, APP_BASE_BOARD_LED2_PIN, &led_config);

    SYSCON -> MAINCLKSEL       = 3;  /* 32 M FRO */
    SystemCoreClockUpdate();
    OSA_TimeInit();

    /* Initialise exception handlers for debugging */
    vDebugExceptionHandlersInitialise();
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
	BOARD_InitHardware();
}
