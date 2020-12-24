/*****************************************************************************
 *
 * MODULE:             JN-AN-1245
 *
 * COMPONENT:          app_zlo_switch_node.c
 *
 * DESCRIPTION:        ZLO Controller & Switch Demo : Stack <-> Remote Control App Interaction
 *                     (Implementation)
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
#include <jendefs.h>
#include <AppApi.h>
#include "pdum_apl.h"
#include "pdum_gen.h"
#include "PDM.h"
#include "dbg.h"

#ifdef APP_LOW_POWER_API
#include "PWR_interface.h"
#else
#include "pwrm.h"
#endif

#include "zps_gen.h"
#include "zps_apl_af.h"
#include "zps_apl_zdo.h"
#include "zps_apl_aib.h"
#include "zps_apl_zdp.h"
#include "rnd_pub.h"
#include "Identify.h"
#include "app_common.h"
#include "app_main.h"
#include "Groups.h"
#include "PDM_IDs.h"
#include "app_events.h"
#include "ZTimer.h"
#include "app_zlo_switch_node.h"
#include "app_zcl_switch_task.h"
#include "app_events.h"
#include "zcl_customcommand.h"
#include "app_buttons.h"
#include "app_switch_state_machine.h"
#include "fsl_reset.h"
#ifdef CLD_OTA
    #include "OTA.h"
    #include "app_ota_client.h"
#endif
#ifdef APP_NTAG_ICODE
#include "app_ntag_icode.h"
#include "nfc_nwk.h"
#endif
#include "appZdpExtraction.h"
#include "app_main.h"
#include "MicroSpecific.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifdef DEBUG_SWITCH_NODE
    #define TRACE_SWITCH_NODE   TRUE
#else
    #define TRACE_SWITCH_NODE   FALSE
#endif

#define bWakeUpFromSleep() bWaitingToSleep()  /* For readability purpose */

#define APP_LONG_SLEEP_DURATION_IN_SEC 1
#define WAKE_FROM_DEEP_SLEEP    (1<<11)

/* ZDO endpoint for all devices is always 0 */
#define SWITCH_ZDO_ENDPOINT    (0)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PUBLIC void vStopTimer(uint8 u8Timer);
#ifdef SLEEP_ENABLE
    PRIVATE void vStopAllTimers(void);
    PRIVATE void vLoadKeepAliveTime(uint8 u8TimeInSec);
#endif

PRIVATE void vDeletePDMOnButtonPress(uint8 u8ButtonDIO);
PRIVATE void vHandleJoinAndRejoin(void);
PRIVATE void app_vStartNodeFactoryNew(void);
PRIVATE void vAppHandleAfEvent( BDB_tsZpsAfEvent *psZpsAfEvent);
PRIVATE void vHandleRunningStackEvent(ZPS_tsAfEvent* psStackEvent);
PRIVATE void vAppHandleZdoEvents( BDB_tsZpsAfEvent *psZpsAfEvent);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

PUBLIC tsDeviceDesc             sDeviceDesc;
PUBLIC uint16                   u16GroupId;

extern const uint8 u8MyEndpoint;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

PRIVATE uint16 u16FastPoll;

#ifdef SLEEP_ENABLE
    PRIVATE bool bDataPending=FALSE;
    #ifdef DEEP_SLEEP_ENABLE
        PRIVATE uint8 u8DeepSleepTime= DEEP_SLEEP_TIME;
    #endif
    PRIVATE uint8 u8KeepAliveTime = KEEP_ALIVETIME;
#ifdef APP_LOW_POWER_API
    PRIVATE PWR_tsWakeTimerEvent sWake;
    extern uint8_t mLPMFlag;
#else
    PRIVATE pwrm_tsWakeTimerEvent sWake;
#endif
#endif
PRIVATE bool bWaitingForLeave = FALSE;

#ifdef SLEEP_MIN_RETENTION
PRIVATE uint8 u8NumOfPollFailure = 0;
/*this better be aligned with the NumPollFailuresBeforeRejoin zps config file*/
#define MAX_POLL_FAILURE  3
extern void BDB_vRejoinCycle(bool_t bSkipDirectJoin);
#endif

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: APP_vInitialiseNode
 *
 * DESCRIPTION:
 * Initialises the application related functions
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vInitialiseNode(bool_t bColdStart)
{
    if(bColdStart)
    {
        #ifdef APP_ANALYZE
            GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_BLUE, 0);
        #endif
        DBG_vPrintf(TRACE_SWITCH_NODE, "\r\nAPP_vInitialiseNode");
        #ifdef DEEP_SLEEP_ENABLE
            vReloadSleepTimers();
        #endif
        #ifdef CLD_OTA
            vLoadOTAPersistedData();
        #endif

        /* Restore any application data previously saved to flash */
        uint16 u16ByteRead;
        PDM_eReadDataFromRecord(PDM_ID_APP_ZLO_SWITCH,
                                &sDeviceDesc,
                                sizeof(tsDeviceDesc),
                                &u16ByteRead);
    }

#ifdef JN517x  // TODO
    /* Default module configuration: change E_MODULE_DEFAULT as appropriate */
      vAHI_ModuleConfigure(E_MODULE_DEFAULT);
#endif

    ZPS_psAplAibGetAib()->bUseInstallCode = BDB_JOIN_USES_INSTALL_CODE_KEY;

#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_BLUE, 1);
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_PURPLE, 0);
#endif

    if(bColdStart)
    {
        /* Update radio temperature (loading calibration) */
        APP_vRadioTempUpdate(TRUE);
        /* Initialise ZBPro stack */
        ZPS_eAplAfInit();
    }
    else
    {
        /* Update timer with sleep period */
        APP_vRadioTempUpdateMs(SLEEP_DURATION_MS);
        /* ReInitialise ZBPro stack */
        ZPS_eAplAfReInit();
    }

#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_PURPLE, 1);
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_BLUE, 0);
#endif

    APP_ZCL_vInitialise(bColdStart);

#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_BLUE, 1);
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_PURPLE, 0);
#endif
    ZPS_bAplAfSetEndDeviceTimeout(ZED_TIMEOUT_16384_MIN);
    /* If the device state has been restored from flash, re-start the stack
     * and set the application running again.
     */
    if (sDeviceDesc.eNodeState == E_RUNNING)
    {
        sBDB.sAttrib.bbdbNodeIsOnANetwork = TRUE;
    }
    else
    {
        app_vStartNodeFactoryNew();
        DBG_vPrintf(TRACE_SWITCH_NODE, "\r\nFactory New Start");
        sBDB.sAttrib.bbdbNodeIsOnANetwork = FALSE;
    }

    sBDB.sAttrib.u32bdbPrimaryChannelSet = BDB_PRIMARY_CHANNEL_SET;
    sBDB.sAttrib.u32bdbSecondaryChannelSet = BDB_SECONDARY_CHANNEL_SET;
    BDB_tsInitArgs sInitArgs;
    sInitArgs.hBdbEventsMsgQ = &APP_msgBdbEvents;

#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_PURPLE, 1);
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_BLUE, 0);
#endif
    BDB_vInit(&sInitArgs);

#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_BLUE, 1);
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_PURPLE, 0);
#endif
    uint64 u64MacAddr = ZPS_u64AplZdoGetIeeeAddr();
    u16GroupId = ((uint16)u64MacAddr) ^ ((uint16)(u64MacAddr>>16));

    /*
     * Initialise buttons and LEDs
     */
    APP_vInitLeds();
    APP_bButtonInitialise();

    if(bColdStart)
    {
        /*In case of a deep sleep device any button wake up would cause a PDM delete , only check for User Interface
         * pressed for deleting the context */
        vDeletePDMOnButtonPress(APP_BOARD_SW0_PIN);

        #ifdef PDM_EEPROM
            vDisplayPDMUsage();
        #endif
    }

#ifdef APP_ANALYZE
    GPIO_PinWrite(GPIO, APP_BOARD_GPIO_PORT, APP_ANALYZE_PURPLE, 1);
#endif

}


/****************************************************************************
 *
 * NAME: vStartFastPolling
 *
 * DESCRIPTION:
 * Set fast poll time
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vStartFastPolling(uint8 u8Seconds)
{
    /* Fast poll is every 100ms, so times by 10 */
    u16FastPoll = 10*u8Seconds;
}

/****************************************************************************
 *
 * NAME: APP_vBdbCallback
 *
 * DESCRIPTION:
 * Callbak from the BDB
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vBdbCallback(BDB_tsBdbEvent *psBdbEvent)
{
    static uint8 u8NoQueryCount;

   // DBG_vPrintf(1,"BdbCb type %d\r\n", psBdbEvent->eEventType);

    switch(psBdbEvent->eEventType)
    {
        case BDB_EVENT_NONE:
            break;
        case BDB_EVENT_ZPSAF:                // Use with BDB_tsZpsAfEvent
            vAppHandleAfEvent(&psBdbEvent->uEventData.sZpsAfEvent);
            break;

        case BDB_EVENT_INIT_SUCCESS:
            DBG_vPrintf(TRACE_SWITCH_NODE,"APP: BdbInitSuccessful\r\n");
            break;

        case BDB_EVENT_REJOIN_FAILURE: // only for ZED
            DBG_vPrintf(TRACE_SWITCH_NODE, "BDB EVT INIT failed to join\r\n");
            /* If waiting for leave do not try to go to sleep*/
            if(bWaitingForLeave == FALSE)
            {
                DBG_vPrintf(TRACE_SWITCH_NODE,"Init Rejoin Failure!\r\n");
                // Go to deep sleep
                #ifdef SLEEP_ENABLE
                    vLoadKeepAliveTime(0);
                    #ifdef DEEP_SLEEP_ENABLE
                        vLoadDeepSleepTimer(0);
                    #endif
                    ZTIMER_eStart(u8TimerTick, ZTIMER_TIME_MSEC(10));
                #endif
            }
            break;

        case BDB_EVENT_REJOIN_SUCCESS: // only for ZED
            DBG_vPrintf(TRACE_SWITCH_NODE, "BDB EVT INIT Join success\r\n");
            vHandleJoinAndRejoin();
            break;


        case BDB_EVENT_NWK_STEERING_SUCCESS:
            // go to running state
            DBG_vPrintf(TRACE_SWITCH_NODE,"GoRunningState!\r\n");
            vHandleJoinAndRejoin();
            break;

        case BDB_EVENT_NO_NETWORK:
            DBG_vPrintf(TRACE_SWITCH_NODE, "No Network\r\n");
            // Go to deep sleep
            #ifdef SLEEP_ENABLE
                vLoadKeepAliveTime(0);
                #ifdef DEEP_SLEEP_ENABLE
                    vLoadDeepSleepTimer(0);
                    vUpdateKeepAliveTimer();
			sDeviceDesc.eNodeState = E_GOING_DEEP;
                #endif
            #endif
            break;

        case BDB_EVENT_APP_START_POLLING:
            DBG_vPrintf(TRACE_SWITCH_NODE,"Start Polling!\r\n");
            /* Start 1 seconds polling */
            DBG_vPrintf(TRACE_SWITCH_NODE, "APP_vBdbCallback: eNodeState = E_WAIT_STARTUP\r\n");
            sDeviceDesc.eNodeState = E_WAIT_STARTUP;
            ZTIMER_eStart(u8TimerPoll, POLL_TIME_FAST);
            vStartFastPolling(2);
            break;

        case BDB_EVENT_FB_HANDLE_SIMPLE_DESC_RESP_OF_TARGET:
            DBG_vPrintf(TRACE_SWITCH_NODE,"Simple descriptor Ep %d on Target 0x%x Profile %04x Device %04x Version %d \r\n",
                    psBdbEvent->uEventData.psFindAndBindEvent->u8TargetEp,
                    psBdbEvent->uEventData.psFindAndBindEvent->u16TargetAddress,
                    psBdbEvent->uEventData.psFindAndBindEvent->u16ProfileId,
                    psBdbEvent->uEventData.psFindAndBindEvent->u16DeviceId,
                    psBdbEvent->uEventData.psFindAndBindEvent->u8DeviceVersion);
            break;

        case BDB_EVENT_FB_CHECK_BEFORE_BINDING_CLUSTER_FOR_TARGET:
            DBG_vPrintf(TRACE_SWITCH_NODE,"Check For Binding Cluster 0x%x \r\n",psBdbEvent->uEventData.psFindAndBindEvent->uEvent.u16ClusterId);
            break;

        case BDB_EVENT_FB_CLUSTER_BIND_CREATED_FOR_TARGET:
            DBG_vPrintf(TRACE_SWITCH_NODE,"Bind Created for cluster 0x%x \r\n",psBdbEvent->uEventData.psFindAndBindEvent->uEvent.u16ClusterId);
            break;

        case BDB_EVENT_FB_BIND_CREATED_FOR_TARGET:
        {
            DBG_vPrintf(TRACE_SWITCH_NODE,"Bind Created for target EndPt %d \r\n",psBdbEvent->uEventData.psFindAndBindEvent->u8TargetEp);
            u8NoQueryCount = 0;
            // Example to ask to stop identification to an end point on completing discovery

                uint8 u8Seq;
                tsZCL_Address sAddress;
                tsCLD_Identify_IdentifyRequestPayload sPayload;

                sPayload.u16IdentifyTime = 0;
                sAddress.eAddressMode = E_ZCL_AM_SHORT_NO_ACK;
                sAddress.uAddress.u16DestinationAddress = psBdbEvent->uEventData.psFindAndBindEvent->u16TargetAddress;

                eCLD_IdentifyCommandIdentifyRequestSend(
                                        psBdbEvent->uEventData.psFindAndBindEvent->u8InitiatorEp,
                                        psBdbEvent->uEventData.psFindAndBindEvent->u8TargetEp,
                                        &sAddress,
                                        &u8Seq,
                                        &sPayload);

                break;
        }

        case BDB_EVENT_FB_GROUP_ADDED_TO_TARGET:
        {
            DBG_vPrintf(TRACE_SWITCH_NODE,"Group Id 0x%x Added to Target 0x%x \r\n",
                    psBdbEvent->uEventData.psFindAndBindEvent->uEvent.u16GroupId,
                    psBdbEvent->uEventData.psFindAndBindEvent->u16TargetAddress);
            u8NoQueryCount = 0;
            //Example to ask to Stop identification to that group
            uint8 u8Seq;
            tsZCL_Address sAddress;
            tsCLD_Identify_IdentifyRequestPayload sPayload;

            sPayload.u16IdentifyTime = 0;
            sAddress.eAddressMode = E_ZCL_AM_SHORT_NO_ACK;
            sAddress.uAddress.u16DestinationAddress = psBdbEvent->uEventData.psFindAndBindEvent->u16TargetAddress;

            eCLD_IdentifyCommandIdentifyRequestSend(
                                    psBdbEvent->uEventData.psFindAndBindEvent->u8InitiatorEp,
                                    psBdbEvent->uEventData.psFindAndBindEvent->u8TargetEp,
                                    &sAddress,
                                    &u8Seq,
                                    &sPayload);
            break;
        }

        case BDB_EVENT_FB_ERR_BINDING_TABLE_FULL:
            DBG_vPrintf(TRACE_SWITCH_NODE,"ERR: Bind Table Full\r\n");
            break;

        case BDB_EVENT_FB_ERR_BINDING_FAILED:
            DBG_vPrintf(TRACE_SWITCH_NODE,"ERR: Bind\r\n");
            break;

        case BDB_EVENT_FB_ERR_GROUPING_FAILED:
            DBG_vPrintf(TRACE_SWITCH_NODE,"ERR: Group\r\n");
            break;

        case BDB_EVENT_FB_NO_QUERY_RESPONSE:
            DBG_vPrintf(TRACE_SWITCH_NODE,"Find & Bind: No Query response\r\n");
            //Example to stop further query repeating
            if(u8NoQueryCount >= 2)
            {
                u8NoQueryCount = 0;
                vAPP_ZCL_DeviceSpecific_SetIdentifyTime(0);
                BDB_vFbExitAsInitiator();
                DBG_vPrintf(TRACE_SWITCH_NODE,"Find & Bind: Stop\r\n");
            }
            else
            {
                u8NoQueryCount++;
            }
            break;

        case BDB_EVENT_FB_TIMEOUT:
            DBG_vPrintf(TRACE_SWITCH_NODE,"ERR: TimeOut\r\n");
            break;

        default:
            DBG_vPrintf(1, "BDB EVT default evt %d\r\n", psBdbEvent->eEventType);
            break;
    }
}

/****************************************************************************
 *
 * NAME: vAppHandleZdoEvents
 *
 * DESCRIPTION:
 * This is the main state machine which decides whether to call up the startup
 * or running function. This depends on whether we are in the network on not.
 *
 * PARAMETERS:
 * ZPS_tsAfEvent sAppStackEvent Stack event information.
 *
 ****************************************************************************/
PRIVATE void vAppHandleAfEvent( BDB_tsZpsAfEvent *psZpsAfEvent)
{
    if (psZpsAfEvent->u8EndPoint == app_u8GetDeviceEndpoint() )
    {
        if ((psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION) ||
            (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION))
        {
            DBG_vPrintf(TRACE_SWITCH_NODE, "Pass to ZCL\r\n");
            APP_ZCL_vEventHandler( &psZpsAfEvent->sStackEvent);
         }
    } else if (psZpsAfEvent->u8EndPoint == SWITCH_ZDO_ENDPOINT)
    {
        // events for ep 0
        vAppHandleZdoEvents( psZpsAfEvent);
    }

    /* Ensure Freeing of Apdus */
    if (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
    {
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
    }
    else if ( psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION )
    {
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsInterPanDataIndEvent.hAPduInst);
    }

}

/****************************************************************************
 *
 * NAME: vAppHandleZdoEvents
 *
 * DESCRIPTION:
 * Application handler for stack events for end point 0 (ZDO)
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vAppHandleZdoEvents( BDB_tsZpsAfEvent *psZpsAfEvent)
{
    //DBG_vPrintf(1, "For ep 0, State %s\r\n", strings[sZllState.eNodeState] );
    /* Handle events depending on node state */
    switch (sDeviceDesc.eNodeState)
    {
    case E_STARTUP:
        break;

    case E_RUNNING:
        DBG_vPrintf(TRACE_SWITCH_NODE, "E_RUNNING\r\n");
        vHandleRunningStackEvent(&psZpsAfEvent->sStackEvent);
        break;
    default:
        break;
    }
}

/****************************************************************************
 *
 * NAME: vHandleRunningStackEvent
 *
 * DESCRIPTION:
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vHandleRunningStackEvent(ZPS_tsAfEvent* psStackEvent)
{
    switch (psStackEvent->eType)
    {
    case ZPS_EVENT_NWK_JOINED_AS_ENDDEVICE:
        vHandleJoinAndRejoin();
        #ifdef APP_NTAG_ICODE
        {
            /* Not a rejoin ? */
            if (FALSE == psStackEvent->uEvent.sNwkJoinedEvent.bRejoin)
            {
                /* Write network data to tag */
                APP_vNtagStart(DIMMERSWITCH_SWITCH_ENDPOINT);
            }
        }
        #endif
        break;

    case ZPS_EVENT_NWK_FAILED_TO_JOIN:
        DBG_vPrintf(TRACE_SWITCH_NODE, "Running Failed to join\r\n");
        if (ZPS_psAplAibGetAib()->u64ApsUseExtendedPanid != 0)
        {
            DBG_vPrintf(TRACE_SWITCH_NODE, "Restore epid %016llx\r\n", ZPS_psAplAibGetAib()->u64ApsUseExtendedPanid);
            ZPS_vNwkNibSetExtPanId(ZPS_pvAplZdoGetNwkHandle(), ZPS_psAplAibGetAib()->u64ApsUseExtendedPanid);
        }
        break;

    case ZPS_EVENT_NWK_LEAVE_INDICATION:
       DBG_vPrintf(TRACE_SWITCH_NODE, "LEAVE IND Addr %016llx Rejoin %02x\r\n",
               psStackEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr,
               psStackEvent->uEvent.sNwkLeaveIndicationEvent.u8Rejoin);

       if ( (psStackEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr == 0UL) &&
            (psStackEvent->uEvent.sNwkLeaveIndicationEvent.u8Rejoin == 0) )
       {
           /* We sare asked to Leave without rejoin */
           DBG_vPrintf(TRACE_SWITCH_NODE, "LEAVE IND -> For Us No Rejoin\r\n");
           DBG_vPrintf(TRACE_SWITCH_NODE, "Leave -> Reset Data Structures\r\n");
           APP_vFactoryResetRecords();
           DBG_vPrintf(TRACE_SWITCH_NODE, "Leave -> Soft Reset\r\n");
       MICRO_DISABLE_INTERRUPTS();
           RESET_SystemReset();
           }
           break;

    case ZPS_EVENT_NWK_LEAVE_CONFIRM:
            /* reset app data and restart */
        APP_vFactoryResetRecords();
        /* force a restart */
        MICRO_DISABLE_INTERRUPTS();
        RESET_SystemReset();
        break;

    case ZPS_EVENT_APS_DATA_INDICATION:
        #ifdef CLD_OTA
        if ((psStackEvent->uEvent.sApsDataIndEvent.eStatus == ZPS_E_SUCCESS) &&
                (psStackEvent->uEvent.sApsDataIndEvent.u8DstEndpoint == 0))
        {
            // Data Ind for ZDp Ep
            if (ZPS_ZDP_MATCH_DESC_RSP_CLUSTER_ID == psStackEvent->uEvent.sApsDataIndEvent.u16ClusterId)
            {
                vHandleMatchDescriptor(psStackEvent);
            } else if (ZPS_ZDP_IEEE_ADDR_RSP_CLUSTER_ID == psStackEvent->uEvent.sApsDataIndEvent.u16ClusterId) {
                vHandleIeeeAddressRsp(psStackEvent);
            }
        }
        #endif
        break;

#ifdef SLEEP_ENABLE
    case ZPS_EVENT_NWK_POLL_CONFIRM:
        if (MAC_ENUM_SUCCESS == psStackEvent->uEvent.sNwkPollConfirmEvent.u8Status)
        {
            bDataPending = TRUE;
        }
        else if (MAC_ENUM_NO_DATA == psStackEvent->uEvent.sNwkPollConfirmEvent.u8Status)
        {
            bDataPending = FALSE;
        }
		
#ifdef SLEEP_MIN_RETENTION
    	if (psStackEvent->uEvent.sNwkPollConfirmEvent.u8Status == MAC_ENUM_NO_ACK)
		u8NumOfPollFailure++;
	else
		u8NumOfPollFailure = 0;

	if(u8NumOfPollFailure > MAX_POLL_FAILURE)
	{
		DBG_vPrintf(TRACE_SWITCH_NODE, "\r\nPoll failure exceed MAX_POLL_FAILURE, start BDB rejoin\r\n");
		BDB_vRejoinCycle(TRUE);
		u8NumOfPollFailure=0;
	}
#endif

    break;
#endif

    default:
        //DBG_vPrintf(1, "Running unhandled %d\r\n", psStackEvent->eType);
        break;
    }
}
/****************************************************************************
 *
 * NAME: APP_ZLO_Switch_Task
 *
 * DESCRIPTION:
 * Task that handles the application related functionality
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_taskSwitch(void)
{

    APP_tsEvent sAppEvent;
    sAppEvent.eType = APP_E_EVENT_NONE;

    if (ZQ_bQueueReceive(&APP_msgAppEvents, &sAppEvent) == TRUE)
    {
        DBG_vPrintf(TRACE_SWITCH_NODE, "ZPR: App event %d, NodeState=%d\r\n", sAppEvent.eType, sDeviceDesc.eNodeState);

#if ((defined APP_NTAG_ICODE) && (APP_BUTTONS_NFC_FD != (0xff)))
        /* Is this a button event on NTAG_FD ? */
        if ( (sAppEvent.eType == APP_E_EVENT_BUTTON_DOWN || sAppEvent.eType == APP_E_EVENT_BUTTON_UP)
                && (sAppEvent.uEvent.sButton.u8Button == APP_E_BUTTONS_NFC_FD) )
        {
            #if APP_NTAG_ICODE
                APP_vNtagStart(DIMMERSWITCH_SWITCH_ENDPOINT);
            #endif
        }
        /* Other event (handle as normal) ? */
        else
#endif
        {
            if (sDeviceDesc.eNodeState == E_RUNNING)
            {
                switch(sAppEvent.eType)
                {
                    case APP_E_EVENT_BUTTON_DOWN:
                    case APP_E_EVENT_BUTTON_UP:

                        vApp_ProcessKeyCombination(sAppEvent);
                        #ifdef SLEEP_ENABLE
                            vReloadSleepTimers();
                        #endif
                        /* TODO:Reset the channel mask to last used so that the
                         * rejoining joining will be attempted
                         * */
                    break;

                    default:
                    break;
                }
            }
            else if(sDeviceDesc.eNodeState == E_STARTUP)
            {
                DBG_vPrintf(TRACE_SWITCH_NODE," Start Steering \r\n");
                sBDB.sAttrib.u32bdbPrimaryChannelSet = BDB_PRIMARY_CHANNEL_SET;
                sBDB.sAttrib.u32bdbSecondaryChannelSet = BDB_SECONDARY_CHANNEL_SET;
                BDB_eNsStartNwkSteering();
            }
        }
    }
}


/****************************************************************************
 *
 * NAME: vHandleJoinAndRejoin
 *
 * DESCRIPTION:
 * Handles the Join and Rejoin events
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vHandleJoinAndRejoin( void  )
{
    DBG_vPrintf(TRACE_SWITCH_NODE, "DEVICE_IN_NETWORK \r\n");
    DBG_vPrintf(TRACE_SWITCH_NODE, "vHandleJoinAndRejoin: eNodeState = E_RUNNING\r\n");
    /* set the aps use pan id */
    ZPS_eAplAibSetApsUseExtendedPanId( ZPS_u64NwkNibGetEpid(ZPS_pvAplZdoGetNwkHandle()) );
    sDeviceDesc.eNodeState = E_RUNNING;

    PDM_eSaveRecordData(PDM_ID_APP_ZLO_SWITCH,
                            &sDeviceDesc,
                            sizeof(tsDeviceDesc));
    ZPS_vSaveAllZpsRecords();
    /* Stop Fast polling */
    vStartFastPolling(0);
    /* Start 1 seconds polling */
    ZTIMER_eStart(u8TimerPoll, POLL_TIME);
    ZTIMER_eStart(u8TimerTick, ZCL_TICK_TIME);
}

#ifdef SLEEP_ENABLE
#ifdef DEEP_SLEEP_ENABLE
/****************************************************************************
 *
 * NAME: vLoadDeepSleepTimer
 *
 * DESCRIPTION:
 * Loads the deep sleep time
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vLoadDeepSleepTimer(uint8 u8SleepTime)
{
    u8DeepSleepTime = u8SleepTime;
    DBG_vPrintf(TRACE_SWITCH_NODE, "\r\ndst = %d A", u8DeepSleepTime);
}
/****************************************************************************
 *
 * NAME: bGoingDeepSleep
 *
 * DESCRIPTION:
 * Checks if the module is going to deep sleep
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC bool bGoingDeepSleep(void)
{
    if (0==u8DeepSleepTime)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#endif
/****************************************************************************
 *
 * NAME: vLoadKeepAliveTime
 *
 * DESCRIPTION:
 * Loads the keep alive timer based on the right conditions.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vLoadKeepAliveTime(uint8 u8TimeInSec)
{
    //uint8 a;
    u8KeepAliveTime=u8TimeInSec;
    ZTIMER_eStop( u8TimerPoll);
    ZTIMER_eStart(u8TimerPoll, POLL_TIME);
    ZTIMER_eStop( u8TimerTick);
    ZTIMER_eStart(u8TimerTick, ZCL_TICK_TIME);
}

/****************************************************************************
 *
 * NAME: bWaitingToSleep
 *
 * DESCRIPTION:
 * Gets the status if the module is waiting for sleep.
 *
 * RETURNS:
 * bool
 *
 ****************************************************************************/
PUBLIC bool bWaitingToSleep(void)
{
    if (0 == u8KeepAliveTime)
        return TRUE;
    else
        return FALSE;
}

/****************************************************************************
 *
 * NAME: vStopAllTimers
 *
 * DESCRIPTION:
 * Stops all the timers
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vStopAllTimers(void)
{
    vStopTimer(u8TimerPoll);
    vStopTimer(u8TimerButtonScan);
    vStopTimer(u8TimerTick);
    //vStopTimer(u8TimerButtonDelay);
    //vStopTimer(u8TimerChangeMode);
}

/****************************************************************************
 *
 * NAME: vUpdateKeepAliveTimer
 *
 * DESCRIPTION:
 * Updates the Keep Alive time at 1 sec call from the tick timer that served ZCL as well.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vUpdateKeepAliveTimer(void)
{
    te_SwitchState eSwitchState = eGetSwitchState();

    if( (eSwitchState == LIGHT_CONTROL_MODE ) || (eSwitchState == INDIVIDUAL_CONTROL_MODE ) )
    {
        /* OTA enabled ? */
        #ifdef CLD_OTA
        {
            /* OTA allowing deep sleep ? */
            if (bOTASleepAllowed())
            {
                if(0 < u8KeepAliveTime)
                {
                    u8KeepAliveTime--;
                }
            }
            /* OTA not allowing deep sleep */
            else
            {
               /* stop sleeping until OTA string is downloaded */
               u8KeepAliveTime = KEEP_ALIVETIME;
            }
        }
        /* OTA disabled ? */
        #else
        {
            if(0 < u8KeepAliveTime)
            {
                u8KeepAliveTime--;
            }
        }
        #endif
        if (u8KeepAliveTime == 0)
        {
            uint16 u16Activities;

            /* Get number of activities */
#ifdef APP_LOW_POWER_API
            u16Activities = mLPMFlag;
#else
            u16Activities = PWRM_u16GetActivityCount();
#endif

            /* Is poll timer running and activities are pending - decrement activities (we don't care about poll timer) */
            if (ZTIMER_eGetState(u8TimerPoll) == E_ZTIMER_STATE_RUNNING && u16Activities > 0) u16Activities--;
            /* Is button scan timer running and activities are pending - decrement activities (we don't care about button scan timer (in current codebase)) */
            if (ZTIMER_eGetState(u8TimerButtonScan) == E_ZTIMER_STATE_RUNNING && u16Activities > 0) u16Activities--;
            /* Is tick timer running and activities are pending - decrement activities (we don't care about tick timer) */
            if (ZTIMER_eGetState(u8TimerTick) == E_ZTIMER_STATE_RUNNING && u16Activities > 0) u16Activities--;

            /* No important activities ? */
            if (u16Activities == 0)
            {
                vStopAllTimers();

                #ifdef DEEP_SLEEP_ENABLE
                    if(u8DeepSleepTime > 0 )
                    {
#ifdef APP_LOW_POWER_API
                        u16Activities = mLPMFlag;
#else
                        u16Activities = PWRM_u16GetActivityCount();
#endif
                        /* The activity counter seems to be still greater than 0 - Go back to be awake, Try again in the next sleep time*/
                        if(u16Activities)
                        {
                            DBG_vPrintf(TRACE_SWITCH_NODE,"\r\n Abort Sleep - Reload timers\r\n");
                            vReloadSleepTimers();
                        }
                        else
                        {
#ifdef APP_LOW_POWER_API
                            PWR_teStatus eStatus = PWR_eScheduleActivity(&sWake, APP_LONG_SLEEP_DURATION_IN_SEC*1000 , vWakeCallBack);
#else
                            PWRM_teStatus eStatus = PWRM_eScheduleActivity(&sWake, APP_LONG_SLEEP_DURATION_IN_SEC*1000 , vWakeCallBack);
#endif
                            DBG_vPrintf(TRACE_SWITCH_NODE,"\r\nSleep Status = %d, u8DeepSleepTime = %d \r\n",eStatus,u8DeepSleepTime);
                        }
                    }
                    else
                    {
#ifdef APP_LOW_POWER_API
                        (void) PWR_ChangeDeepSleepMode(PWR_E_SLEEP_OSCOFF_RAMOFF);
                        PWR_Init();
                        PWR_vForceRadioRetention(FALSE);
#else
                        PWRM_vInit(E_AHI_SLEEP_OSCOFF_RAMOFF);
                        PWRM_vForceRadioRetention(FALSE);
#endif
                    }
                #else
                    /* The activity counter seems to be still greater than 0 - Go back to be awake, Try again in the next sleep time*/
#ifdef APP_LOW_POWER_API
                    u16Activities = mLPMFlag;
#else
                    u16Activities = PWRM_u16GetActivityCount();
#endif
                    if(u16Activities)
                    {
                        vReloadSleepTimers();
                    }
                    else
                    {
                        /* The activity counter is 0 so a sleep with ram on can be scheduled*/
#ifdef APP_LOW_POWER_API
                        PWR_teStatus eStatus = PWR_eScheduleActivity(&sWake, APP_LONG_SLEEP_DURATION_IN_SEC*1000 , vWakeCallBack);
#else
                        PWRM_teStatus eStatus = PWRM_eScheduleActivity(&sWake, APP_LONG_SLEEP_DURATION_IN_SEC*1000 , vWakeCallBack);
#endif
                        DBG_vPrintf(TRACE_SWITCH_NODE,"\r\nSleep Status = %d\r\n",eStatus);
                    }
                #endif
            }
            /* Activities ? */
            else
            {
            }
        }
    }
    else
    {
        vReloadSleepTimers();

    }
}
#endif

/****************************************************************************
 *
 * NAME: vDeletePDMOnButtonPress
 *
 * DESCRIPTION:
 * PDM context clearing on button press
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vDeletePDMOnButtonPress(uint8 u8ButtonDIO)
{
    bool_t bDeleteRecords = FALSE;
    uint8 u8Status;

    uint32 u32Buttons = APP_u32GetSwitchIOState() & (1 << u8ButtonDIO);

    if (u32Buttons == 0)
    {
        bDeleteRecords = TRUE;
    }
    else
    {
        bDeleteRecords = FALSE;
    }
    /* If required, at this point delete the network context from flash, perhaps upon some condition
     * For example, check if a button is being held down at reset, and if so request the Persistent
     * Data Manager to delete all its records:
     * e.g. bDeleteRecords = vCheckButtons();
     * Alternatively, always call PDM_vDeleteAllDataRecords() if context saving is not required.
     */
    if(bDeleteRecords)
    {
        /* wait for button release */
        while (u32Buttons == 0)
        {
            u32Buttons = APP_u32GetSwitchIOState() & (1 << u8ButtonDIO);
        }
        u8Status = ZPS_eAplZdoLeaveNetwork(0, FALSE,FALSE);
        if (ZPS_E_SUCCESS !=  u8Status )
        {
            /* Leave failed,so just reset everything */
            DBG_vPrintf(TRACE_SWITCH_NODE,"Leave failed status %x Deleting the PDM\r\n", u8Status);
            APP_vFactoryResetRecords();
            MICRO_DISABLE_INTERRUPTS();
            RESET_SystemReset();
        } else { DBG_vPrintf(TRACE_SWITCH_NODE, "RESET: Sent Leave\r\n"); }
    }
}



/****************************************************************************
 *
 * NAME: vAppOnOff
 *
 * DESCRIPTION:
 *    Send out ON or OFF command, the address mode(group/unicast/bound etc)
 *    is taken from the selected light index set by the caller
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppOnOff(teCLD_OnOff_Command eCmd) {

    uint8 u8Seq;
    tsZCL_Address sDestinationAddress;
    teZCL_Status eStatus;

    sDestinationAddress.eAddressMode = E_ZCL_AM_BOUND_NON_BLOCKING;

    if ((eCmd == E_CLD_ONOFF_CMD_ON) || (eCmd == E_CLD_ONOFF_CMD_OFF) || (eCmd
            == E_CLD_ONOFF_CMD_TOGGLE)) {
        eStatus = eCLD_OnOffCommandSend(
                u8MyEndpoint,
                0,
                &sDestinationAddress, &u8Seq, eCmd);

        if (eStatus != E_ZCL_SUCCESS)
        {
            DBG_vPrintf(TRACE_SWITCH_NODE, "Send On Off Failed x%02x Last error %02x\r\n",
                            eStatus, eZCL_GetLastZpsError());
        }
    }
}


/****************************************************************************
 *
 * NAME: vAppIdentify
 *
 * DESCRIPTION:
 *    Send out Identify command, the address mode(group/unicast/bound etc)
 *    is taken from the selected light index set by the caller
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppIdentify( uint16 u16Time) {
    uint8 u8Seq;
    tsZCL_Address sDestinationAddress;
    tsCLD_Identify_IdentifyRequestPayload sPayload;
    teZCL_Status eStatus;

    sPayload.u16IdentifyTime = u16Time;

  //  vSetAddress(&sAddress,FALSE,GENERAL_CLUSTER_ID_IDENTIFY);

    eStatus = eCLD_IdentifyCommandIdentifyRequestSend(
                            u8MyEndpoint,
                  1,         // sDeviceInfo.sLightInfo[sDeviceInfo.u8Index].u8Ep,
                            &sDestinationAddress,
                            &u8Seq,
                            &sPayload);

    if (eStatus != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_SWITCH_NODE, "Send Identify Failed x%02x Last error %02x\r\n",
                        eStatus, eZCL_GetLastZpsError());
    }
}

/****************************************************************************
 *
 * NAME: vAppLevelMove
 *
 * DESCRIPTION:
 *    Send out Level Up or Down command, the address mode(group/unicast/bound etc)
 *    is taken from the selected light index set by the caller
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppLevelMove(teCLD_LevelControl_MoveMode eMode, uint8 u8Rate, bool_t bWithOnOff)
{
    tsCLD_LevelControl_MoveCommandPayload sPayload = {0};
    uint8 u8Seq;
    tsZCL_Address sDestinationAddress;
    teZCL_Status eStatus;

    sDestinationAddress.eAddressMode = E_ZCL_AM_BOUND_NON_BLOCKING;

    sPayload.u8Rate = u8Rate;
    sPayload.u8MoveMode = eMode;

    eStatus = eCLD_LevelControlCommandMoveCommandSend(
                                    u8MyEndpoint,
                                    0,
                                    &sDestinationAddress,
                                    &u8Seq,
                                    bWithOnOff, /* with on off */
                                    &sPayload);
    if (eStatus != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_SWITCH_NODE, "Send Level Move Failed x%02x Last error %02x\r\n",
                        eStatus, eZCL_GetLastZpsError());
    }
}


/****************************************************************************
 *
 * NAME: vAppLevelStop
 *
 * DESCRIPTION:
 *    Send out Level Stop command, the address mode(group/unicast/bound etc)
 *    is taken from the selected light index set by the caller
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppLevelStop(void)
{
    tsCLD_LevelControl_StopCommandPayload sPayload = {0};
    uint8 u8Seq;
    tsZCL_Address sDestinationAddress;
    teZCL_Status eStatus;

    sDestinationAddress.eAddressMode = E_ZCL_AM_BOUND_NON_BLOCKING;
    eStatus = eCLD_LevelControlCommandStopCommandSend(
                        u8MyEndpoint,
                        0,
                        &sDestinationAddress,
                        &u8Seq,
                        FALSE, /* without on off */
                        &sPayload);

    if (eStatus != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_SWITCH_NODE, "Send Level Stop Failed x%02x Last error %02x\r\n",
                        eStatus, eZCL_GetLastZpsError());
    }
}






/****************************************************************************
 *
 * NAME: vAppAddGroup
 *
 * DESCRIPTION:
 *    Send out Add Group command, the address mode is unicast addressing and
 *    is taken from the selected light index set by the caller
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppAddGroup( uint16 u16GroupId, bool_t bBroadcast)
{

    tsCLD_Groups_AddGroupRequestPayload sPayload;
    uint8 u8Seq;
    tsZCL_Address sAddress;
    teZCL_Status eStatus;

    //vSetAddress(&sAddress, bBroadcast,GENERAL_CLUSTER_ID_GROUPS);

    sPayload.sGroupName.pu8Data = (uint8*)"";
    sPayload.sGroupName.u8Length = 0;
    sPayload.sGroupName.u8MaxLength = 0;
    sPayload.u16GroupId = u16GroupId;

    eStatus = eCLD_GroupsCommandAddGroupRequestSend(
                            u8MyEndpoint,
                  1 ,  //       sDeviceInfo.sLightInfo[sDeviceInfo.u8Index].u8Ep,
                            &sAddress,
                            &u8Seq,
                            &sPayload);

    if (eStatus != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_SWITCH_NODE, "Send Add Group Failed x%02x Last error %02x\r\n",
                        eStatus, eZCL_GetLastZpsError());
    }

}

/****************************************************************************
 *
 * NAME: vAppRemoveGroup
 *
 * DESCRIPTION:
 *    Send out remove group command, the address mode (group/unicast/bound etc)
 *    is taken from the selected light index set by the caller
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppRemoveGroup( uint16 u16GroupId, bool_t bBroadcast)
{

    tsCLD_Groups_RemoveGroupRequestPayload sPayload;
    uint8 u8Seq;
    tsZCL_Address sAddress;
    teZCL_Status eStatus;

   // vSetAddress(&sAddress, bBroadcast,GENERAL_CLUSTER_ID_GROUPS);

    sPayload.u16GroupId = u16GroupId;

    eStatus = eCLD_GroupsCommandRemoveGroupRequestSend(
                            u8MyEndpoint,
                     1,//       sDeviceInfo.sLightInfo[sDeviceInfo.u8Index].u8Ep,
                            &sAddress,
                            &u8Seq,
                            &sPayload);

    if (eStatus != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_SWITCH_NODE, "Send Remove Group Failed x%02x Last error %02x\r\n",
                        eStatus, eZCL_GetLastZpsError());
    }

}

/****************************************************************************
 *
 * NAME: vAppRemoveAllGroups
 *
 * DESCRIPTION:
 *    Send out Remove All group command, the address mode(group/unicast/bound etc)
 *    is taken from the selected light index set by the caller
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppRemoveAllGroups(bool_t bBroadcast)
{

    uint8 u8Seq;
    tsZCL_Address sAddress;
    teZCL_Status eStatus;

   // vSetAddress(&sAddress, bBroadcast,GENERAL_CLUSTER_ID_GROUPS);

    eStatus = eCLD_GroupsCommandRemoveAllGroupsRequestSend(
                            u8MyEndpoint,
                   1,//         sDeviceInfo.sLightInfo[sDeviceInfo.u8Index].u8Ep,
                            &sAddress,
                            &u8Seq);

    if (eStatus != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_SWITCH_NODE, "Send Remove All Groups Failed x%02x Last error %02x\r\n",
                        eStatus, eZCL_GetLastZpsError());
    }

}

/****************************************************************************
 *
 * NAME: vStopTimer
 *
 * DESCRIPTION:
 * Stops the timer
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vStopTimer(uint8 u8Timer)
{
    ZTIMER_eStop(u8Timer);
}


/****************************************************************************
 *
 * NAME: vManageWakeUponSysControlISR
 *
 * DESCRIPTION:
 * Called from SysControl ISR to process the wake up conditions
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vManageWakeUponSysControlISR(teInterruptType eInterruptType)
{
    #ifdef SLEEP_ENABLE
        /*In any case this could be a wake up from timer interrupt or from buttons
         * press
         * */
        if(TRUE == bWakeUpFromSleep())
        {
            /*Only called if the module is coming out of sleep */
            DBG_vPrintf(TRACE_SWITCH_NODE,"vISR_SystemController on WakeUP\r\n\r\n");
            vLoadKeepAliveTime(KEEP_ALIVETIME);
            vWakeCallBack();
        }
    #endif
}
#ifdef SLEEP_ENABLE
/****************************************************************************
 *
 * NAME: vWakeCallBack
 *
 * DESCRIPTION:
 * Wake up call back called upon wake up by the schedule activity event.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vWakeCallBack(void)
{
    /*Decrement the deepsleep count so that if there is no activity for
     * DEEP_SLEEPTIME then the module is put to deep sleep.
     * */
    if (0 < u8DeepSleepTime)
    {
        /* Deep sleep disabled ? */
        #if NEVER_DEEP_SLEEP
        {
            /* Reset deep sleep countdown */
            u8DeepSleepTime = DEEP_SLEEP_TIME;
//            DBG_vPrintf(TRACE_SWITCH_NODE, "\r\ndst = %d B", u8DeepSleepTime);
        }
        /* Deep sleep enabled ? */
        #else
        {
            /* OTA enabled ? */
            #ifdef CLD_OTA
            {
                /* OTA allowing deep sleep ? */
                if (bOTADeepSleepAllowed())
                {
                    /* Decrement deep sleep countdown */
                    u8DeepSleepTime--;
//                    DBG_vPrintf(TRACE_SWITCH_NODE, "\r\ndst = %d C", u8DeepSleepTime);
                }
                /* OTA not allowing deep sleep */
                else
                {
                    /* Reset deep sleep countdown */
                    u8DeepSleepTime = DEEP_SLEEP_TIME;
//                    DBG_vPrintf(TRACE_SWITCH_NODE, "\r\ndst = %d D", u8DeepSleepTime);
                }
            }
            /* OTA disabled ? */
            #else
            {
                /* Decrement deep sleep countdown */
                u8DeepSleepTime--;
//                DBG_vPrintf(TRACE_SWITCH_NODE, "\r\ndst = %d E", u8DeepSleepTime);
            }
            #endif
        }
        #endif
    }
    /* OTA enabled ? */
    #ifdef CLD_OTA
        /* Run OTA processing */
        vRunAppOTAStateMachine(APP_LONG_SLEEP_DURATION_IN_SEC*1000);
    #endif
}
#endif
/****************************************************************************
 *
 * NAME: APP_cbTimerPoll
 *
 * DESCRIPTION:
 * Poll Task for the polling as well it triggers the rejoin in case of pool failure
 * It also manages sleep timing.
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_cbTimerPoll(void *pvParam)
{
    uint32 u32PollPeriod = POLL_TIME;


    if(
    #ifdef SLEEP_ENABLE
      !bWaitingToSleep() &&
    #endif
       /* Do fast polling when the device is running */
      ((sDeviceDesc.eNodeState == E_RUNNING) || (sDeviceDesc.eNodeState == E_WAIT_STARTUP)))

    {

        if( u16FastPoll )
        {
            u16FastPoll--;
            u32PollPeriod = POLL_TIME_FAST;
            /*Reload the Sleep timer during fast poll*/
            #ifdef SLEEP_ENABLE
                vReloadSleepTimers();
            #endif
        }
        ZTIMER_eStop(u8TimerPoll);
        ZTIMER_eStart(u8TimerPoll, u32PollPeriod);

        ZPS_teStatus u8PStatus;
        u8PStatus = ZPS_eAplZdoPoll();
        if( u8PStatus )
        {
            DBG_vPrintf(TRACE_SWITCH_NODE, "\r\nPoll Failed %d\r\n", u8PStatus );
        }
    }
}

/****************************************************************************
*
* NAME: vAppChangeChannel
*
* DESCRIPTION: This function change the channel randomly to one of the other
* primaries
*
* RETURNS:
* void
*
****************************************************************************/
PUBLIC void vAppChangeChannel( void)
{
    /*Primary channel Set */
    uint8 au8ZHAChannelSet[]={11,14,15,19,20,24,25};

    ZPS_tsAplZdpMgmtNwkUpdateReq sZdpMgmtNwkUpdateReq;
    PDUM_thAPduInstance hAPduInst;
    ZPS_tuAddress uDstAddr;
    uint8 u8Seq;
    uint8 u8Min=0, u8Max=6;
    uint8 u8CurrentChannel, u8RandomNum;

    hAPduInst = PDUM_hAPduAllocateAPduInstance(apduZDP);
    if (hAPduInst != NULL)
    {
        sZdpMgmtNwkUpdateReq.u8ScanDuration = 0xfe;

        u8CurrentChannel = ZPS_u8AplZdoGetRadioChannel();
        u8RandomNum = RND_u32GetRand(u8Min,u8Max);
        if(u8CurrentChannel != au8ZHAChannelSet[u8RandomNum])
        {
            sZdpMgmtNwkUpdateReq.u32ScanChannels = (1<<au8ZHAChannelSet[u8RandomNum]);
        }
        else /* Increment the channel by one rather than spending in RND_u32GetRand */
        {
            /*  For roll over situation */
            if(u8RandomNum == u8Max)
            {
                sZdpMgmtNwkUpdateReq.u32ScanChannels = (1<<au8ZHAChannelSet[u8Min]);
            }
            else
            {
                sZdpMgmtNwkUpdateReq.u32ScanChannels = (1<<au8ZHAChannelSet[u8RandomNum+1]);
            }
        }

        sZdpMgmtNwkUpdateReq.u8NwkUpdateId = ZPS_psAplZdoGetNib()->sPersist.u8UpdateId + 1;
        uDstAddr.u16Addr = 0xfffd;

        if ( 0 == ZPS_eAplZdpMgmtNwkUpdateRequest( hAPduInst,
                                         uDstAddr,
                                         FALSE,
                                         &u8Seq,
                                         &sZdpMgmtNwkUpdateReq))
        {
            DBG_vPrintf(TRACE_SWITCH_NODE, "update Id\r\n");
            /* should really be in stack?? */
            ZPS_psAplZdoGetNib()->sPersist.u8UpdateId++;
        }
        else
        {
            PDUM_eAPduFreeAPduInstance(hAPduInst);
        }
    }
}
#ifdef SLEEP_ENABLE
/****************************************************************************
*
* NAME: vReloadSleepTimers
*
* DESCRIPTION:
* reloads boththe timers on identify
*
* RETURNS:
* void
*
****************************************************************************/
PUBLIC void vReloadSleepTimers(void)
{
    vLoadKeepAliveTime(KEEP_ALIVETIME);
    #ifdef DEEP_SLEEP_ENABLE
        vLoadDeepSleepTimer(DEEP_SLEEP_TIME);
    #endif
}

/****************************************************************************
 *
 * NAME: APP_vStartUpHW
 *
 * DESCRIPTION:
 * Task activated by the wake up event to manage sleep
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vStartUpHW(void)
{
//  uint8 u8Status;

    /* Restart the keyboard scanning timer as we've come up through */
    /* warm start via the Power Manager if we get here              */
//  u8Status =
    ZPS_eAplZdoPoll();
//  DBG_vPrintf(TRACE_SWITCH_NODE, " Wake poll %02x\r\n", u8Status);
    /*Start Polling*/
    ZTIMER_eStart(u8TimerPoll, POLL_TIME);

    /*Start the APP_TickTimer to continue the ZCL tasks */
    ZTIMER_eStart(u8TimerTick, ZCL_TICK_TIME);
}
#endif

/****************************************************************************
 *
 * NAME: app_vStartNodeFactoryNew
 *
 * DESCRIPTION:
 * Start the ZigBee Stack for the first ever Time.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void app_vStartNodeFactoryNew(void)
{
    /* Stay awake for joining */
    DBG_vPrintf(TRACE_SWITCH_NODE, "\r\nFactory New Start");
    DBG_vPrintf(TRACE_SWITCH_NODE, "app_vStartNodeFactoryNew: eNodeState = E_STARTUP\r\n");
    sDeviceDesc.eNodeState = E_STARTUP;
}

#ifdef CLD_OTA
/****************************************************************************
 *
 * NAME: sGetOTACallBackPersistdata
 *
 * DESCRIPTION:
 * returns a copy to the OTA persistent data from the device structure.
 *
 * RETURNS:
 * tsOTA_PersistedData
 *
 ****************************************************************************/
PUBLIC tsOTA_PersistedData sGetOTACallBackPersistdata(void)
{
    return sSwitch.sCLD_OTA_CustomDataStruct.sOTACallBackMessage.sPersistedData;
}
#endif

/****************************************************************************
 *
 * NAME: APP_vFactoryResetRecords
 *
 * DESCRIPTION: reset application and stack to factory new state
 *              preserving the outgoing nwk frame counter
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vFactoryResetRecords( void)
{
    /* clear out the stack */
    ZPS_vDefaultStack();
    ZPS_eAplAibSetApsUseExtendedPanId(0);
    ZPS_vSetKeys();

    /* clear out the application */
    DBG_vPrintf(TRACE_SWITCH_NODE, "vFactoryResetRecords: eNodeState = E_STARTUP\r\n");
    sDeviceDesc.eNodeState = E_STARTUP;

#ifdef CLD_OTA
    vResetOTADiscovery();
    vOTAResetPersist();
#endif

    /* save everything */
    PDM_eSaveRecordData(PDM_ID_APP_ZLO_SWITCH,
                            &sDeviceDesc,
                            sizeof(tsDeviceDesc));
    ZPS_vSaveAllZpsRecords();
}

/****************************************************************************
 *
 * NAME: APP_bIsRunning
 *
 * DESCRIPTION: Returns TRUE if in running state
 *
 * RETURNS:
 * Running state
 *
 ****************************************************************************/
PUBLIC bool APP_bIsRunning(void)
{
    bool bReturn = FALSE;

    if (sDeviceDesc.eNodeState == E_RUNNING)
    {
        bReturn = TRUE;
    }

    return bReturn;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
