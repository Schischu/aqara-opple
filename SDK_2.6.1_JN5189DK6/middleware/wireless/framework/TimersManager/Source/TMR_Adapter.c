/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2017 NXP
* All rights reserved.
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#include "TMR_Adapter.h"
#include "fsl_device_registers.h"
#include "fsl_os_abstraction.h"
#include "fsl_common.h"
//#include "board.h"
#include "TimersManager.h"

#if gTimerMgrUseFtm_c
  #include "fsl_ftm.h"
#elif gTimerMgrUseLpcRtc_c
  #include "fsl_rtc.h"
#elif gTimerMgrUseRtcFrc_c
  #include "fsl_rtc.h"
  #include "clock_config.h"
#elif gTimerMgrUseCtimer_c
  #include "fsl_ctimer.h"
#else
   #include "fsl_tpm.h"
#endif


/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
#if gTimerMgrUseFtm_c
static const IRQn_Type mFtmIrqId[] = FTM_IRQS;
static FTM_Type * mFtmBase[] = FTM_BASE_PTRS;
static const ftm_config_t mFtmConfig = {
    .prescale = kFTM_Prescale_Divide_128,
    .bdmMode = kFTM_BdmMode_0,
    .pwmSyncMode = kFTM_SoftwareTrigger,
    .reloadPoints = 0,
    .faultMode = kFTM_Fault_Disable,
    .faultFilterValue = 0,
    .deadTimePrescale = kFTM_Deadtime_Prescale_1,
    .deadTimeValue = 0,
    .extTriggers = 0,
    .chnlInitState = 0,
    .chnlPolarity = 0,
    .useGlobalTimeBase = 0
};

#elif gTimerMgrUseLpcRtc_c
static const IRQn_Type  mRtcFrIrqId = RTC_IRQn;

#elif gTimerMgrUseRtcFrc_c
static const IRQn_Type  mRtcFrIrqId = RTC_FR_IRQn;
static       RTC_Type   *mRtcBase[] = RTC_BASE_PTRS;

#elif gTimerMgrUseCtimer_c
static const IRQn_Type          mCtimerIrqId[]  = CTIMER_IRQS;
static CTIMER_Type              *mCtimerBase[]  = CTIMER_BASE_PTRS;
static const ctimer_config_t    mCtimerConfig[FSL_FEATURE_SOC_CTIMER_COUNT]
                                                = {{.mode =  kCTIMER_TimerMode,
                                                    .input = kCTIMER_Capture_0,
                                                    .prescale = 0},
                                                   {.mode =  kCTIMER_TimerMode,
                                                    .input = kCTIMER_Capture_0,
                                                    .prescale = 0},
#if(FSL_FEATURE_SOC_CTIMER_COUNT > 2)
                                                   {.mode =  kCTIMER_TimerMode,
                                                    .input = kCTIMER_Capture_0,
                                                    .prescale = 0},
                                                   {.mode =  kCTIMER_TimerMode,
                                                    .input = kCTIMER_Capture_0,
                                                    .prescale = 0}
#endif
};
static ctimer_match_config_t mCtimerMatchConfig = {.enableCounterReset = true,
                                                   .enableCounterStop = true,
                                                   .matchValue = 0xff,
                                                   .outControl = kCTIMER_Output_NoAction,
                                                   .outPinInitState = false,
                                                   .enableInterrupt = true};

/*! @brief List of Timer Match channel interrupts,
 * this can be accessed using the channel number as index*/
static const ctimer_interrupt_enable_t ctimer_match_ch_interrupts[] = {
    kCTIMER_Match0InterruptEnable,
    kCTIMER_Match1InterruptEnable,
    kCTIMER_Match2InterruptEnable,
    kCTIMER_Match3InterruptEnable,
};

#ifndef ENABLE_RAM_VECTOR_TABLE
ctimer_callback_t cTimerCallbacks[2] = {StackTimer_ISR_withParam, NULL};
#endif

#else
static const IRQn_Type mTpmIrqId[] = TPM_IRQS;
static TPM_Type * mTpmBase[] = TPM_BASE_PTRS;
static const tpm_config_t mTpmConfig = {
    .prescale = kTPM_Prescale_Divide_128,
    .useGlobalTimeBase = 0,
    .enableDoze = 0,
    .enableDebugMode = 0,
    .enableReloadOnTrigger = 0,
    .enableStopOnOverflow = 0,
    .enableStartOnTrigger = 0,
    .triggerSelect = kTPM_Trigger_Select_0
};
#endif


static void ConfigureIntHandler(IRQn_Type irqId, void (*cb)(void))
{
    /* Overwrite old ISR */
    OSA_InstallIntHandler(irqId, cb);
    /* set interrupt priority */
    NVIC_SetPriority(irqId, gStackTimer_IsrPrio_c >> (8 - __NVIC_PRIO_BITS));
    NVIC_ClearPendingIRQ(irqId);
    NVIC_EnableIRQ(irqId);
}

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
void StackTimer_Init(void (*cb)(void))
{
    TMR_DBG_LOG("cb=%x", cb);
    IRQn_Type irqId;
#if gTimerMgrUseFtm_c
    FTM_Type *ftmBaseAddr = (FTM_Type*)mFtmBase[gStackTimerInstance_c];

    FTM_Init(ftmBaseAddr, &mFtmConfig);
    FTM_StopTimer(ftmBaseAddr);
    ftmBaseAddr->MOD = 0xFFFF;
    /* Configure channel to Software compare; output pin not used */
    FTM_SetupOutputCompare(ftmBaseAddr, (ftm_chnl_t)gStackTimerChannel_c, kFTM_NoOutputSignal, 0x01);

    /* Install ISR */
    irqId = mFtmIrqId[gStackTimerInstance_c];
    FTM_EnableInterrupts(ftmBaseAddr, kFTM_TimeOverflowInterruptEnable |  (1 << gStackTimerChannel_c));
#elif gTimerMgrUseLpcRtc_c
    /* RTC time counter has to be stopped before setting the date & time in the TSR register */
    /* RTC clocks dividers */
    CLOCK_SetClkDiv(kCLOCK_DivRtcClk, 32, false);
    CLOCK_SetClkDiv(kCLOCK_DivRtc1HzClk, 1, true);

    RTC_StopTimer(RTC);
    RTC_Init(RTC);

    /* Enable RTC interrupt */
#if gTimestamp_Enabled_d
    RTC_EnableInterrupts(RTC, kRTC_WakeupInterruptEnable | kRTC_AlarmInterruptEnable);
#else
    RTC_EnableInterrupts(RTC, kRTC_WakeupInterruptEnable);
#endif
    irqId = mRtcFrIrqId;
#elif gTimerMgrUseRtcFrc_c
    RTC_Init(RTC);
    /* Enable RTC free running interrupt */
    RTC_EnableInterrupts(RTC, kRTC_FreeRunningInterruptEnable);
    irqId = mRtcFrIrqId;
#elif gTimerMgrUseCtimer_c
    CTIMER_Type *ctimerBaseAddr = mCtimerBase[gStackTimerInstance_c];

    CTIMER_Init(ctimerBaseAddr, &mCtimerConfig[gStackTimerInstance_c]);
    CTIMER_StopTimer(ctimerBaseAddr);

    /* Configure channel to Software compare; output pin not used */
    CTIMER_SetupMatch(ctimerBaseAddr, (ctimer_match_t)gStackTimerChannel_c, &mCtimerMatchConfig);

    /* Install ISR */
    irqId = mCtimerIrqId[gStackTimerInstance_c];
    CTIMER_EnableInterrupts(ctimerBaseAddr, ctimer_match_ch_interrupts[gStackTimerChannel_c]);
#ifndef ENABLE_RAM_VECTOR_TABLE
    CTIMER_RegisterCallBack(ctimerBaseAddr, &cTimerCallbacks[0], kCTIMER_SingleCallback);
#endif

#else
    TPM_Type *tpmBaseAddr = (TPM_Type*)mTpmBase[gStackTimerInstance_c];

    TPM_Init(tpmBaseAddr, &mTpmConfig);
    TPM_StopTimer(tpmBaseAddr);

    /* Set the timer to be in free-running mode */
    tpmBaseAddr->MOD = 0xFFFF;
    /* Configure channel to Software compare; output pin not used */
    TPM_SetupOutputCompare(tpmBaseAddr, (tpm_chnl_t)gStackTimerChannel_c, kTPM_NoOutputSignal, 0x01);
    
    /* Install ISR */
    irqId = mTpmIrqId[gStackTimerInstance_c];
    TPM_EnableInterrupts(tpmBaseAddr, kTPM_TimeOverflowInterruptEnable | (1 << gStackTimerChannel_c));
#endif

    ConfigureIntHandler( irqId, cb);
}

/*************************************************************************************/
int StackTimer_ReInit(void (*cb)(void))
{

    uint32_t result = 0;

#if gTimerMgrUseLpcRtc_c
    IRQn_Type irqId;

    /* RTC time counter has to be stopped before setting the date & time in the TSR register */
    irqId = mRtcFrIrqId;
    ConfigureIntHandler( irqId, cb);
#else
    result = -1;  // failure : Timers which are not lowpower shall be fully reinitialized
#endif
    TMR_DBG_LOG("cb=%x res=%x", cb, result);
    return result;
}

/*************************************************************************************/
void StackTimer_Enable(void)
{
    TMR_DBG_LOG("");
#if gTimerMgrUseFtm_c
    FTM_StartTimer(mFtmBase[gStackTimerInstance_c], kFTM_SystemClock);
#elif gTimerMgrUseLpcRtc_c
    RTC_StartTimer(RTC);
#elif gTimerMgrUseRtcFrc_c
    RTC_FreeRunningEnable(mRtcBase[0], true);
#elif gTimerMgrUseCtimer_c
    CTIMER_StartTimer(mCtimerBase[gStackTimerInstance_c]);
#else
    TPM_StartTimer(mTpmBase[gStackTimerInstance_c], kTPM_SystemClock);
#endif
}

/*************************************************************************************/
void StackTimer_Disable(void)
{
    TMR_DBG_LOG("");
#if gTimerMgrUseFtm_c
    FTM_StopTimer(mFtmBase[gStackTimerInstance_c]);
#elif gTimerMgrUseLpcRtc_c
    RTC_StopTimer(RTC);
#elif gTimerMgrUseRtcFrc_c
    RTC_FreeRunningEnable(mRtcBase[0], false);
#elif gTimerMgrUseCtimer_c
    CTIMER_StopTimer(mCtimerBase[gStackTimerInstance_c]);
#else
    TPM_StopTimer(mTpmBase[gStackTimerInstance_c]);
#endif
}

/*************************************************************************************/
uint32_t StackTimer_GetInputFrequency(void)
{
    uint32_t prescaller = 0;
    uint32_t refClk     = 0;
    uint32_t result     = 0;
#if gTimerMgrUseFtm_c
    refClk = BOARD_GetFtmClock(gStackTimerInstance_c);
    prescaller = mFtmConfig.prescale;
    result = refClk / (1 << prescaller);
#elif gTimerMgrUseLpcRtc_c
    (void)prescaller; /* unused variables  */
    (void)refClk;     /* suppress warnings */
    result = 1000;    /* The high-resolution RTC timer uses a 1kHz clock */
#elif gTimerMgrUseRtcFrc_c
    
    (void)prescaller; /* unused variables  */
    (void)refClk;     /* suppress warnings */
  #if (defined(BOARD_XTAL1_CLK_HZ) && (BOARD_XTAL1_CLK_HZ == CLK_XTAL_32KHZ))
    result = CLOCK_GetFreq(kCLOCK_32KClk);  //32,768Khz crystal is used
  #else
    if( RTC->CTRL | RTC_CTRL_CAL_EN_MASK) // is calibration enabled ?
    { 
        /* result = 32000 +- ( (32768-32000)*(calculated_ppm / ppm_for_32_000))
         *        = 32000 +- ( 768 * calculated_ppm / 0x6000 )
         *        = 32000 +- (3 * calculated_ppm / 0x60)
         */
        if (RTC->CAL & RTC_CAL_DIR_MASK) //backward calibration
        {
            result = 32000U - ((3 * (RTC->CAL & RTC_CAL_PPM_MASK)) / 0x60);
        }
        else
        {
            result = 32000U + ((3 * (RTC->CAL & RTC_CAL_PPM_MASK)) / 0x60);
        }
    }
    else
    {
        result = CLOCK_GetFreq(kCLOCK_32KClk);  //32,000Khz internal RCO is used
    }
  #endif
#elif gTimerMgrUseCtimer_c
    refClk = BOARD_GetCtimerClock(mCtimerBase[gStackTimerInstance_c]);
    prescaller = mCtimerConfig[gStackTimerInstance_c].prescale;
    result = refClk / (prescaller + 1);
#else
    refClk = BOARD_GetTpmClock(gStackTimerInstance_c);
    prescaller = mTpmConfig.prescale;
    result = refClk / (1 << prescaller);
#endif
    TMR_DBG_LOG("%d", result);
    return result;
}

/*************************************************************************************/
uint32_t StackTimer_GetCounterValue(void)
{
    uint32_t counter_value = 0;
#if gTimerMgrUseFtm_c
    counter_value = mFtmBase[gStackTimerInstance_c]->CNT;
#elif gTimerMgrUseLpcRtc_c
    counter_value = RTC_GetWakeupCount(RTC);
#elif gTimerMgrUseRtcFrc_c
    counter_value = RTC_GetFreeRunningCount(mRtcBase[0]);
#elif gTimerMgrUseCtimer_c
    counter_value = mCtimerBase[gStackTimerInstance_c]->TC;
#else
    counter_value = mTpmBase[gStackTimerInstance_c]->CNT;
#endif
    TMR_DBG_LOG("%d", counter_value);

    return counter_value;
}

/*************************************************************************************/
void StackTimer_SetOffsetTicks(uint32_t offset)
{
#if gTimerMgrUseFtm_c
    FTM_SetupOutputCompare(mFtmBase[gStackTimerInstance_c], (ftm_chnl_t)gStackTimerChannel_c, kFTM_NoOutputSignal, offset);
#elif gTimerMgrUseLpcRtc_c
    RTC_SetWakeupCount(RTC, (uint16_t)offset);
#elif gTimerMgrUseRtcFrc_c
    RTC_SetFreeRunningInterruptThreshold(mRtcBase[0], offset);
#elif gTimerMgrUseCtimer_c
    mCtimerMatchConfig.matchValue = offset;
    CTIMER_SetupMatch(mCtimerBase[gStackTimerInstance_c], (ctimer_match_t)gStackTimerInstance_c, &mCtimerMatchConfig);
#else
    TPM_SetupOutputCompare(mTpmBase[gStackTimerInstance_c], (tpm_chnl_t)gStackTimerChannel_c, kTPM_NoOutputSignal, offset);
#endif
    TMR_DBG_LOG("%d", offset);

}

/*************************************************************************************/
uint32_t StackTimer_ClearIntFlag(void)
{
    uint32_t flags = 0;
#if gTimerMgrUseFtm_c
    flags = FTM_GetStatusFlags(mFtmBase[gStackTimerInstance_c]);
    FTM_ClearStatusFlags(mFtmBase[gStackTimerInstance_c], flags);
#elif gTimerMgrUseLpcRtc_c
    flags = RTC_GetStatusFlags(RTC);
    if(flags & kRTC_WakeupFlag)
        RTC_ClearStatusFlags(RTC, kRTC_WakeupFlag);
    if(flags & kRTC_AlarmFlag)
        RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
#elif gTimerMgrUseRtcFrc_c
    flags = RTC_STATUS_FREE_RUNNING_INT_MASK;
    RTC_ClearStatusFlags(mRtcBase[0], flags);
#elif gTimerMgrUseCtimer_c
    flags = CTIMER_GetStatusFlags(mCtimerBase[gStackTimerInstance_c]);
    CTIMER_ClearStatusFlags(mCtimerBase[gStackTimerInstance_c], flags);
#else
    flags = TPM_GetStatusFlags(mTpmBase[gStackTimerInstance_c]);
    TPM_ClearStatusFlags(mTpmBase[gStackTimerInstance_c], flags);
#endif
    TMR_DBG_LOG("flags=%x", flags);
    return flags;
}

/*************************************************************************************/
/*                                       PWM                                         */
/*************************************************************************************/
#if (FSL_FEATURE_SOC_PWM_COUNT < 1)
void PWM_Init(uint8_t instance)
{
#if gTimerMgrUseFtm_c
    ftm_config_t config;
    FTM_GetDefaultConfig(&config);
    FTM_Init(mFtmBase[instance], &config);
    /* Enable TPM compatibility. Free running counter and synchronization compatible with TPM */
    mFtmBase[instance]->MODE &= ~(FTM_MODE_FTMEN_MASK);
    FTM_StartTimer(mFtmBase[instance], kFTM_SystemClock);
#elif gTimerMgrUseRtcFrc_c
    // do nothing, RTC has no PWM
#elif gTimerMgrUseCtimer_c
    ctimer_config_t config;
    CTIMER_GetDefaultConfig(&config);
    CTIMER_Init(mCtimerBase[gStackTimerInstance_c], &config);
    CTIMER_StartTimer(mCtimerBase[gStackTimerInstance_c]);
#else
    tpm_config_t config;
    TPM_GetDefaultConfig(&config);
    TPM_Init(mTpmBase[instance], &config);
    TPM_StartTimer(mTpmBase[instance], kTPM_SystemClock);
#endif
}

/*************************************************************************************/
void PWM_SetChnCountVal(uint8_t instance, uint8_t channel, tmrTimerTicks_t val)
{
#if gTimerMgrUseFtm_c
    mFtmBase[instance]->CONTROLS[channel].CnV = val;
#elif gTimerMgrUseRtcFrc_c
    // do nothing, RTC has no PWM
#elif gTimerMgrUseCtimer_c
    mCtimerBase[gStackTimerInstance_c]->MR[channel] = val;
#else
    mTpmBase[instance]->CONTROLS[channel].CnV = val;
#endif
}

/*************************************************************************************/
tmrTimerTicks_t PWM_GetChnCountVal(uint8_t instance, uint8_t channel)
{
    tmrTimerTicks_t value = 0;
#if gTimerMgrUseFtm_c
    value = mFtmBase[instance]->CONTROLS[channel].CnV;
#elif gTimerMgrUseRtcFrc_c
    // do nothing, RTC has no PWM
#elif gTimerMgrUseCtimer_c
    value =  mCtimerBase[gStackTimerInstance_c]->MR[channel];
#else
    value = mTpmBase[instance]->CONTROLS[channel].CnV;
#endif
    return value;
}

#if !defined(gTimerMgrUseCtimer_c) || (gTimerMgrUseCtimer_c == 0)
/* For QN9080 CTimer pwm output always starts low and rises on compare match */
/*************************************************************************************/
void PWM_StartEdgeAlignedLowTrue(uint8_t instance, tmr_adapter_pwm_param_t *param, uint8_t channel)
{
#if gTimerMgrUseFtm_c
    ftm_chnl_pwm_signal_param_t pwmChannelConfig = {
        .chnlNumber = (ftm_chnl_t)channel,
        .level = kFTM_LowTrue,
        .dutyCyclePercent = param->initValue,
        .firstEdgeDelayPercent = 0
    };
    
    FTM_SetupPwm(mFtmBase[instance], &pwmChannelConfig, 1, kFTM_EdgeAlignedPwm, param->frequency, BOARD_GetFtmClock(instance));
#else
    tpm_chnl_pwm_signal_param_t pwmChannelConfig = {
        .chnlNumber = (tpm_chnl_t)channel,
        .level = kTPM_LowTrue,
        .dutyCyclePercent = param->initValue,
#if defined(FSL_FEATURE_TPM_HAS_COMBINE) && FSL_FEATURE_TPM_HAS_COMBINE
        .firstEdgeDelayPercent = 0
#endif
    };
    
    TPM_SetupPwm(mTpmBase[instance], &pwmChannelConfig, 1, kTPM_EdgeAlignedPwm, param->frequency, BOARD_GetTpmClock(instance));
#endif
}
#endif
#endif
