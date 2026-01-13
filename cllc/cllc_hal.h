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
void CLLC_HAL_setupDevice(void);
void CLLC_HAL_disablePWMClkCounting(void);
void CLLC_HAL_enablePWMClkCounting(void);
void CLLC_HAL_setupPWM(uint16_t powerFlowDir);
void CLLC_HAL_setupCMPSSHighLowLimit(uint32_t base1,
                                 float32_t currentLimit,
                                 float32_t currentMaxSense,
                                 uint16_t hysteresis,
                                 uint16_t filterClkPrescalar,
                                 uint16_t filterSampleWindow,
                                 uint16_t filterThreshold);
void CLLC_HAL_setupHRPWMinUpDownCountModeWithDeadBand(uint32_t base1,
                                float32_t pwmFreq_Hz,
                                float32_t pwmSysClkFreq_Hz,
                                float32_t red_ns,
                                float32_t fed_ns);
void CLLC_HAL_setupHRPWMinUpDownCount2ChAsymmetricMode(uint32_t base1,
                            float32_t pwmFreq_Hz,
                            float32_t pwmSysClkFreq_Hz);
void CLLC_HAL_setupPWMinUpDownCountMode(uint32_t base1,
                            float32_t pwmFreq,
                            float32_t pwmSysClkFreq);
void CLLC_HAL_setupPWMpins(uint16_t mode);
void CLLC_HAL_setupADC(void);
void CLLC_HAL_setupProfilingGPIO(void);
void CLLC_HAL_setupSynchronousRectificationAction(uint16_t powerFlow);
void CLLC_HAL_setupSynchronousRectificationActionDebug(uint16_t powerFlow);
void CLLC_HAL_setupBoardProtection(void);
void CLLC_HAL_setupIprimSensedSignalChain(void);
void CLLC_HAL_setupTrigForADC(void);
void CLLC_HAL_setupECAPinPWMMode(uint32_t base1,
                            float32_t pwmFreq_Hz,
                            float32_t pwmSysClkFreq_Hz);
void CLLC_HAL_setupCLA(void);

//
//CLA C Tasks defined in Cla1Tasks_C.cla
//
__attribute__((interrupt))  void Cla1Task1();
__attribute__((interrupt))  void Cla1Task2();
__attribute__((interrupt))  void Cla1Task3();
__attribute__((interrupt))  void Cla1Task4();
__attribute__((interrupt))  void Cla1Task5();
__attribute__((interrupt))  void Cla1Task6();
__attribute__((interrupt))  void Cla1Task7();
__attribute__((interrupt))  void Cla1BackgroundTask();

extern uint16_t Cla1ProgLoadStart;
extern uint16_t Cla1ProgLoadEnd;
extern uint16_t Cla1ProgLoadSize;
extern uint16_t Cla1ProgRunStart;
extern uint16_t Cla1ProgRunEnd;
extern uint16_t Cla1ProgRunSize;

//
// ISR related
//
#if CLLC_ISR1_RUNNING_ON == C28x_CORE

#ifndef __TMS320C28XX_CLA__
    #pragma CODE_SECTION(CLLC_ISR1,"isrcodefuncs");
    #pragma INTERRUPT (CLLC_ISR1, HPI)
    interrupt void CLLC_ISR1(void);

    #pragma INTERRUPT (CLLC_ISR1_second, HPI)
    #pragma CODE_SECTION(CLLC_ISR1_second,"isrcodefuncs");
    interrupt void CLLC_ISR1_second(void);

    static inline void CLLC_HAL_clearISR1InterruputFlag(void);
#endif

#endif

#if ISR2_RUNNING_ON == C28x_CORE

#ifndef __TMS320C28XX_CLA__
    #pragma CODE_SECTION(CLLC_ISR2_primToSecPowerFlow,"isrcodefuncs");
    interrupt void CLLC_ISR2_primToSecPowerFlow(void);
    #pragma CODE_SECTION(CLLC_ISR2_secToPrimPowerFlow,"isrcodefuncs");
    interrupt void CLLC_ISR2_secToPrimPowerFlow(void);
    static inline void CLLC_HAL_clearISR2InterruputFlag(void);
#endif

#endif

#ifndef __TMS320C28XX_CLA__
    #pragma CODE_SECTION(CLLC_ISR3,"ramfuncs");
    interrupt void CLLC_ISR3(void);
#endif

//#if ISR2_RUNNING_ON == C28x_CORE

#ifndef __TMS320C28XX_CLA__
    interrupt void CLLC_ISR2_primToSecPowerFlow(void);
    #pragma CODE_SECTION(CLLC_ISR2_secToPrimPowerFlow,"isrcodefuncs");
    interrupt void CLLC_ISR2_secToPrimPowerFlow(void);
    static inline void CLLC_HAL_clearISR2InterruputFlag(void);
#endif

//#endif

#ifndef __TMS320C28XX_CLA__
    #pragma CODE_SECTION(CLLC_ISR3,"ramfuncs");
    interrupt void CLLC_ISR3(void);
#endif

//
// Inline functions
//
#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_readTripFlags)
static inline int16_t CLLC_HAL_readTripFlags(void)
{
    int16_t tripIndicator;
    if(XBAR_getInputFlagStatus(CLLC_IPRIM_CMPSS_XBAR_FLAG1) == 1 ||
            XBAR_getInputFlagStatus(CLLC_IPRIM_CMPSS_XBAR_FLAG2) == 1)
    {
        tripIndicator = 1;
        XBAR_clearInputFlag(CLLC_IPRIM_CMPSS_XBAR_FLAG1);
        XBAR_clearInputFlag(CLLC_IPRIM_CMPSS_XBAR_FLAG2);
    }
    else if(XBAR_getInputFlagStatus(CLLC_ISEC_CMPSS_XBAR_FLAG1) == 1 ||
            XBAR_getInputFlagStatus(CLLC_ISEC_CMPSS_XBAR_FLAG2) == 1)
    {
        tripIndicator = 2;
        XBAR_clearInputFlag(CLLC_ISEC_CMPSS_XBAR_FLAG1);
        XBAR_clearInputFlag(CLLC_ISEC_CMPSS_XBAR_FLAG2);
    }
    else if(XBAR_getInputFlagStatus(CLLC_VSEC_CMPSS_XBAR_FLAG1) == 1)
    {
        tripIndicator = 4;
        XBAR_clearInputFlag(CLLC_VSEC_CMPSS_XBAR_FLAG1);
    }
    else if(EPWM_getOneShotTripZoneFlagStatus(CLLC_PRIM_LEG1_PWM_BASE) & 0x2)
    {
        tripIndicator = 6;
        // EPWM_clearOneShotTripZoneFlag(CLLC_PRIM_LEG1_PWM_BASE, CLLC_GANFAULTn_EPWM_FLAG);
    }
    else
    {
        tripIndicator = 0;
    }

    return(tripIndicator);
}

static inline void CLLC_HAL_resetSynchronousRectifierTripAction(
        uint16_t powerFlow)
{
    if(powerFlow == CLLC_POWER_FLOW_PRIM_SEC)
    {
        EALLOW;
        HWREGH(CLLC_SEC_LEG1_PWM_BASE + EPWM_O_TZCTLDCA) = 0xFFFF;
        HWREGH(CLLC_SEC_LEG1_PWM_BASE + EPWM_O_TZCTLDCB) = 0xFFFF;
        HWREGH(CLLC_SEC_LEG2_PWM_BASE + EPWM_O_TZCTLDCA) = 0xFFFF;
        HWREGH(CLLC_SEC_LEG2_PWM_BASE + EPWM_O_TZCTLDCB) = 0xFFFF;
        EDIS;
    }
    else
    {
        EALLOW;
        HWREGH(CLLC_PRIM_LEG1_PWM_BASE + EPWM_O_TZCTLDCA) = 0xFFFF;
        HWREGH(CLLC_PRIM_LEG1_PWM_BASE + EPWM_O_TZCTLDCB) = 0xFFFF;
        HWREGH(CLLC_PRIM_LEG2_PWM_BASE + EPWM_O_TZCTLDCA) = 0xFFFF;
        HWREGH(CLLC_PRIM_LEG2_PWM_BASE + EPWM_O_TZCTLDCB) = 0xFFFF;
        EDIS;
    }

}

static inline void CLLC_HAL_setupSynchronousRectifierTripAction(
        uint16_t powerFlow)
{
    if(powerFlow == CLLC_POWER_FLOW_PRIM_SEC)
    {
        //
        // no describe the behavior in case when DCAEVT2 and
        //
        EPWM_setTripZoneAdvDigitalCompareActionA(CLLC_SEC_LEG1_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_D,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionB(CLLC_SEC_LEG1_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_U,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionA(CLLC_SEC_LEG2_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_U,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionB(CLLC_SEC_LEG2_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_D,
                                        EPWM_TZ_ADV_ACTION_LOW);
    }
    else
    {
        //
        // no describe the behavior in case when DCAEVT2 and
        //
        EPWM_setTripZoneAdvDigitalCompareActionA(CLLC_PRIM_LEG1_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_D,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionB(CLLC_PRIM_LEG1_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_U,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionA(CLLC_PRIM_LEG2_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_U,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionB(CLLC_PRIM_LEG2_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_D,
                                        EPWM_TZ_ADV_ACTION_LOW);
    }
}

#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_updatePWMDutyPeriodPhaseShift)
static inline void CLLC_HAL_updatePWMDutyPeriodPhaseShift(
                                uint32_t period_ticks,
                                uint32_t dutyAPrim_ticks,
                                uint32_t dutyBPrim_ticks,
                                uint32_t dutyASec_ticks,
                                uint32_t dutyBSec_ticks,
                                uint32_t phaseShiftPrimSec_ticks)
{
    EALLOW;
    HWREG(CLLC_PRIM_LEG1_PWM_BASE + HRPWM_O_TBPRDHR) = period_ticks;
    HWREG(CLLC_PRIM_LEG1_PWM_BASE + HRPWM_O_CMPA) = dutyAPrim_ticks;
    HWREG(CLLC_PRIM_LEG1_PWM_BASE + HRPWM_O_CMPB) = dutyBPrim_ticks;

    HWREG(CLLC_SEC_LEG1_PWM_BASE + HRPWM_O_CMPA) = dutyASec_ticks;
    HWREG(CLLC_SEC_LEG1_PWM_BASE + HRPWM_O_CMPB) = dutyBSec_ticks;

    HWREG(CLLC_SEC_LEG1_PWM_BASE + EPWM_O_TBPHS) = phaseShiftPrimSec_ticks;
    HWREG(CLLC_SEC_LEG2_PWM_BASE + EPWM_O_TBPHS) = phaseShiftPrimSec_ticks;

    EDIS;
}
#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_updatePWMDeadBandPrim)
static inline void CLLC_HAL_updatePWMDeadBandPrim(uint32_t dbRED_ticks,
                                uint32_t dbFED_ticks)
{
    EALLOW;
    HWREG(CLLC_PRIM_LEG1_PWM_BASE + HRPWM_O_DBREDHR) = dbRED_ticks;
    HWREG(CLLC_PRIM_LEG1_PWM_BASE + HRPWM_O_DBFEDHR) = dbFED_ticks;
    HWREG(CLLC_PRIM_LEG2_PWM_BASE + HRPWM_O_DBREDHR) = dbRED_ticks;
    HWREG(CLLC_PRIM_LEG2_PWM_BASE + HRPWM_O_DBFEDHR) = dbFED_ticks;
    EDIS;
}

#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_setProfilingGPIO1)
static inline void CLLC_HAL_setProfilingGPIO1(void)
{
    #pragma diag_suppress = 770
    #pragma diag_suppress = 173
    HWREG(GPIODATA_BASE + CLLC_GPIO_PROFILING1_SET_REG) =
                                              CLLC_GPIO_PROFILING1_SET;
    #pragma diag_warning = 770
    #pragma diag_warning = 173
}

#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_resetProfilingGPIO1)
static inline void CLLC_HAL_resetProfilingGPIO1(void)
{
    #pragma diag_suppress = 770
    #pragma diag_suppress = 173
    HWREG(GPIODATA_BASE + CLLC_GPIO_PROFILING1_CLEAR_REG) =
                                                  CLLC_GPIO_PROFILING1_CLEAR;
    #pragma diag_warning = 770
    #pragma diag_warning = 173
}

static inline void CLLC_HAL_setProfilingGPIO2(void)
{
    #pragma diag_suppress = 770
    #pragma diag_suppress = 173
    HWREG(GPIODATA_BASE  +  CLLC_GPIO_PROFILING2_SET_REG ) =
                                                 CLLC_GPIO_PROFILING2_SET;
    #pragma diag_warning = 770
    #pragma diag_warning = 173
}

static inline void CLLC_HAL_resetProfilingGPIO2(void)
{
    #pragma diag_suppress = 770
    #pragma diag_suppress = 173
    HWREG(GPIODATA_BASE  +  CLLC_GPIO_PROFILING2_CLEAR_REG ) =
                                                 CLLC_GPIO_PROFILING2_CLEAR;
    #pragma diag_warning = 770
    #pragma diag_warning = 173
}

static inline void CLLC_HAL_setProfilingGPIO3(void)
{
    #pragma diag_suppress = 770
    #pragma diag_suppress = 173
    HWREG(GPIODATA_BASE  +  CLLC_GPIO_PROFILING3_SET_REG ) =
                                                 CLLC_GPIO_PROFILING3_SET;
    #pragma diag_warning = 770
    #pragma diag_warning = 173
}

static inline void CLLC_HAL_resetProfilingGPIO3(void)
{
    #pragma diag_suppress = 770
    #pragma diag_suppress = 173
    HWREG(GPIODATA_BASE  +  CLLC_GPIO_PROFILING3_CLEAR_REG ) =
                                                 CLLC_GPIO_PROFILING3_CLEAR;
    #pragma diag_warning = 770
    #pragma diag_warning = 173
}

#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_clearISR1PeripheralInterruptFlag)
static inline void CLLC_HAL_clearISR1PeripheralInterruptFlag()
{
    EPWM_clearEventTriggerInterruptFlag(CLLC_ISR1_PERIPHERAL_TRIG_BASE);
}

#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_clearISR2PeripheralInterruptFlag)
static inline void CLLC_HAL_clearISR2PeripheralInterruptFlag()
{
    ECAP_clearInterrupt(CLLC_ISR2_ECAP_BASE, ECAP_ISR_SOURCE_COUNTER_PERIOD);
    ECAP_clearGlobalInterrupt(CLLC_ISR2_ECAP_BASE);
}

#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_clearISR3PeripheralInterruptFlag)
static inline void CLLC_HAL_clearISR3PeripheralInterruptFlag()
{
    ADC_clearInterruptStatus(CLLC_ISR3_PERIPHERAL_TRIG_BASE, ADC_INT_NUMBER2);
}

#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_setupISR1Trigger)
static inline void CLLC_HAL_setupISR1Trigger(float32_t freq)
{
    //
    // below sets up the ISR trigger for ISR1
    // ISR is triggered right before period value by a compare C match
    // as latency on C28x and CLA is different, for CLA a -20 is used
    // for C28x -27 is used based in GPIO toggle orbserved on the oscilloscope
    //
    #if CLLC_ISR1_RUNNING_ON == CLA_CORE
        EPWM_setCounterCompareValue(CLLC_ISR1_PERIPHERAL_TRIG_BASE,
                                       EPWM_COUNTER_COMPARE_C,
         ((TICKS_IN_PWM_FREQUENCY(freq, CLLC_PWMSYSCLOCK_FREQ_HZ) >> 1) - 20));
    #else
        EPWM_setCounterCompareValue(CLLC_ISR1_PERIPHERAL_TRIG_BASE,
                                       EPWM_COUNTER_COMPARE_C,
         ((TICKS_IN_PWM_FREQUENCY(freq, CLLC_PWMSYSCLOCK_FREQ_HZ) >> 1) - 27));
    #endif

}

#ifndef __TMS320C28XX_CLA__
#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_clearISR1InterruputFlag)
static inline void CLLC_HAL_clearISR1InterruputFlag()
{
    Interrupt_clearACKGroup(CLLC_ISR1_PIE_GROUP);
}

#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_clearISR2InterruputFlag)
static inline void CLLC_HAL_clearISR2InterruputFlag()
{
    Interrupt_clearACKGroup(CLLC_ISR2_PIE_GROUP);
}

#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_clearISR3InterruputFlag)
static inline void CLLC_HAL_clearISR3InterruputFlag()
{
    Interrupt_clearACKGroup(CLLC_ISR3_PIE_GROUP);
}

#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_setupInterrupt)
static inline void CLLC_HAL_setupInterrupt(uint16_t powerFlow)
{

    #if(CLLC_ISR1_RUNNING_ON == CLA_CORE || CLLC_ISR2_RUNNING_ON == CLA_CORE)
        CLLC_HAL_setupCLA();
    #endif


    EPWM_setInterruptSource(CLLC_ISR1_PERIPHERAL_TRIG_BASE,
                            EPWM_INT_TBCTR_U_CMPC);
    CLLC_HAL_setupISR1Trigger(CLLC_MIN_PWM_SWITCHING_FREQUENCY_HZ * 0.8);
    EPWM_setInterruptEventCount(CLLC_ISR1_PERIPHERAL_TRIG_BASE, 1);
    EPWM_clearEventTriggerInterruptFlag(CLLC_ISR1_PERIPHERAL_TRIG_BASE);
    EPWM_enableInterrupt(CLLC_ISR1_PERIPHERAL_TRIG_BASE);


    //
    // How to sync ISR to ECAP
    //
    CLLC_HAL_setupECAPinPWMMode(CLLC_ISR2_ECAP_BASE,
                                 CLLC_ISR2_FREQUENCY_HZ,
                                 CLLC_PWMSYSCLOCK_FREQ_HZ);
    ECAP_enableInterrupt(CLLC_ISR2_ECAP_BASE, ECAP_ISR_SOURCE_COUNTER_PERIOD);


    CPUTimer_enableInterrupt(CLLC_ISR3_TIMEBASE);
    CPUTimer_clearOverflowFlag(CLLC_ISR3_TIMEBASE);
    ADC_setInterruptSource(CLLC_ISR3_PERIPHERAL_TRIG_BASE,
                           ADC_INT_NUMBER2, CLLC_VSEC_ADC_SOC_NO_13);
    ADC_enableInterrupt(CLLC_ISR3_PERIPHERAL_TRIG_BASE, ADC_INT_NUMBER2);
    ADC_enableContinuousMode(CLLC_ISR3_PERIPHERAL_TRIG_BASE, ADC_INT_NUMBER2);
    ADC_clearInterruptStatus(CLLC_ISR3_PERIPHERAL_TRIG_BASE, ADC_INT_NUMBER2);

    //
    //Note the ISR1 is enabled in the PIE, though the peripheral interrupt is
    //not triggered until later
    //
    #if CLLC_ISR1_RUNNING_ON == C28x_CORE
        Interrupt_register(CLLC_ISR1_TRIG, &CLLC_ISR1);
        CLLC_HAL_clearISR1InterruputFlag();
        Interrupt_enable(CLLC_ISR1_TRIG);
    #endif

        //
        //Still needed by TTPLPFC ISR2 code, do not disable when CLLLC is running on CLA
        //
        if(powerFlow == CLLC_POWER_FLOW_SEC_PRIM)
        {
            Interrupt_register(CLLC_ISR2_TRIG, &CLLC_ISR2_secToPrimPowerFlow);
        }
        else
        {
            Interrupt_register(CLLC_ISR2_TRIG, &CLLC_ISR2_primToSecPowerFlow);
        }
        CLLC_HAL_clearISR2InterruputFlag();
        Interrupt_enable(CLLC_ISR2_TRIG);


    Interrupt_register(CLLC_ISR3_TRIG, &CLLC_ISR3);
    Interrupt_enable(CLLC_ISR3_TRIG);

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

#pragma FUNC_ALWAYS_INLINE(CLLC_HAL_clearPWMTripFlags)
static inline void CLLC_HAL_clearPWMTripFlags(uint32_t base)
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

static inline void CLLC_HAL_clearPWMOneShotTripFlag(uint32_t base)
{
    //
    // clear all the configured trip sources for the PWM module
    //
    EPWM_clearTripZoneFlag(base, EPWM_TZ_INTERRUPT_OST);
}

static inline void CLLC_HAL_forcePWMOneShotTrip(uint32_t base)
{
    EPWM_forceTripZoneEvent(base, EPWM_TZ_FORCE_EVENT_OST);
}
#ifdef __cplusplus
}
#endif                                  /* extern "C" */
#endif
