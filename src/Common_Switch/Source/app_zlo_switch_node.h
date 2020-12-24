/*****************************************************************************
 *
 * MODULE:             JN-AN-1245
 *
 * COMPONENT:          app_zlo_switch_node.c
 *
 * DESCRIPTION:        ZLO Controller & Switch Demo : Stack <-> Remote Control App Interaction
 *                     (Interface)
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

#ifndef APP_SENSOR_NODE_H_
#define APP_SENSOR_NODE_H_

#include "zps_nwk_sap.h"
#include "zps_nwk_nib.h"
#include "zps_nwk_pub.h"
#include "zps_apl.h"
#include "zps_apl_zdo.h"
#include "app_common.h"
#include "app_events.h"
#include "zcl_customcommand.h"
#ifdef PDM_EEPROM
 #include "app_pdm.h"
#endif
#ifdef CLD_OTA
#include "OTA.h"
#endif
#include "bdb_api.h"
#include "bdb_fb_api.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifdef DEEP_SLEEP_ENABLE
#ifndef CLD_OTA
    #define DEEP_SLEEP_TIME         10 /* drop in the deep sleep after 10*12 secs + last iteration = 2 minutes to check for ota server */
#else
    #define DEEP_SLEEP_TIME         5 /* drop in the deep sleep after 5*12 secs + last iteration = 1 minutes */
#endif
#endif

#define APP_IDENTIFY_TIME_IN_SECS   5
#define NUMBER_DEVICE_TO_BE_DISCOVERED 8 // Should be same as Binding table size
#define SLEEP_DURATION_MS           (APP_LONG_SLEEP_DURATION_IN_SEC*1000)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC void APP_vInitialiseNode(bool_t bColdStart);
PUBLIC void APP_taskSwitch(void);
PUBLIC bool bLightsDiscovered(void);
PUBLIC void vStartFastPolling(uint8 u8Seconds);

PUBLIC void vAppOnOff(teCLD_OnOff_Command eCmd);

PUBLIC void vAppLevelMove(teCLD_LevelControl_MoveMode eMode, uint8 u8Rate, bool_t bWithOnOff);
PUBLIC void vAppLevelStop(void);

PUBLIC void vAppIdentify(uint16 u16Time);

PUBLIC void vAppAddGroup(uint16 u16GroupId, bool_t bBroadcast);
PUBLIC void vAppRemoveGroup(uint16 u16GroupId, bool_t bBroadcast);
PUBLIC void vAppRemoveAllGroups(bool_t bBroadcast);

#ifdef SLEEP_ENABLE
    PUBLIC void vReloadSleepTimers(void);
    PUBLIC void vUpdateKeepAliveTimer(void);
    PUBLIC bool bWaitingToSleep(void);
    PUBLIC void vWakeCallBack(void);
    #ifdef DEEP_SLEEP_ENABLE
        PUBLIC void vLoadDeepSleepTimer(uint8 u8SleepTime);
        PUBLIC bool bGoingDeepSleep(void);
    #endif
    PUBLIC void APP_vStartUpHW(void);
#endif

#ifdef CLD_OTA
    PUBLIC tsOTA_PersistedData sGetOTACallBackPersistdata(void);
#endif

PUBLIC void APP_cbTimerPoll(void *pvParam);
PUBLIC void APP_vSleepTask(void);
PUBLIC void APP_vFactoryResetRecords( void);
PUBLIC bool APP_bIsRunning(void);

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/
extern tsDeviceDesc sDeviceDesc;
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /*APP_SENSOR_NDOE_H_*/
