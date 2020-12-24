/*****************************************************************************
 *
 * MODULE:             JN-AN-1245
 *
 * COMPONENT:          pin_mux.c
 *
 * DESCRIPTION:        Pin multiplexing
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
 * Copyright NXP B.V. 2017-2018. All rights reserved
 *
 ***************************************************************************/
#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_gpio.h"
#include "pin_mux.h"
#include "app.h"
#if ((defined CoinCellSwitch) && (defined USE_DIO_FOR_MEASUREMENTS))
#include "app_coincell_dio.h"
#endif
/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/



#ifndef BOARD_USECLKINSRC
#define BOARD_USECLKINSRC   (0)
#endif

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Local Prototypes
 ****************************************************************************/


 /*****************************************************************************
 * Private functions
 ****************************************************************************/



/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_InitPins(void)
{
    /* USART0 RX/TX pin */
    IOCON_PinMuxSet(IOCON, 0, 8, IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);
    IOCON_PinMuxSet(IOCON, 0, 9, IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);

#ifdef ColorSceneController
    /* USART1 RX/TX pin */
    IOCON_PinMuxSet(IOCON, 0, 10, IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);
    IOCON_PinMuxSet(IOCON, 0, 11, IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);
#endif

    /* Debugger signals (SWCLK, SWDIO) - need to turn it OFF to reduce power consumption in power modes*/
    IOCON_PinMuxSet(IOCON, 0, 12, IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);
    IOCON_PinMuxSet(IOCON, 0, 13, IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);

#if ((defined DimmerSwitch) || (defined GpEhSwitch))
    /* I2C0  */
    IOCON_PinMuxSet(IOCON, 0, 10, IOCON_FUNC5 | IOCON_DIGITAL_EN | IOCON_STDI2C_EN);  /* I2C0_SCL */
    IOCON_PinMuxSet(IOCON, 0, 11, IOCON_FUNC5 | IOCON_DIGITAL_EN | IOCON_STDI2C_EN);  /* I2C0_SDA */
#endif

#if (defined CoinCellSwitch)
    IOCON_PinMuxSet(IOCON, 0, 20, IOCON_MODE_INACT | IOCON_FUNC5 | IOCON_DIGITAL_EN); /* RFTX */
    IOCON_PinMuxSet(IOCON, 0, 21, IOCON_MODE_INACT | IOCON_FUNC5 | IOCON_DIGITAL_EN); /* RFRX */
#endif
}

void BOARD_SetPinsForPowerMode(void)
{

#ifdef CoinCellSwitch
    for ( int i=0; i<22; i++)
    {
        /* configure GPIOs to Input mode */
    	if ( ((1 << i) & ((1 << APP_BOARD_SW1_PIN)|(1 << APP_BOARD_SW2_PIN) | (1 << APP_BOARD_SW3_PIN) | (1 << APP_BOARD_SW4_PIN))) != (1<<i))
    	{
            GPIO_PinInit(GPIO, 0, i, &((const gpio_pin_config_t){kGPIO_DigitalInput, 1}));
            IOCON_PinMuxSet(IOCON, 0, i, IOCON_FUNC0 |  IOCON_MODE_INACT | IOCON_ANALOG_EN);
    	}
    }
#else
    for ( int i=0; i<22; i++)
    {
        GPIO_PinInit(GPIO, 0, i, &((const gpio_pin_config_t){kGPIO_DigitalInput, 1}));
        IOCON_PinMuxSet(IOCON, 0, i, IOCON_FUNC0 |  IOCON_MODE_INACT | IOCON_ANALOG_EN);
    }
#endif
}

#if ((defined CoinCellSwitch) && (defined USE_DIO_FOR_MEASUREMENTS))

void BOARD_InitPinsForMeasurements(void)
{
	gpio_pin_config_t dio_config_down = {
			kGPIO_DigitalOutput, 0
	};

	gpio_pin_config_t dio_config_up = {
			kGPIO_DigitalOutput, 1
	};

#if ((defined CoinCellSwitch) && ((defined USE_DIO_FOR_MEASUREMENTS) &&  (!bGPIOClockAlreadyEnabled)))
    	IOCON_PinMuxSet(IOCON, 0, POWER_UP_DIO, IOCON_FUNC0 |  IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN);
    	GPIO_PinInit(GPIO, 0, POWER_UP_DIO, &dio_config_down);
#endif

	IOCON_PinMuxSet(IOCON, 0, SLEEP_DIO, IOCON_FUNC0 |  IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN);
	GPIO_PinInit(GPIO, 0, SLEEP_DIO, &dio_config_down);

	IOCON_PinMuxSet(IOCON, 0, RADIO_COLD_INIT_DIO, IOCON_FUNC0 |  IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN);
	GPIO_PinInit(GPIO, 0, RADIO_COLD_INIT_DIO, &dio_config_down);

	IOCON_PinMuxSet(IOCON, 0, RADIO_INIT_DIO, IOCON_FUNC0 |  IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN);
	GPIO_PinInit(GPIO, 0, RADIO_INIT_DIO, &dio_config_down);

	IOCON_PinMuxSet(IOCON, 0, STACK_INIT_DIO, IOCON_FUNC0 |  IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN);
	GPIO_PinInit(GPIO, 0, STACK_INIT_DIO, &dio_config_up);

	IOCON_PinMuxSet(IOCON, 0, INIT_PACKET_TRANSMISSION_DIO, IOCON_FUNC0 |  IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN);
	GPIO_PinInit(GPIO, 0, INIT_PACKET_TRANSMISSION_DIO, &dio_config_up);

#if 0
	IOCON_PinMuxSet(IOCON, 0, POWER_MANAGER_DIO, IOCON_FUNC0 |  IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN);
	GPIO_PinInit(GPIO, 0, POWER_MANAGER_DIO, &dio_config_down);
#endif

	IOCON_PinMuxSet(IOCON, 0, SAVE_ZPS_RECORDS_DIO, IOCON_FUNC0 |  IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN);
	GPIO_PinInit(GPIO, 0, SAVE_ZPS_RECORDS_DIO, &dio_config_down);

	IOCON_PinMuxSet(IOCON, 0, ZCL_INIT_DIO, IOCON_FUNC0 |  IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN);
	GPIO_PinInit(GPIO, 0, ZCL_INIT_DIO, &dio_config_down);

}
#endif
