/****************************************************************************
 *
 * MODULE:             UART
 *
 * COMPONENT:          Uart interface for BDB Application Template AN1217 (Coorinator)
 *
 * VERSION:
 *
 * REVISION:
 *
 * DATED:
 *
 * STATUS:
 *
 * AUTHOR:
 *
 * DESCRIPTION:        Hardware abstarction layer for UART peripheral
 *
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
 ****************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h>
#include "board.h"
#include "app.h"
#include "fsl_usart.h"
#include "app_uart.h"
#include "ZQueue.h"
#include "portmacro.h"
#include "app_main.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifndef TRACE_UART
#define TRACE_UART  FALSE
#endif

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

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vUART_Init
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC void UART_vInit(void)
{
    usart_config_t config;
    DBG_vPrintf(TRACE_UART, "\r\nInitialising UART ...");

    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = UART_BAUD_RATE;
    config.enableTx = true;
    config.enableRx = true;

    USART_Init(UART, &config, CLOCK_GetFreq(kCLOCK_Fro32M));

    /* Enable RX interrupt. */
    USART_EnableInterrupts(UART, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    EnableIRQ(UART0_IRQ);
    DBG_vPrintf(TRACE_UART, "\r\nDone");
}

/****************************************************************************
 *
 * NAME: vUART_SetBuadRate
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 *
 ****************************************************************************/

PUBLIC void UART_vSetBaudRate(uint32 u32BaudRate)
{
    int result;

    /* setup baudrate */
    result = USART_SetBaudRate(UART, u32BaudRate, CLOCK_GetFreq(kCLOCK_Fro32M));
    if (kStatus_Success != result)
    {
        //DBG_vPrintf(TRACE_UART,"\r\nFailed to set UART speed ");
    }
}

/****************************************************************************
 *
 * NAME: UartIsr
 *
 * DESCRIPTION: Handle interrupts from uart
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
 void APP_isrUart ( void )
{
    uint8 u8Byte;
    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(UART))
    {
        uint8 u8Byte = USART_ReadByte(UART);
        ZQ_bQueueSend(&APP_msgSerialRx, &u8Byte);
    }
    else if ((kUSART_TxFifoNotFullFlag & USART_GetStatusFlags(UART)))
    {
        if (TRUE == ZQ_bQueueReceive(&APP_msgSerialTx, &u8Byte)) {
            USART_WriteByte(UART, u8Byte);
            DBG_vPrintf(TRACE_UART, "\r\ntx u8Byte %x", u8Byte);

        }
        else
        {
            /* disable tx interrupt as nothing to send */
            UART_vSetTxInterrupt(FALSE);
        }
    }
}

/****************************************************************************
 *
 * NAME: UART_vRtsStopFlow
 *
 * DESCRIPTION:
 * Set UART RS-232 RTS line high to stop any further data coming in
 *
 ****************************************************************************/

PUBLIC void UART_vRtsStopFlow(void)
{
    //vAHI_UartSetControl(UART, FALSE, FALSE, E_AHI_UART_WORD_LEN_8, TRUE, E_AHI_UART_RTS_HIGH);
}

/****************************************************************************
 *
 * NAME: UART_vRtsStartFlow
 *
 * DESCRIPTION:
 * Set UART RS-232 RTS line low to allow further data
 *
 ****************************************************************************/

PUBLIC void UART_vRtsStartFlow(void)
{
    //vAHI_UartSetControl(UART, FALSE, FALSE, E_AHI_UART_WORD_LEN_8, TRUE, E_AHI_UART_RTS_LOW);
}
/* [I SP001222_P1 283] end */

/****************************************************************************
 *
 * NAME: vUART_TxChar
 *
 * DESCRIPTION:
 * Set UART RS-232 RTS line low to allow further data
 *
 ****************************************************************************/
PUBLIC void UART_vTxChar(uint8 u8Char)
{
    USART_WriteByte(UART, u8Char);
    /* Wait to finish transfer */
    while (!(UART->FIFOSTAT & USART_FIFOSTAT_TXEMPTY_MASK))
    {
    }
}

/****************************************************************************
 *
 * NAME: vUART_TxReady
 *
 * DESCRIPTION:
 * Set UART RS-232 RTS line low to allow further data
 *
 ****************************************************************************/
PUBLIC bool_t UART_bTxReady()
{
    return ( (kUSART_TxFifoEmptyFlag|kUSART_TxFifoNotFullFlag) & USART_GetStatusFlags(UART) );
}

/****************************************************************************
 *
 * NAME: vUART_SetTxInterrupt
 *
 * DESCRIPTION:
 * Enable / disable the tx interrupt
 *
 ****************************************************************************/
PUBLIC void UART_vSetTxInterrupt(bool_t bState)
{
    USART_DisableInterrupts(UART, (kUSART_TxErrorInterruptEnable | kUSART_RxErrorInterruptEnable | kUSART_TxLevelInterruptEnable| kUSART_RxLevelInterruptEnable) );
    if (bState)
    {
        USART_EnableInterrupts(UART, (kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable) );
    }
    else
    {
        USART_EnableInterrupts(UART, (kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable | kUSART_TxLevelInterruptEnable|kUSART_TxErrorInterruptEnable ));
    }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
