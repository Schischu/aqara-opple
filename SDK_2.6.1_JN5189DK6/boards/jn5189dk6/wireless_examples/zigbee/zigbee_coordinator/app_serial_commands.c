/*
* Copyright 2019 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "EmbeddedTypes.h"
#include "app.h"
#include "dbg.h"
#include "zps_gen.h"
#include "app_coordinator.h"
#include "app_zcl_task.h"
#include "app_buttons.h"
#include "app_common.h"
#include "app_serial_commands.h"
#include "app_main.h"
#include "ZQueue.h"
#include "ZTimer.h"
#include "fsl_reset.h"
#include "MicroSpecific.h"
#include "app_uart.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef TRACE_SERIAL
    #define TRACE_SERIAL      FALSE
#endif


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
#define COMMAND_BUF_SIZE   80

typedef struct
{
    uint8_t  au8Buffer[COMMAND_BUF_SIZE];
    uint8_t  u8Pos;
}tsCommand;

#define stricmp strcasecmp

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void vProcessRxChar(uint8_t u8Char);
static void vProcessCommand(void);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static tsCommand sCommand;
char * strings[] = {
        "START UP",
        "NFN START",
        "RUNNING"
        };
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/******************************************************************************
 * NAME: APP_taskAtSerial
 *
 * DESCRIPTION:
 *
 *
 * PARAMETERS:      Name            Usage
 *
 * RETURNS:
 * None
 ****************************************************************************/
void APP_taskAtSerial( void)
{
    uint8_t u8RxByte;
    if ( UART_bBufferReceive ( &u8RxByte ) ) {
    	vProcessRxChar ( u8RxByte);
    }
}

/******************************************************************************
 * NAME: vProcessRxChar
 *
 * DESCRIPTION:
 * Processes the received character
 *
 * PARAMETERS:      Name            Usage
 * uint8_t            u8Char          Character
 *
 * RETURNS:
 * None
 ****************************************************************************/
static void vProcessRxChar(uint8_t u8Char)
{

    if ((u8Char >= 'a' && u8Char <= 'z'))
    {
        u8Char -= ('a' - 'A');
    }
    if ((sCommand.u8Pos < COMMAND_BUF_SIZE)  && (u8Char != 0x0d))
    {
        sCommand.au8Buffer[sCommand.u8Pos++] = u8Char;
    }
    else if (sCommand.u8Pos >= COMMAND_BUF_SIZE)
    {
        DBG_vPrintf(TRACE_SERIAL, "OverFlow\r\n");
        memset(sCommand.au8Buffer, 0, COMMAND_BUF_SIZE);
        sCommand.u8Pos = 0;
    }

    if (u8Char == 0x0d)
    {
        vProcessCommand();
    }
}

/******************************************************************************
 * NAME: vProcessCommand
 *
 * DESCRIPTION:
 * Processed the received command
 *
 * PARAMETERS:      Name            Usage
 *
 * RETURNS:
 * None
 ****************************************************************************/
static void vProcessCommand(void)
{
    uint8_t Delimiters[] = " ";
    uint8_t *token = NULL;

    token = (uint8_t *)strtok((char *)sCommand.au8Buffer, (char *)Delimiters);
    APP_tsEvent sButtonEvent;
    sButtonEvent.eType = APP_E_EVENT_NONE;

    if (0 == stricmp((char*)token, "toggle"))
    {
        DBG_vPrintf(TRACE_SERIAL, "Toggle\r\n");
        sButtonEvent.eType = APP_E_EVENT_SERIAL_TOGGLE;
    }
    else if (0 == stricmp((char*)token, "steer"))
    {
        DBG_vPrintf(TRACE_SERIAL, "Steer\r\n");
        sButtonEvent.eType = APP_E_EVENT_SERIAL_NWK_STEER;
    }
    else if (0 == stricmp((char*)token, "form"))
    {
        DBG_vPrintf(TRACE_SERIAL, "Form\r\n");
        sButtonEvent.eType = APP_E_EVENT_SERIAL_FORM_NETWORK;
    }
    else if (0 == stricmp((char*)token, "find"))
    {
        DBG_vPrintf(TRACE_SERIAL, "Find\r\n");
        sButtonEvent.eType = APP_E_EVENT_SERIAL_FIND_BIND_START;
    }
    else if (0 == stricmp((char*)token, "factory"))
    {
        token = (uint8_t *)strtok( NULL, (char *)Delimiters);
        if (0 == stricmp((char*)token, "reset"))
        {
            DBG_vPrintf(TRACE_SERIAL, "Factory reset\r\n");
            APP_vFactoryResetRecords();
            MICRO_DISABLE_INTERRUPTS();
            RESET_SystemReset();
        }
    }
    else if (0 == stricmp((char*)token, "soft"))
    {
        token = (uint8_t *)strtok( NULL, (char *)Delimiters);
        if (0 == stricmp((char*)token, "reset"))
        {
            MICRO_DISABLE_INTERRUPTS();
            RESET_SystemReset();
        }
    }
    else
    {
        DBG_vPrintf(1, "Unkown serial command %s\r\n", token);
        DBG_vPrintf(1,"Commands Supported \r\n");
        DBG_vPrintf(1,"****************** \r\n");
        DBG_vPrintf(1,"toggle - sends on\\off toggle command\r\n");
        DBG_vPrintf(1,"steer  - opens permit join window\r\n");
        DBG_vPrintf(1,"form   - forms the network\r\n");
        DBG_vPrintf(1,"find   - makes a binding on a target which is identifying\r\n");
        DBG_vPrintf(1,"factory reset  - clears all data and starts afresh \r\n");
        DBG_vPrintf(1,"soft reset  - retains network configuration but resets the node \r\n");
    }

    if (sButtonEvent.eType != APP_E_EVENT_NONE)
    {
        if( ZQ_bQueueSend(&APP_msgAppEvents, &sButtonEvent)  == FALSE)
        {
            DBG_vPrintf(1, "Queue Overflow has happened \n");
        }
    }


    memset(sCommand.au8Buffer, 0, COMMAND_BUF_SIZE);
    sCommand.u8Pos = 0;

}



/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
