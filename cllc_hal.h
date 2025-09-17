//#############################################################################
//
// FILE:   hal.h
//
// TITLE: solution hardware abstraction layer header file
//        This file consists of common variables and functions
//        for a particular hardware board, like functions to read current
//        and voltage signals on the board and functions to setup the
//        basic peripherals of the board
//
//#############################################################################

#ifndef HAL_H
#define HAL_H

#ifdef __cplusplus

extern "C" {
#endif


//
// the includes
//
#include <stdint.h>
#include "inc/hw_types.h"
#include "driverlib.h"
#include "device.h"
#include "cllc_settings.h"
//
// the function prototypes
//
void HAL_setupDevice(void);
void HAL_disablePWMClkCounting(void);
void HAL_enablePWMClkCounting(void);
void HAL_setupPWM(uint16_t powerFlowDir);
void HAL_setupCMPSSHighLowLimit(uint32_t base1,
                                 float32_t currentLimit,
                                 float32_t currentMaxSense,
                                 uint16_t hysteresis,
                                 uint16_t filterClkPrescalar,
                                 uint16_t filterSampleWindow,
                                 uint16_t filterThreshold);
void HAL_setupHRPWMinUpDownCountModeWithDeadBand(uint32_t base1,
                                float32_t pwmFreq_Hz,
                                float32_t pwmSysClkFreq_Hz,
                                float32_t red_ns,
                                float32_t fed_ns);
void HAL_setupHRPWMinUpDownCount2ChAsymmetricMode(uint32_t base1,
                            float32_t pwmFreq_Hz,
                            float32_t pwmSysClkFreq_Hz);
void HAL_setupPWMinUpDownCountMode(uint32_t base1,
                            float32_t pwmFreq,
                            float32_t pwmSysClkFreq);
void HAL_setupPWMpins(uint16_t mode);
void HAL_setupADC(void);
void HAL_setupProfilingGPIO(void);
void HAL_setupSynchronousRectificationAction(uint16_t powerFlow);
void HAL_setupSynchronousRectificationActionDebug(uint16_t powerFlow);
void HAL_setupBoardProtection(void);
void HAL_setupIprimSensedSignalChain(void);
void HAL_setupTrigForADC(void);
void HAL_setupECAPinPWMMode(uint32_t base1,
                            float32_t pwmFreq_Hz,
                            float32_t pwmSysClkFreq_Hz);
void HAL_setupCLA(void);
//
// ISR related
//
#if ISR1_RUNNING_ON == C28x_CORE

#ifndef __TMS320C28XX_CLA__
    #pragma CODE_SECTION(ISR1,"isrcodefuncs");
    #pragma INTERRUPT (ISR1, HPI)
    interrupt void ISR1(void);

    #pragma INTERRUPT (ISR1_second, HPI)
    #pragma CODE_SECTION(ISR1_second,"isrcodefuncs");
    interrupt void ISR1_second(void);

    static inline void HAL_clearISR1InterruputFlag(void);
#endif

#endif

#if ISR2_RUNNING_ON == C28x_CORE

#ifndef __TMS320C28XX_CLA__
    #pragma CODE_SECTION(ISR2_primToSecPowerFlow,"isrcodefuncs");
    interrupt void ISR2_primToSecPowerFlow(void);
    #pragma CODE_SECTION(ISR2_secToPrimPowerFlow,"isrcodefuncs");
    interrupt void ISR2_secToPrimPowerFlow(void);
    static inline void HAL_clearISR2InterruputFlag(void);
#endif

#endif

#ifndef __TMS320C28XX_CLA__
    #pragma CODE_SECTION(ISR3,"ramfuncs");
    interrupt void ISR3(void);
#endif

//
// Inline functions
//
static inline int16_t HAL_readTripFlags(void)
{
    int16_t tripIndicator;

    if(XBAR_getInputFlagStatus(IPRIM_SHUNT_CMPSS_XBAR_FLAG1) == 1 ||
       XBAR_getInputFlagStatus(IPRIM_SHUNT_CMPSS_XBAR_FLAG2) == 1)
    {
        tripIndicator = 5;
        XBAR_clearInputFlag(IPRIM_SHUNT_CMPSS_XBAR_FLAG1);
        XBAR_clearInputFlag(IPRIM_SHUNT_CMPSS_XBAR_FLAG2);
    }
    else if(XBAR_getInputFlagStatus(IPRIM_CMPSS_XBAR_FLAG1) == 1 ||
            XBAR_getInputFlagStatus(IPRIM_CMPSS_XBAR_FLAG2) == 1)
    {
        tripIndicator = 1;
        XBAR_clearInputFlag(IPRIM_CMPSS_XBAR_FLAG1);
        XBAR_clearInputFlag(IPRIM_CMPSS_XBAR_FLAG2);
    }
    else if(XBAR_getInputFlagStatus(ISEC_CMPSS_XBAR_FLAG1) == 1 ||
            XBAR_getInputFlagStatus(ISEC_CMPSS_XBAR_FLAG2) == 1)
    {
        tripIndicator = 2;
        XBAR_clearInputFlag(ISEC_CMPSS_XBAR_FLAG1);
        XBAR_clearInputFlag(ISEC_CMPSS_XBAR_FLAG2);
    }
    else
    {
        tripIndicator = 0;
    }

    return(tripIndicator);
}

static inline void HAL_resetSynchronousRectifierTripAction(
        uint16_t powerFlow)
{
    if(powerFlow == POWER_FLOW_PRIM_SEC)
    {
        EALLOW;
        HWREGH(SEC_LEG1_PWM_BASE + EPWM_O_TZCTLDCA) = 0xFFFF;
        HWREGH(SEC_LEG1_PWM_BASE + EPWM_O_TZCTLDCB) = 0xFFFF;
        HWREGH(SEC_LEG2_PWM_BASE + EPWM_O_TZCTLDCA) = 0xFFFF;
        HWREGH(SEC_LEG2_PWM_BASE + EPWM_O_TZCTLDCB) = 0xFFFF;
        EDIS;
    }
    else
    {
        EALLOW;
        HWREGH(PRIM_LEG1_PWM_BASE + EPWM_O_TZCTLDCA) = 0xFFFF;
        HWREGH(PRIM_LEG1_PWM_BASE + EPWM_O_TZCTLDCB) = 0xFFFF;
        HWREGH(PRIM_LEG2_PWM_BASE + EPWM_O_TZCTLDCA) = 0xFFFF;
        HWREGH(PRIM_LEG2_PWM_BASE + EPWM_O_TZCTLDCB) = 0xFFFF;
        EDIS;
    }

}

static inline void  HAL_setupSynchronousRectifierTripAction(
        uint16_t powerFlow)
{
    if(powerFlow == POWER_FLOW_PRIM_SEC)
    {
        //
        // no describe the behavior in case when DCAEVT2 and
        //
        EPWM_setTripZoneAdvDigitalCompareActionA(SEC_LEG1_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_D,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionB(SEC_LEG1_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_U,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionA(SEC_LEG2_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_U,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionB(SEC_LEG2_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_D,
                                        EPWM_TZ_ADV_ACTION_LOW);
    }
    else
    {
        //
        // no describe the behavior in case when DCAEVT2 and
        //
        EPWM_setTripZoneAdvDigitalCompareActionA(PRIM_LEG1_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_D,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionB(PRIM_LEG1_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_U,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionA(PRIM_LEG2_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_U,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionB(PRIM_LEG2_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_D,
                                        EPWM_TZ_ADV_ACTION_LOW);
    }
}

#pragma FUNC_ALWAYS_INLINE(HAL_updatePWMDutyPeriodPhaseShift)
static inline void HAL_updatePWMDutyPeriodPhaseShift(
                                uint32_t period_ticks,
                                uint32_t dutyAPrim_ticks,
                                uint32_t dutyBPrim_ticks,
                                uint32_t dutyASec_ticks,
                                uint32_t dutyBSec_ticks,
                                uint32_t phaseShiftPrimSec_ticks)
{
    EALLOW;
    HWREG(PRIM_LEG1_PWM_BASE + HRPWM_O_TBPRDHR) = period_ticks;
    HWREG(PRIM_LEG1_PWM_BASE + HRPWM_O_CMPA) = dutyAPrim_ticks;
    HWREG(PRIM_LEG1_PWM_BASE + HRPWM_O_CMPB) = dutyBPrim_ticks;

    HWREG(SEC_LEG1_PWM_BASE + HRPWM_O_CMPA) = dutyASec_ticks;
    HWREG(SEC_LEG1_PWM_BASE + HRPWM_O_CMPB) = dutyBSec_ticks;

    HWREG(SEC_LEG1_PWM_BASE + EPWM_O_TBPHS) = phaseShiftPrimSec_ticks;
    HWREG(SEC_LEG2_PWM_BASE + EPWM_O_TBPHS) = phaseShiftPrimSec_ticks;

    EDIS;
}
#pragma FUNC_ALWAYS_INLINE(HAL_updatePWMDeadBandPrim)
static inline void HAL_updatePWMDeadBandPrim(uint32_t dbRED_ticks,
                                uint32_t dbFED_ticks)
{
    EALLOW;
    HWREG(PRIM_LEG1_PWM_BASE + HRPWM_O_DBREDHR) = dbRED_ticks;
    HWREG(PRIM_LEG1_PWM_BASE + HRPWM_O_DBFEDHR) = dbFED_ticks;
    HWREG(PRIM_LEG2_PWM_BASE + HRPWM_O_DBREDHR) = dbRED_ticks;
    HWREG(PRIM_LEG2_PWM_BASE + HRPWM_O_DBFEDHR) = dbFED_ticks;
    EDIS;
}

#pragma FUNC_ALWAYS_INLINE(HAL_setProfilingGPIO1)
static inline void HAL_setProfilingGPIO1(void)
{
    #pragma diag_suppress = 770
    #pragma diag_suppress = 173
    HWREG(GPIODATA_BASE + GPIO_PROFILING1_SET_REG) =
                                              GPIO_PROFILING1_SET;
    #pragma diag_warning = 770
    #pragma diag_warning = 173
}

#pragma FUNC_ALWAYS_INLINE(HAL_resetProfilingGPIO1)
static inline void HAL_resetProfilingGPIO1(void)
{
    #pragma diag_suppress = 770
    #pragma diag_suppress = 173
    HWREG(GPIODATA_BASE + GPIO_PROFILING1_CLEAR_REG) =
                                                  GPIO_PROFILING1_CLEAR;
    #pragma diag_warning = 770
    #pragma diag_warning = 173
}

static inline void HAL_setProfilingGPIO2(void)
{
    #pragma diag_suppress = 770
    #pragma diag_suppress = 173
    HWREG(GPIODATA_BASE  +  GPIO_PROFILING2_SET_REG ) =
                                                 GPIO_PROFILING2_SET;
    #pragma diag_warning = 770
    #pragma diag_warning = 173
}

static inline void HAL_resetProfilingGPIO2(void)
{
    #pragma diag_suppress = 770
    #pragma diag_suppress = 173
    HWREG(GPIODATA_BASE  +  GPIO_PROFILING2_CLEAR_REG ) =
                                                 GPIO_PROFILING2_CLEAR;
    #pragma diag_warning = 770
    #pragma diag_warning = 173
}

static inline void HAL_setProfilingGPIO3(void)
{
    #pragma diag_suppress = 770
    #pragma diag_suppress = 173
    HWREG(GPIODATA_BASE  +  GPIO_PROFILING3_SET_REG ) =
                                                 GPIO_PROFILING3_SET;
    #pragma diag_warning = 770
    #pragma diag_warning = 173
}

static inline void HAL_resetProfilingGPIO3(void)
{
    #pragma diag_suppress = 770
    #pragma diag_suppress = 173
    HWREG(GPIODATA_BASE  +  GPIO_PROFILING3_CLEAR_REG ) =
                                                 GPIO_PROFILING3_CLEAR;
    #pragma diag_warning = 770
    #pragma diag_warning = 173
}

#pragma FUNC_ALWAYS_INLINE(HAL_clearISR1PeripheralInterruptFlag)
static inline void HAL_clearISR1PeripheralInterruptFlag()
{
    EPWM_clearEventTriggerInterruptFlag(ISR1_PERIPHERAL_TRIG_BASE);
}

#pragma FUNC_ALWAYS_INLINE(HAL_clearISR2PeripheralInterruptFlag)
static inline void HAL_clearISR2PeripheralInterruptFlag()
{
    ECAP_clearInterrupt(ISR2_ECAP_BASE, ECAP_ISR_SOURCE_COUNTER_PERIOD);
    ECAP_clearGlobalInterrupt(ISR2_ECAP_BASE);
}

#pragma FUNC_ALWAYS_INLINE(HAL_clearISR3PeripheralInterruptFlag)
static inline void HAL_clearISR3PeripheralInterruptFlag()
{
    ADC_clearInterruptStatus(ISR3_PERIPHERAL_TRIG_BASE, ADC_INT_NUMBER2);
}

#pragma FUNC_ALWAYS_INLINE(HAL_setupISR1Trigger)
static inline void HAL_setupISR1Trigger(float32_t freq)
{
    //
    // below sets up the ISR trigger for ISR1
    // ISR is triggered right before period value by a compare C match
    // as latency on C28x and CLA is different, for CLA a -20 is used
    // for C28x -27 is used based in GPIO toggle orbserved on the oscilloscope
    //
    #if ISR1_RUNNING_ON == CLA_CORE
        EPWM_setCounterCompareValue(ISR1_PERIPHERAL_TRIG_BASE,
                                       EPWM_COUNTER_COMPARE_C,
         ((TICKS_IN_PWM_FREQUENCY(freq, PWMSYSCLOCK_FREQ_HZ) >> 1) - 20));
    #else
        EPWM_setCounterCompareValue(ISR1_PERIPHERAL_TRIG_BASE,
                                       EPWM_COUNTER_COMPARE_C,
         ((TICKS_IN_PWM_FREQUENCY(freq, PWMSYSCLOCK_FREQ_HZ) >> 1) - 27));
    #endif

}

#ifndef __TMS320C28XX_CLA__
static inline void HAL_clearISR1InterruputFlag()
{
    Interrupt_clearACKGroup(ISR1_PIE_GROUP);
}

static inline void HAL_clearISR2InterruputFlag()
{
    Interrupt_clearACKGroup(ISR2_PIE_GROUP);
}

static inline void HAL_clearISR3InterruputFlag()
{
    Interrupt_clearACKGroup(ISR3_PIE_GROUP);
}

static inline void HAL_setupInterrupt(uint16_t powerFlow)
{

    #if(ISR1_RUNNING_ON == CLA_CORE || ISR2_RUNNING_ON == CLA_CORE)
        HAL_setupCLA();
    #endif


    EPWM_setInterruptSource(ISR1_PERIPHERAL_TRIG_BASE,
                            EPWM_INT_TBCTR_U_CMPC);
    HAL_setupISR1Trigger(MIN_PWM_SWITCHING_FREQUENCY_HZ * 0.8);
    EPWM_setInterruptEventCount(ISR1_PERIPHERAL_TRIG_BASE, 1);
    EPWM_clearEventTriggerInterruptFlag(ISR1_PERIPHERAL_TRIG_BASE);
    EPWM_enableInterrupt(ISR1_PERIPHERAL_TRIG_BASE);



    ECAP_enableInterrupt(ISR2_ECAP_BASE, ECAP_ISR_SOURCE_COUNTER_PERIOD);


    CPUTimer_enableInterrupt(ISR3_TIMEBASE);
    CPUTimer_clearOverflowFlag(ISR3_TIMEBASE);
    ADC_setInterruptSource(ISR3_PERIPHERAL_TRIG_BASE,
                           ADC_INT_NUMBER2, VPRIM_ADC_SOC_NO_13);
    ADC_enableInterrupt(ISR3_PERIPHERAL_TRIG_BASE, ADC_INT_NUMBER2);
    ADC_enableContinuousMode(ISR3_PERIPHERAL_TRIG_BASE, ADC_INT_NUMBER2);
    ADC_clearInterruptStatus(ISR3_PERIPHERAL_TRIG_BASE, ADC_INT_NUMBER2);

    //
    //Note the ISR1 is enabled in the PIE, though the peripheral interrupt is
    //not triggered until later
    //
    #if ISR1_RUNNING_ON == C28x_CORE
        Interrupt_register(ISR1_TRIG, &ISR1);
        HAL_clearISR1InterruputFlag();
        Interrupt_enable(ISR1_TRIG);
    #endif

    #if ISR2_RUNNING_ON == C28x_CORE
        if(powerFlow == POWER_FLOW_SEC_PRIM)
        {
            Interrupt_register(ISR2_TRIG, &ISR2_secToPrimPowerFlow);
        }
        else
        {
            Interrupt_register(ISR2_TRIG, &ISR2_primToSecPowerFlow);
        }
        HAL_clearISR2InterruputFlag();
        Interrupt_enable(ISR2_TRIG);
    #endif

    Interrupt_register(ISR3_TRIG, &ISR3);
    Interrupt_enable(ISR3_TRIG);

    EALLOW;
    //
    // Enable Global interrupt INTM
    //
    EINT;
    //
    // Enable Global real-time interrupt DBGM
    //
    ERTM;
    EDIS;
}
#endif

#pragma FUNC_ALWAYS_INLINE(HAL_clearPWMTripFlags)
static inline void HAL_clearPWMTripFlags(uint32_t base)
{
    //
    // clear all the configured trip sources for the PWM module
    //
    EPWM_clearTripZoneFlag(base,
                           ( EPWM_TZ_INTERRUPT_OST |
                             EPWM_TZ_INTERRUPT_CBC |
                             EPWM_TZ_INTERRUPT_DCAEVT1 )
                           );
}

static inline void HAL_clearPWMOneShotTripFlag(uint32_t base)
{
    //
    // clear all the configured trip sources for the PWM module
    //
    EPWM_clearTripZoneFlag(base, EPWM_TZ_INTERRUPT_OST);
}

static inline void HAL_forcePWMOneShotTrip(uint32_t base)
{
    EPWM_forceTripZoneEvent(base, EPWM_TZ_FORCE_EVENT_OST);
}
#ifdef __cplusplus
}
#endif                                  /* extern "C" */
#endif
