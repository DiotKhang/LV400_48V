//#############################################################################
//
// FILE:   hal.c
//
// TITLE: solution hardware abstraction layer,
//        This file consists of common variables and functions
//        for a particular hardware board, like functions to read current
//        and voltage signals on the board and functions to setup the
//        basic peripherals of the board
//
//#############################################################################

#include "hal.h"

volatile uint32_t ePWM[9] = {0,
                             EPWM1_BASE,
                             EPWM2_BASE,
                             EPWM3_BASE,
                             EPWM4_BASE,
                             EPWM5_BASE,
                             EPWM6_BASE,
                             EPWM7_BASE,
                             EPWM8_BASE};

//
//  This routine sets up the basic device configuration such as initializing PLL
//  CPU timers and copying code from FLASH to RAM
// 

void HAL_setupDevice(void)
{

    //
    // Initialize device clock and peripherals
    //
    Device_init();

    //
    // Disable pin locks and enable internal pull-ups.
    //
    Device_initGPIO();

    //
    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    //
    // initialize CPU timers
    // Initialize timer period to rate at which they will be used
    // to slice of task
    // TASK A FREQUENCY
    //
    CPUTimer_setPeriod(TASKA_CPUTIMER_BASE,
                       DEVICE_SYSCLK_FREQ / TASKA_FREQ_HZ);

    //
    // TASK B FREQUENCY
    //
    CPUTimer_setPeriod(TASKB_CPUTIMER_BASE,
                       DEVICE_SYSCLK_FREQ / TASKB_FREQ_HZ);

    //
    // TASK C FREQUENCY
    //
    CPUTimer_setPeriod(TASKC_CPUTIMER_BASE,
                       DEVICE_SYSCLK_FREQ / TASKC_FREQ_HZ);

    //
    // Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
    //
    CPUTimer_setPreScaler(TASKA_CPUTIMER_BASE, 0);

    //
    // Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
    //
    CPUTimer_setPreScaler(TASKB_CPUTIMER_BASE, 0);

    //
    // Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
    //
    CPUTimer_setPreScaler(TASKC_CPUTIMER_BASE, 0);

    //
    // Make sure timer is stopped
    //
    CPUTimer_stopTimer(TASKA_CPUTIMER_BASE);
    CPUTimer_stopTimer(TASKB_CPUTIMER_BASE);
    CPUTimer_stopTimer(TASKC_CPUTIMER_BASE);

    CPUTimer_setEmulationMode(TASKA_CPUTIMER_BASE,
                              CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);
    CPUTimer_setEmulationMode(TASKB_CPUTIMER_BASE,
                              CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);
    CPUTimer_setEmulationMode(TASKC_CPUTIMER_BASE,
                              CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);

    CPUTimer_startTimer(TASKA_CPUTIMER_BASE);
    CPUTimer_startTimer(TASKB_CPUTIMER_BASE);
    CPUTimer_startTimer(TASKC_CPUTIMER_BASE);

}

void HAL_disablePWMClkCounting(void)
{
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
}

void HAL_enablePWMClkCounting(void)
{
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
}

void HAL_setupPWMinUpDownCountMode(uint32_t base1,
                            float32_t pwmFreq_Hz,
                            float32_t pwmSysClkFreq_Hz)
{

    uint32_t pwmPeriod_ticks;

    pwmPeriod_ticks = TICKS_IN_PWM_FREQUENCY(pwmFreq_Hz, pwmSysClkFreq_Hz);

    //
    // Time Base SubModule Registers
    //
    EPWM_setPeriodLoadMode(base1, EPWM_PERIOD_DIRECT_LOAD);
    EPWM_setTimeBasePeriod(base1, pwmPeriod_ticks >> 1);
    EPWM_setTimeBaseCounter(base1, 0);
    EPWM_setPhaseShift(base1, 0);
    EPWM_setTimeBaseCounterMode(base1, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_setClockPrescaler(base1, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);

    //
    // configure PWM 1 as master and Phase 2 as slaves and
    // let it pass the sync in pulse from PWM1
    //
    EPWM_disablePhaseShiftLoad(base1);
    EPWM_setSyncOutPulseMode(base1, EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO);

    //
    // Counter Compare Submodule Registers
    // set duty 0% initially
    //
    EPWM_setCounterCompareValue(base1, EPWM_COUNTER_COMPARE_A, 0);
    EPWM_setCounterCompareShadowLoadMode(base1, EPWM_COUNTER_COMPARE_A,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO);

    //
    // Action Qualifier SubModule Registers
    //
    HWREGH(base1 + EPWM_O_AQCTLA) = 0 ;
    EPWM_setActionQualifierAction(base1, EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(base1, EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

}

void HAL_setupPWM(uint16_t powerFlowDir)
{
    if(powerFlowDir == POWER_FLOW_PRIM_SEC)
    {
        //
        //setup the prim PWM
        //
        HAL_setupEPWMinUpDownCountModeWithDeadBand(
                                   PRIM_LEG1_PWM_BASE,
                                   NOMINAL_PWM_SWITCHING_FREQUENCY_HZ,
                                   PWMSYSCLOCK_FREQ_HZ,
                                   PRIM_PWM_DEADBAND_RED_NS,
                                   PRIM_PWM_DEADBAND_FED_NS);
        HAL_setupEPWMinUpDownCountModeWithDeadBand(
                                   PRIM_LEG2_PWM_BASE,
                                   NOMINAL_PWM_SWITCHING_FREQUENCY_HZ,
                                   PWMSYSCLOCK_FREQ_HZ,
                                   PRIM_PWM_DEADBAND_RED_NS,
                                   PRIM_PWM_DEADBAND_FED_NS);

        //
        //setup the sec PWM
        //
        HAL_setupEPWMinUpDownCount2ChAsymmetricMode(
                                  SEC_LEG1_PWM_BASE,
                                  NOMINAL_PWM_SWITCHING_FREQUENCY_HZ,
                                  PWMSYSCLOCK_FREQ_HZ);
        HAL_setupEPWMinUpDownCount2ChAsymmetricMode(
                                  SEC_LEG2_PWM_BASE,
                                  NOMINAL_PWM_SWITCHING_FREQUENCY_HZ,
                                  PWMSYSCLOCK_FREQ_HZ);

        //
        //setup phase shift behavior
        //
        EPWM_disablePhaseShiftLoad(PRIM_LEG1_PWM_BASE);
        EPWM_disablePhaseShiftLoad(PRIM_LEG2_PWM_BASE);
        EPWM_setSyncOutPulseMode(PRIM_LEG1_PWM_BASE,
                                 EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO);

        EPWM_setSyncOutPulseMode(PRIM_LEG2_PWM_BASE,
                                 EPWM_SYNC_OUT_PULSE_ON_SOFTWARE);
        EPWM_enablePhaseShiftLoad(SEC_LEG1_PWM_BASE);

        //
        // pass syncin
        //
        EPWM_setSyncOutPulseMode(SEC_LEG1_PWM_BASE,
                                 EPWM_SYNC_OUT_PULSE_ON_SOFTWARE);
        EPWM_setPhaseShift(SEC_LEG1_PWM_BASE, 2);
        EPWM_setCountModeAfterSync(SEC_LEG1_PWM_BASE,
                                   EPWM_COUNT_MODE_UP_AFTER_SYNC);

        //
        // by default EPWM3 syncout is passed to EPWM4
        // so no configuration is needed here
        //

        SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM4 ,
                             SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);

        EPWM_enablePhaseShiftLoad(SEC_LEG2_PWM_BASE);
        //
        // used by blanking logic
        //
        EPWM_setSyncOutPulseMode(SEC_LEG2_PWM_BASE,
                                 EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO );
        EPWM_setPhaseShift(SEC_LEG2_PWM_BASE, 2 );
        EPWM_setCountModeAfterSync(SEC_LEG2_PWM_BASE,
                                   EPWM_COUNT_MODE_UP_AFTER_SYNC);

        //
        //Enable swap deadband
        //
        HWREGH(PRIM_LEG2_PWM_BASE + EPWM_O_DBCTL) =
                (HWREGH(PRIM_LEG2_PWM_BASE + EPWM_O_DBCTL) | 0x3000);


        HWREGH(SEC_LEG1_PWM_BASE + EPWM_O_DBCTL) =
                (HWREGH(SEC_LEG1_PWM_BASE + EPWM_O_DBCTL) | 0x3000);

        EPWM_setupEPWMLinks(PRIM_LEG2_PWM_BASE,
                          EPWM_LINK_WITH_EPWM_1,
                          EPWM_LINK_TBPRD);

        EPWM_setupEPWMLinks(SEC_LEG1_PWM_BASE,
                          EPWM_LINK_WITH_EPWM_1,
                          EPWM_LINK_TBPRD);

        EPWM_setupEPWMLinks(SEC_LEG2_PWM_BASE,
                          EPWM_LINK_WITH_EPWM_1,
                          EPWM_LINK_TBPRD);

        EPWM_setupEPWMLinks(PRIM_LEG2_PWM_BASE,
                          EPWM_LINK_WITH_EPWM_1,
                          EPWM_LINK_COMP_A);

        EPWM_setupEPWMLinks(PRIM_LEG2_PWM_BASE,
                          EPWM_LINK_WITH_EPWM_1,
                          EPWM_LINK_COMP_B);

        EPWM_setupEPWMLinks(SEC_LEG2_PWM_BASE,
                          EPWM_LINK_WITH_EPWM_3,
                          EPWM_LINK_COMP_A);

        EPWM_setupEPWMLinks(SEC_LEG2_PWM_BASE,
                          EPWM_LINK_WITH_EPWM_3,
                          EPWM_LINK_COMP_B);
    }
    else
    {
        //
        //setup the prim PWM
        //
        HAL_setupEPWMinUpDownCount2ChAsymmetricMode(
                                   PRIM_LEG1_PWM_BASE,
                                   NOMINAL_PWM_SWITCHING_FREQUENCY_HZ,
                                   PWMSYSCLOCK_FREQ_HZ);
        HAL_setupEPWMinUpDownCount2ChAsymmetricMode(
                                   PRIM_LEG2_PWM_BASE,
                                   NOMINAL_PWM_SWITCHING_FREQUENCY_HZ,
                                   PWMSYSCLOCK_FREQ_HZ);


        //
        //setup the sec PWM
        //
        HAL_setupEPWMinUpDownCountModeWithDeadBand(
                                   SEC_LEG1_PWM_BASE,
                                   NOMINAL_PWM_SWITCHING_FREQUENCY_HZ,
                                   PWMSYSCLOCK_FREQ_HZ,
                                   PRIM_PWM_DEADBAND_RED_NS,
                                   PRIM_PWM_DEADBAND_FED_NS);
        HAL_setupEPWMinUpDownCountModeWithDeadBand(
                                   SEC_LEG2_PWM_BASE,
                                   NOMINAL_PWM_SWITCHING_FREQUENCY_HZ,
                                   PWMSYSCLOCK_FREQ_HZ,
                                   PRIM_PWM_DEADBAND_RED_NS,
                                   PRIM_PWM_DEADBAND_FED_NS);

        //
        //setup phase shift behavior
        //
        EPWM_disablePhaseShiftLoad(PRIM_LEG1_PWM_BASE);
        EPWM_disablePhaseShiftLoad(PRIM_LEG2_PWM_BASE);
        EPWM_setSyncOutPulseMode(PRIM_LEG1_PWM_BASE,
                               EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO);

        EPWM_setSyncOutPulseMode(PRIM_LEG2_PWM_BASE,
                               EPWM_SYNC_OUT_PULSE_ON_SOFTWARE);
        EPWM_enablePhaseShiftLoad(SEC_LEG1_PWM_BASE);

        //
        // This basically is also SYNCIN pass, need to correct driver lib
        // comments
        //
        EPWM_setSyncOutPulseMode(SEC_LEG1_PWM_BASE,
                               EPWM_SYNC_OUT_PULSE_ON_SOFTWARE);
        EPWM_setPhaseShift(SEC_LEG1_PWM_BASE, 2);
        EPWM_setCountModeAfterSync(SEC_LEG1_PWM_BASE,
                                 EPWM_COUNT_MODE_UP_AFTER_SYNC);

        //
        // by default EPWM1 syncout is passed to EPWM4
        //

        SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM4 ,
                           SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);

        EPWM_enablePhaseShiftLoad(SEC_LEG2_PWM_BASE);

        EPWM_setSyncOutPulseMode(PRIM_LEG2_PWM_BASE,
                               EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO );
        EPWM_setPhaseShift(SEC_LEG2_PWM_BASE, 2 );
        EPWM_setCountModeAfterSync(SEC_LEG2_PWM_BASE,
                                 EPWM_COUNT_MODE_UP_AFTER_SYNC);

        //
        // Enable swap deadband
        //
        HWREGH(PRIM_LEG2_PWM_BASE + EPWM_O_DBCTL) =
              (HWREGH(PRIM_LEG2_PWM_BASE + EPWM_O_DBCTL) | 0x3000);


        HWREGH(SEC_LEG2_PWM_BASE + EPWM_O_DBCTL) =
              (HWREGH(SEC_LEG1_PWM_BASE + EPWM_O_DBCTL) | 0x3000);

        EPWM_setupEPWMLinks(PRIM_LEG2_PWM_BASE,
                        EPWM_LINK_WITH_EPWM_1,
                        EPWM_LINK_TBPRD);

        EPWM_setupEPWMLinks(SEC_LEG1_PWM_BASE,
                        EPWM_LINK_WITH_EPWM_1,
                        EPWM_LINK_TBPRD);

        EPWM_setupEPWMLinks(SEC_LEG2_PWM_BASE,
                        EPWM_LINK_WITH_EPWM_1,
                        EPWM_LINK_TBPRD);

        EPWM_setupEPWMLinks(PRIM_LEG2_PWM_BASE,
                        EPWM_LINK_WITH_EPWM_1,
                        EPWM_LINK_COMP_A);

        EPWM_setupEPWMLinks(PRIM_LEG2_PWM_BASE,
                        EPWM_LINK_WITH_EPWM_1,
                        EPWM_LINK_COMP_B);

        EPWM_setupEPWMLinks(SEC_LEG2_PWM_BASE,
                        EPWM_LINK_WITH_EPWM_3,
                        EPWM_LINK_COMP_A);

        EPWM_setupEPWMLinks(SEC_LEG2_PWM_BASE,
                        EPWM_LINK_WITH_EPWM_3,
                        EPWM_LINK_COMP_B);
    }

    EPWM_setGlobalLoadOneShotLatch(PRIM_LEG1_PWM_BASE);
}

void HAL_setupECAPinPWMMode(uint32_t base1,
                            float32_t pwmFreq_Hz,
                            float32_t pwmSysClkFreq_Hz)
{
    int32_t ecap_ticks = (int32_t) ((float32_t)(pwmSysClkFreq_Hz / pwmFreq_Hz));

    ECAP_enableAPWMMode(base1);
    ECAP_setAPWMPeriod(base1, ecap_ticks);
    ECAP_setAPWMCompare(base1, ecap_ticks >> 1);

    ECAP_setAPWMShadowPeriod(base1, ecap_ticks);
    ECAP_setAPWMShadowCompare(base1, ecap_ticks >> 1);

    ECAP_clearInterrupt(base1, 0xFF);
    ECAP_clearGlobalInterrupt(base1);

    ECAP_startCounter(base1);

}

void HAL_setupEPWMinUpDownCountModeWithDeadBand(uint32_t base1,
                                float32_t pwmFreq_Hz,
                                float32_t pwmSysClkFreq_Hz,
                                float32_t red_ns,
                                float32_t fed_ns)
{
    uint32_t pwmPeriod_ticks;
    uint32_t dbFED_ticks, dbRED_ticks;

    pwmPeriod_ticks = (uint32_t)(pwmSysClkFreq_Hz / (float32_t)pwmFreq_Hz) >> 1;
    dbRED_ticks = ((uint32_t)(red_ns * ((float32_t)ONE_NANO_SEC) * pwmSysClkFreq_Hz * 2.0f));
    dbFED_ticks = ((uint32_t)(fed_ns * ((float32_t)ONE_NANO_SEC) * pwmSysClkFreq_Hz * 2.0f));

    //
    // Time Base SubModule Registers
    //
    EPWM_setPeriodLoadMode(base1, EPWM_PERIOD_SHADOW_LOAD);
    EPWM_setTimeBasePeriod(base1, pwmPeriod_ticks);
    EPWM_setTimeBaseCounter(base1, 0);
    EPWM_disablePhaseShiftLoad(base1);	
    EPWM_setPhaseShift(base1, 0);
    EPWM_setTimeBaseCounterMode(base1, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_setClockPrescaler(base1, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);

    // //
    // // Counter Compare Submodule Registers
    // // set duty 50% initially
    // //
    EPWM_setCounterCompareValue(base1, EPWM_COUNTER_COMPARE_A, pwmPeriod_ticks >> 1);	        
    EPWM_setCounterCompareValue(base1, EPWM_COUNTER_COMPARE_B, pwmPeriod_ticks >> 1);
    //
    // set as shadow mode
    //
    EPWM_setCounterCompareShadowLoadMode(base1, EPWM_COUNTER_COMPARE_A,
                                    EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);

    //
    // set as shadow mode
    //
    EPWM_setCounterCompareShadowLoadMode(base1, EPWM_COUNTER_COMPARE_B,
                                    EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);

    EPWM_disableCounterCompareShadowLoadMode(base1, EPWM_COUNTER_COMPARE_C);

    EALLOW;
    //
    // Clear AQCTLA, B and Deadband settings settings
    //
    HWREGH(base1 + EPWM_O_AQCTLA) = 0x0000;
    HWREGH(base1 + EPWM_O_AQCTLB) = 0x0000;

    HWREGH(base1 + EPWM_O_DCBCTL) = 0x0000;
    EDIS;

    //
    // Action Qualifier SubModule Registers
    // CTR = CMPA@UP , xA set to 1
    //
    EPWM_setActionQualifierAction(base1, EPWM_AQ_OUTPUT_A ,
           EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

    //
    // CTR = CMPA@Down , xA set to 0
    //
    EPWM_setActionQualifierAction(base1, EPWM_AQ_OUTPUT_A ,
           EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

    //
    // Active high complementary PWMs - Set up the deadband
    //

    EPWM_setRisingEdgeDelayCountShadowLoadMode(base1,
                                              EPWM_RED_LOAD_ON_CNTR_ZERO);
    EPWM_setFallingEdgeDelayCountShadowLoadMode(base1,
                                              EPWM_FED_LOAD_ON_CNTR_ZERO);
    EPWM_setDeadBandCounterClock(base1, EPWM_DB_COUNTER_CLOCK_HALF_CYCLE);

    EPWM_setDeadBandDelayMode(base1, EPWM_DB_RED, true);
    EPWM_setDeadBandDelayMode(base1, EPWM_DB_FED, true);
    EPWM_setRisingEdgeDeadBandDelayInput(base1, EPWM_DB_INPUT_EPWMA);
    EPWM_setFallingEdgeDeadBandDelayInput(base1, EPWM_DB_INPUT_EPWMA);
    EPWM_setDeadBandDelayPolarity(base1, EPWM_DB_FED,
                                 EPWM_DB_POLARITY_ACTIVE_LOW);
    EPWM_setDeadBandDelayPolarity(base1, EPWM_DB_RED,
                                 EPWM_DB_POLARITY_ACTIVE_HIGH);

    EPWM_setRisingEdgeDelayCount(base1, dbRED_ticks);
    EPWM_setFallingEdgeDelayCount(base1, dbFED_ticks);	

}

void HAL_setupEPWMinUpDownCount2ChAsymmetricMode(uint32_t base1,
                            float32_t pwmFreq_Hz,
                            float32_t pwmSysClkFreq_Hz)
{
    uint32_t pwmPeriod_ticks;

    pwmPeriod_ticks = TICKS_IN_PWM_FREQUENCY(pwmFreq_Hz, pwmSysClkFreq_Hz);

    //
    // Time Base SubModule Registers
    //
    EPWM_setPeriodLoadMode(base1, EPWM_PERIOD_SHADOW_LOAD);
    EPWM_setTimeBasePeriod(base1, pwmPeriod_ticks >> 1);
    EPWM_setTimeBaseCounter(base1, 0);
    EPWM_setPhaseShift(base1, 0);
    EPWM_setTimeBaseCounterMode(base1, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_setClockPrescaler(base1, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);

    //
    // Counter Compare Submodule Registers
    // set duty 50% initially
    //
    EPWM_setCounterCompareValue(base1, EPWM_COUNTER_COMPARE_A,
                                pwmPeriod_ticks >> 2);
    //
    // set as shadow mode
    //
    EPWM_setCounterCompareShadowLoadMode(base1, EPWM_COUNTER_COMPARE_A,
                                EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);


    EPWM_setCounterCompareValue(base1, EPWM_COUNTER_COMPARE_B,
                                   pwmPeriod_ticks >> 2);

    EALLOW;
    //
    // Clear AQCTLA, B and Deadband settings settings
    //
    HWREGH(base1 + EPWM_O_AQCTLA) = 0x0000;
    HWREGH(base1 + EPWM_O_AQCTLB) = 0x0000;

    HWREGH(base1 + EPWM_O_DCBCTL) = 0x0000;
    EDIS;

    //
    // Action Qualifier SubModule Registers
    // CTR = CMPA@0 , xA set to 1
    //
    EPWM_setActionQualifierAction(base1, EPWM_AQ_OUTPUT_A ,
       EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO );
    //
    // CTR = CMPA@Up , xA set to 0
    //
    EPWM_setActionQualifierAction(base1, EPWM_AQ_OUTPUT_A ,
       EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

    //
    // CTR = CMPB@PERIOD, xB set to 1
    //
    EPWM_setActionQualifierAction(base1, EPWM_AQ_OUTPUT_B ,
       EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
    //
    // CTR = CMPB@DOWN , xB set to 0
    //
    EPWM_setActionQualifierAction(base1, EPWM_AQ_OUTPUT_B ,
       EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);

}

//
// mode is 0: no PWM
// mode is 1: prim PWM on sec PWM off
// mode is 2: prim and sec PWM on
// mode is 3: prim and sec PWM on
//
void HAL_setupPWMpins(uint16_t mode)
{
    //
    // if mode is 0 then disable prim & sec PWMs
    //
    if(mode == 0)
    {
        GPIO_writePin(PRIM_LEG1_PWM_H_GPIO, 0);
        GPIO_writePin(PRIM_LEG1_PWM_L_GPIO, 0);
        GPIO_setPinConfig(PRIM_LEG1_PWM_H_DIS_GPIO_PIN_CONFIG);
        GPIO_setPinConfig(PRIM_LEG1_PWM_L_DIS_GPIO_PIN_CONFIG);

        GPIO_writePin(PRIM_LEG2_PWM_H_GPIO, 0);
        GPIO_writePin(PRIM_LEG2_PWM_L_GPIO, 0);
        GPIO_setPinConfig(PRIM_LEG2_PWM_H_DIS_GPIO_PIN_CONFIG);
        GPIO_setPinConfig(PRIM_LEG2_PWM_L_DIS_GPIO_PIN_CONFIG);

    }
    //
    // if mode is 0 or 1 then disable sec PWM
    //
    if(mode == 0 || mode == 1)
    {
        GPIO_writePin(SEC_LEG1_PWM_H_GPIO, 0);
        GPIO_writePin(SEC_LEG1_PWM_L_GPIO, 0);
        GPIO_setPinConfig(SEC_LEG1_PWM_H_DIS_GPIO_PIN_CONFIG);
        GPIO_setPinConfig(SEC_LEG1_PWM_L_DIS_GPIO_PIN_CONFIG);

        GPIO_writePin(SEC_LEG2_PWM_H_GPIO, 0);
        GPIO_writePin(SEC_LEG2_PWM_L_GPIO, 0);
        GPIO_setPinConfig(SEC_LEG2_PWM_H_DIS_GPIO_PIN_CONFIG);
        GPIO_setPinConfig(SEC_LEG2_PWM_L_DIS_GPIO_PIN_CONFIG);
    }
    //
    // if mode is 1 or 2 or 3 then enable prim PWM
    //
    if(mode == 1 || mode == 2 || mode == 3)
    {
        GPIO_setDirectionMode(PRIM_LEG1_PWM_H_GPIO, GPIO_DIR_MODE_OUT);
        GPIO_setPadConfig(PRIM_LEG1_PWM_H_GPIO, GPIO_PIN_TYPE_STD);
        GPIO_setPinConfig(PRIM_LEG1_PWM_H_GPIO_PIN_CONFIG );

        GPIO_setDirectionMode(PRIM_LEG1_PWM_L_GPIO, GPIO_DIR_MODE_OUT);
        GPIO_setPadConfig(PRIM_LEG1_PWM_L_GPIO, GPIO_PIN_TYPE_STD);
        GPIO_setPinConfig(PRIM_LEG1_PWM_L_GPIO_PIN_CONFIG );

        GPIO_setDirectionMode(PRIM_LEG2_PWM_H_GPIO, GPIO_DIR_MODE_OUT);
        GPIO_setPadConfig(PRIM_LEG2_PWM_H_GPIO, GPIO_PIN_TYPE_STD);
        GPIO_setPinConfig(PRIM_LEG2_PWM_H_GPIO_PIN_CONFIG );

        GPIO_setDirectionMode(PRIM_LEG2_PWM_L_GPIO, GPIO_DIR_MODE_OUT);
        GPIO_setPadConfig(PRIM_LEG2_PWM_L_GPIO, GPIO_PIN_TYPE_STD);
        GPIO_setPinConfig(PRIM_LEG2_PWM_L_GPIO_PIN_CONFIG );
    }
    //
    // if mode is 2 or 3 then enable sec PWM
    //
    if(mode == 2 || mode == 3)
    {
        GPIO_setDirectionMode(SEC_LEG1_PWM_L_GPIO, GPIO_DIR_MODE_OUT);
        GPIO_setPadConfig(SEC_LEG1_PWM_L_GPIO, GPIO_PIN_TYPE_STD);
        GPIO_setPinConfig(SEC_LEG1_PWM_L_GPIO_PIN_CONFIG );

        GPIO_setDirectionMode(SEC_LEG2_PWM_L_GPIO, GPIO_DIR_MODE_OUT);
        GPIO_setPadConfig(SEC_LEG2_PWM_L_GPIO, GPIO_PIN_TYPE_STD);
        GPIO_setPinConfig(SEC_LEG2_PWM_L_GPIO_PIN_CONFIG );

        GPIO_setDirectionMode(SEC_LEG1_PWM_H_GPIO, GPIO_DIR_MODE_OUT);
        GPIO_setPadConfig(SEC_LEG1_PWM_H_GPIO, GPIO_PIN_TYPE_STD);
        GPIO_setPinConfig(SEC_LEG1_PWM_H_GPIO_PIN_CONFIG );

        GPIO_setDirectionMode(SEC_LEG2_PWM_H_GPIO, GPIO_DIR_MODE_OUT);
        GPIO_setPadConfig(SEC_LEG2_PWM_H_GPIO, GPIO_PIN_TYPE_STD);
        GPIO_setPinConfig(SEC_LEG2_PWM_H_GPIO_PIN_CONFIG );
    }
}

void HAL_setupADC(void)
{

    ADC_setVREF(ADCA_BASE, ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
    ADC_setVREF(ADCB_BASE, ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
    ADC_setVREF(ADCC_BASE, ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);

    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_2_0);
    ADC_setPrescaler(ADCB_BASE, ADC_CLK_DIV_2_0);
    ADC_setPrescaler(ADCC_BASE, ADC_CLK_DIV_2_0);

    ADC_enableConverter(ADCA_BASE);
    ADC_enableConverter(ADCB_BASE);
    ADC_enableConverter(ADCC_BASE);

    DEVICE_DELAY_US(1000);

    //
    // setup ADC conversions for current and voltage signals
    //IPRIM
    //
    ADC_setupSOC(IPRIM_ADC_MODULE,
                 IPRIM_ADC_SOC_NO,
                 IPRIM_ADC_TRIG_SOURCE,
                 IPRIM_ADC_PIN,
                 IPRIM_ADC_ACQPS_SYS_CLKS);


    //
    //ISEC
    //
    ADC_setupSOC(ISEC_ADC_MODULE,
                 ISEC_ADC_SOC_NO_1,
                 ISEC_ADC_TRIG_SOURCE,
                 ISEC_ADC_PIN,
                 ISEC_ADC_ACQPS_SYS_CLKS);

    #if OVERSAMPLING_ENABLED == 1
        ADC_setupSOC(ISEC_ADC_MODULE,
                     ISEC_ADC_SOC_NO_2,
                     ISEC_ADC_TRIG_SOURCE,
                     ISEC_ADC_PIN,
                     ISEC_ADC_ACQPS_SYS_CLKS);

        ADC_setupSOC(ISEC_ADC_MODULE,
                     ISEC_ADC_SOC_NO_3,
                     ISEC_ADC_TRIG_SOURCE,
                     ISEC_ADC_PIN,
                     ISEC_ADC_ACQPS_SYS_CLKS);

        ADC_setupSOC(ISEC_ADC_MODULE,
                     ISEC_ADC_SOC_NO_4,
                     ISEC_ADC_TRIG_SOURCE,
                     ISEC_ADC_PIN,
                     ISEC_ADC_ACQPS_SYS_CLKS);

        #if OVERSAMPLING_12x == 1
            ADC_setupSOC(ISEC_ADC_MODULE,
                         ISEC_ADC_SOC_NO_5,
                         ISEC_ADC_TRIG_SOURCE,
                         ISEC_ADC_PIN,
                         ISEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(ISEC_ADC_MODULE,
                         ISEC_ADC_SOC_NO_6,
                         ISEC_ADC_TRIG_SOURCE,
                         ISEC_ADC_PIN,
                         ISEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(ISEC_ADC_MODULE,
                         ISEC_ADC_SOC_NO_7,
                         ISEC_ADC_TRIG_SOURCE,
                         ISEC_ADC_PIN,
                         ISEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(ISEC_ADC_MODULE,
                         ISEC_ADC_SOC_NO_8,
                         ISEC_ADC_TRIG_SOURCE,
                         ISEC_ADC_PIN,
                         ISEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(ISEC_ADC_MODULE,
                         ISEC_ADC_SOC_NO_9,
                         ISEC_ADC_TRIG_SOURCE,
                         ISEC_ADC_PIN,
                         ISEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(ISEC_ADC_MODULE,
                         ISEC_ADC_SOC_NO_10,
                         ISEC_ADC_TRIG_SOURCE,
                         ISEC_ADC_PIN,
                         ISEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(ISEC_ADC_MODULE,
                         ISEC_ADC_SOC_NO_11,
                         ISEC_ADC_TRIG_SOURCE,
                         ISEC_ADC_PIN,
                         ISEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(ISEC_ADC_MODULE,
                         ISEC_ADC_SOC_NO_12,
                         ISEC_ADC_TRIG_SOURCE,
                         ISEC_ADC_PIN,
                         ISEC_ADC_ACQPS_SYS_CLKS);
        #endif

    #endif

    //
    // setup another slow ADC conversion for ISR3 trigger
    //
    ADC_setupSOC(ISEC_ADC_MODULE,
                ISEC_ADC_SOC_NO_13,
                ADC_TRIG_SLOW_SOURCE,
                ISEC_ADC_PIN,
                ISEC_ADC_ACQPS_SYS_CLKS);
    //
    //VPRIM
    //
    ADC_setupSOC(VPRIM_ADC_MODULE,
                 VPRIM_ADC_SOC_NO_1,
                 VPRIM_ADC_TRIG_SOURCE,
                 VPRIM_ADC_PIN,
                 VPRIM_ADC_ACQPS_SYS_CLKS);

    #if OVERSAMPLING_ENABLED == 1
        ADC_setupSOC(VPRIM_ADC_MODULE,
                     VPRIM_ADC_SOC_NO_2,
                     VPRIM_ADC_TRIG_SOURCE,
                     VPRIM_ADC_PIN,
                     VPRIM_ADC_ACQPS_SYS_CLKS);

        ADC_setupSOC(VPRIM_ADC_MODULE,
                     VPRIM_ADC_SOC_NO_3,
                     VPRIM_ADC_TRIG_SOURCE,
                     VPRIM_ADC_PIN,
                     VPRIM_ADC_ACQPS_SYS_CLKS);

        ADC_setupSOC(VPRIM_ADC_MODULE,
                     VPRIM_ADC_SOC_NO_4,
                     VPRIM_ADC_TRIG_SOURCE,
                     VPRIM_ADC_PIN,
                     VPRIM_ADC_ACQPS_SYS_CLKS);

        #if OVERSAMPLING_12x == 1
            ADC_setupSOC(VPRIM_ADC_MODULE,
                         VPRIM_ADC_SOC_NO_5,
                         VPRIM_ADC_TRIG_SOURCE,
                         VPRIM_ADC_PIN,
                         VPRIM_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VPRIM_ADC_MODULE,
                         VPRIM_ADC_SOC_NO_6,
                         VPRIM_ADC_TRIG_SOURCE,
                         VPRIM_ADC_PIN,
                         VPRIM_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VPRIM_ADC_MODULE,
                         VPRIM_ADC_SOC_NO_7,
                         VPRIM_ADC_TRIG_SOURCE,
                         VPRIM_ADC_PIN,
                         VPRIM_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VPRIM_ADC_MODULE,
                         VPRIM_ADC_SOC_NO_8,
                         VPRIM_ADC_TRIG_SOURCE,
                         VPRIM_ADC_PIN,
                         VPRIM_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VPRIM_ADC_MODULE,
                         VPRIM_ADC_SOC_NO_9,
                         VPRIM_ADC_TRIG_SOURCE,
                         VPRIM_ADC_PIN,
                         VPRIM_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VPRIM_ADC_MODULE,
                         VPRIM_ADC_SOC_NO_10,
                         VPRIM_ADC_TRIG_SOURCE,
                         VPRIM_ADC_PIN,
                         VPRIM_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VPRIM_ADC_MODULE,
                         VPRIM_ADC_SOC_NO_11,
                         VPRIM_ADC_TRIG_SOURCE,
                         VPRIM_ADC_PIN,
                         VPRIM_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VPRIM_ADC_MODULE,
                         VPRIM_ADC_SOC_NO_12,
                         VPRIM_ADC_TRIG_SOURCE,
                         VPRIM_ADC_PIN,
                         VPRIM_ADC_ACQPS_SYS_CLKS);
        #endif

    #endif

    //
    // setup another slow ADC conversion for ISR3 trigger
    //
    ADC_setupSOC(VPRIM_ADC_MODULE,
                 VPRIM_ADC_SOC_NO_13,
                 ADC_TRIG_SLOW_SOURCE,
                 VPRIM_ADC_PIN,
                 VPRIM_ADC_ACQPS_SYS_CLKS);

    //
    //VSEC
    //
    ADC_setupSOC(VSEC_ADC_MODULE,
                 VSEC_ADC_SOC_NO_1,
                 VSEC_ADC_TRIG_SOURCE,
                 VSEC_ADC_PIN,
                 VSEC_ADC_ACQPS_SYS_CLKS);

    #if OVERSAMPLING_ENABLED == 1
        ADC_setupSOC(VSEC_ADC_MODULE,
                     VSEC_ADC_SOC_NO_2,
                     VSEC_ADC_TRIG_SOURCE,
                     VSEC_ADC_PIN,
                     VSEC_ADC_ACQPS_SYS_CLKS);

        ADC_setupSOC(VSEC_ADC_MODULE,
                     VSEC_ADC_SOC_NO_3,
                     VSEC_ADC_TRIG_SOURCE,
                     VSEC_ADC_PIN,
                     VSEC_ADC_ACQPS_SYS_CLKS);

        ADC_setupSOC(VSEC_ADC_MODULE,
                     VSEC_ADC_SOC_NO_4,
                     VSEC_ADC_TRIG_SOURCE,
                     VSEC_ADC_PIN,
                     VSEC_ADC_ACQPS_SYS_CLKS);

        #if OVERSAMPLING_12x == 1
            ADC_setupSOC(VSEC_ADC_MODULE,
                         VSEC_ADC_SOC_NO_5,
                         VSEC_ADC_TRIG_SOURCE,
                         VSEC_ADC_PIN,
                         VSEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VSEC_ADC_MODULE,
                         VSEC_ADC_SOC_NO_6,
                         VSEC_ADC_TRIG_SOURCE,
                         VSEC_ADC_PIN,
                         VSEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VSEC_ADC_MODULE,
                         VSEC_ADC_SOC_NO_7,
                         VSEC_ADC_TRIG_SOURCE,
                         VSEC_ADC_PIN,
                         VSEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VSEC_ADC_MODULE,
                         VSEC_ADC_SOC_NO_8,
                         VSEC_ADC_TRIG_SOURCE,
                         VSEC_ADC_PIN,
                         VSEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VSEC_ADC_MODULE,
                         VSEC_ADC_SOC_NO_9,
                         VSEC_ADC_TRIG_SOURCE,
                         VSEC_ADC_PIN,
                         VSEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VSEC_ADC_MODULE,
                         VSEC_ADC_SOC_NO_10,
                         VSEC_ADC_TRIG_SOURCE,
                         VSEC_ADC_PIN,
                         VSEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VSEC_ADC_MODULE,
                         VSEC_ADC_SOC_NO_11,
                         VSEC_ADC_TRIG_SOURCE,
                         VSEC_ADC_PIN,
                         VSEC_ADC_ACQPS_SYS_CLKS);

            ADC_setupSOC(VSEC_ADC_MODULE,
                         VSEC_ADC_SOC_NO_12,
                         VSEC_ADC_TRIG_SOURCE,
                         VSEC_ADC_PIN,
                         VSEC_ADC_ACQPS_SYS_CLKS);
        #endif

    #endif

    //
    // setup another slow ADC conversion for ISR3 trigger
    //
    ADC_setupSOC(VSEC_ADC_MODULE,
                 VSEC_ADC_SOC_NO_13,
                 ADC_TRIG_SLOW_SOURCE,
                 VSEC_ADC_PIN,
                 VSEC_ADC_ACQPS_SYS_CLKS);

}

void HAL_setupProfilingGPIO(void)
{
    GPIO_setDirectionMode(GPIO_PROFILING1, GPIO_DIR_MODE_OUT);
    GPIO_setDirectionMode(GPIO_PROFILING2, GPIO_DIR_MODE_OUT);
    GPIO_setDirectionMode(GPIO_PROFILING3, GPIO_DIR_MODE_OUT);

    GPIO_setQualificationMode(GPIO_PROFILING1, GPIO_QUAL_SYNC);
    GPIO_setQualificationMode(GPIO_PROFILING2, GPIO_QUAL_SYNC);
    GPIO_setQualificationMode(GPIO_PROFILING3, GPIO_QUAL_SYNC);

    GPIO_setPinConfig(GPIO_PROFILING1_PIN_CONFIG);
    GPIO_setPinConfig(GPIO_PROFILING2_PIN_CONFIG);
    GPIO_setPinConfig(GPIO_PROFILING3_PIN_CONFIG);
}

void HAL_setupTrigForADC()
{
    //
    //PWM module is used to trigger the SOC in this application
    //As control is carried out in ISR2,
    //The PWM used for ISR2 is configured below
    //Select SOC from counter at ctr =CMPBD
    //
    EPWM_setADCTriggerSource(ISR2_PWM_BASE,
                             EPWM_SOC_A, EPWM_SOC_TBCTR_D_CMPB );
    //
    // set CMPB such that the triggers for the ADC are centered around zero
    // of the ISR generating timebase
    //
    EPWM_setCounterCompareValue(ISR2_PWM_BASE,
                                 EPWM_COUNTER_COMPARE_B,
                                 (IPRIM_ADC_ACQPS_SYS_CLKS *
                      ((float32_t)PWMSYSCLOCK_FREQ_HZ /
                       (float32_t)CPU_SYS_CLOCK_FREQ_HZ)));

    //
    // Generate pulse on 1st even
    //
    EPWM_setADCTriggerEventPrescale(ISR2_PWM_BASE,
                                    EPWM_SOC_A, 1);

    //
    // Enable SOC on A group
    //
    EPWM_enableADCTrigger(ISR2_PWM_BASE,
                          EPWM_SOC_A);

    //
    // for the faster signals such as shunt current sense, the PWM time base of
    // the module used to control the power stage must be used.
    // Also as we are interested in the peak value we must sample close to the
    // falling edge of the leg1 H PWM
    // Select SOC from counter at ctr =CMPA
    //
    EPWM_setADCTriggerSource(PRIM_LEG1_PWM_BASE,
                             EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPD);
    //
    // CMPA is updated in the ISR so nothing to do here
    // pre-scale the trigger of the ADC conversion
    //
    EPWM_setADCTriggerEventPrescale(PRIM_LEG1_PWM_BASE,
                                        EPWM_SOC_A, 6);

    EPWM_enableADCTrigger(PRIM_LEG1_PWM_BASE,
                          EPWM_SOC_A);
}

void HAL_setupSynchronousRectificationActionDebug(uint16_t powerFlow)
{
    //
    // For debug purpose bring out the CTRIPHSEL on an output xBar
    //
    if(powerFlow == 1)
    {

        XBAR_setOutputMuxConfig(XBAR_OUTPUT1,
                                ISEC_TANK_H_OUT_XBAR_MUX_VAL);
        XBAR_enableOutputMux(XBAR_OUTPUT1,
                             ISEC_TANK_H_XBAR_MUX);
        XBAR_setOutputMuxConfig(XBAR_OUTPUT2,
                                ISEC_TANK_L_OUT_XBAR_MUX_VAL);
        XBAR_enableOutputMux(XBAR_OUTPUT2,
                             ISEC_TANK_L_XBAR_MUX);
    }
    else
    {
        XBAR_setOutputMuxConfig(XBAR_OUTPUT1,
                                IPRIM_TANK_H_OUT_XBAR_MUX_VAL);
        XBAR_enableOutputMux(XBAR_OUTPUT1,
                             IPRIM_TANK_H_XBAR_MUX);
        XBAR_setOutputMuxConfig(XBAR_OUTPUT2,
                                IPRIM_TANK_L_OUT_XBAR_MUX_VAL);
        XBAR_enableOutputMux(XBAR_OUTPUT2,
                             IPRIM_TANK_L_XBAR_MUX);
    }


    GPIO_setDirectionMode(GPIO_XBAR1, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(GPIO_XBAR1, GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(GPIO_XBAR1_PIN_CONFIG);

    GPIO_setDirectionMode(GPIO_XBAR2, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(GPIO_XBAR2, GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(GPIO_XBAR2_PIN_CONFIG);
}

void HAL_setupBoardProtection()
{
    //
    // Disable all the muxes first
    //
    XBAR_enableEPWMMux(XBAR_TRIP4, 0x00);

#if BOARD_PROTECTION_IPRIM == 1
    ASysCtl_selectCMPHPMux(IPRIM_CMPSS_ASYSCTRL_CMPHPMUX,
                           IPRIM_CMPSS_ASYSCTRL_MUX_VALUE);
    ASysCtl_selectCMPLPMux(IPRIM_CMPSS_ASYSCTRL_CMPLPMUX,
                           IPRIM_CMPSS_ASYSCTRL_MUX_VALUE);

    HAL_setupCMPSSHighLowLimit(IPRIM_CMPSS_BASE,
                                    IPRIM_TRIP_LIMIT_AMPS,
                                    IPRIM_MAX_SENSE_AMPS,
                                    CMPSS_HYSTERESIS,
                                    CMPSSS_FILTER_PRESCALAR,
                                    CMPSS_WINODW,
                                    CMPSS_THRESHOLD);

    XBAR_setEPWMMuxConfig(XBAR_TRIP4, IPRIM_CMPSS_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, IPRIM_CMPSS_XBAR_MUX);
    XBAR_clearInputFlag(IPRIM_CMPSS_XBAR_FLAG1);
    XBAR_clearInputFlag(IPRIM_CMPSS_XBAR_FLAG2);
#else
#endif

#if BOARD_PROTECTION_IPRIM_TANK == 1
    ASysCtl_selectCMPHPMux(IPRIM_SHUNT_CMPSS_ASYSCTRL_CMPHPMUX,
                           IPRIM_SHUNT_CMPSS_ASYSCTRL_MUX_VALUE);
    ASysCtl_selectCMPLPMux(IPRIM_SHUNT_CMPSS_ASYSCTRL_CMPLPMUX,
                           IPRIM_SHUNT_CMPSS_ASYSCTRL_MUX_VALUE);

    HAL_setupCMPSSHighLowLimit(IPRIM_SHUNT_CMPSS_BASE,
                                IPRIM_TRIP_LIMIT_AMPS,
                                IPRIM_TANK_MAX_SENSE_AMPS,
                                CMPSS_HYSTERESIS,
                                CMPSSS_FILTER_PRESCALAR,
                                CMPSS_WINODW,
                                CMPSS_THRESHOLD);

    XBAR_setEPWMMuxConfig(XBAR_TRIP4, IPRIM_SHUNT_CMPSS_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, IPRIM_SHUNT_CMPSS_XBAR_MUX);
    XBAR_clearInputFlag(IPRIM_SHUNT_CMPSS_XBAR_FLAG1);
    XBAR_clearInputFlag(IPRIM_SHUNT_CMPSS_XBAR_FLAG2);
#else
    #warning BOARD_PROTECTION_IPRIM is disabled
#endif

#if BOARD_PROTECTION_ISEC == 1
    ASysCtl_selectCMPHPMux(ISEC_CMPSS_ASYSCTRL_CMPHPMUX,
                           ISEC_CMPSS_ASYSCTRL_MUX_VALUE);
    ASysCtl_selectCMPLPMux(ISEC_CMPSS_ASYSCTRL_CMPLPMUX,
                           ISEC_CMPSS_ASYSCTRL_MUX_VALUE);

    HAL_setupCMPSSHighLowLimit(ISEC_CMPSS_BASE,
                ISEC_TRIP_LIMIT_AMPS,
                ISEC_MAX_SENSE_AMPS,
                CMPSS_HYSTERESIS,
                CMPSSS_FILTER_PRESCALAR,
                CMPSS_WINODW,
                CMPSS_THRESHOLD);

    XBAR_setEPWMMuxConfig(XBAR_TRIP4, ISEC_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, ISEC_XBAR_MUX);
    XBAR_clearInputFlag(ISEC_CMPSS_XBAR_FLAG1);
    XBAR_clearInputFlag(ISEC_CMPSS_XBAR_FLAG2);
#else
    #warning BOARD_PROTECTION_ISEC is disabled
#endif

#if BOARD_PROTECTION_IPRIM == 1 ||                                       \
    BOARD_PROTECTION_IPRIM_SHUNT == 1 ||                                 \
    BOARD_PROTECTION_ISEC == 1

    //
    //Trip 4 is the input to the DCAHCOMPSEL
    //
    EPWM_selectDigitalCompareTripInput(PRIM_LEG1_PWM_BASE,
                                       EPWM_DC_TRIP_TRIPIN4,
                                       EPWM_DC_TYPE_DCAH);
    EPWM_setTripZoneDigitalCompareEventCondition(PRIM_LEG1_PWM_BASE,
                                                 EPWM_TZ_DC_OUTPUT_A1,
                                                 EPWM_TZ_EVENT_DCXH_HIGH);
    EPWM_setDigitalCompareEventSource(PRIM_LEG1_PWM_BASE,
                                      EPWM_DC_MODULE_A,
                                      EPWM_DC_EVENT_1,
                                      EPWM_DC_EVENT_SOURCE_ORIG_SIGNAL);
    EPWM_setDigitalCompareEventSyncMode(PRIM_LEG1_PWM_BASE,
                                        EPWM_DC_MODULE_A ,
                                        EPWM_DC_EVENT_1,
                                        EPWM_DC_EVENT_INPUT_NOT_SYNCED);

    EPWM_selectDigitalCompareTripInput(PRIM_LEG2_PWM_BASE,
                                       EPWM_DC_TRIP_TRIPIN4,
                                       EPWM_DC_TYPE_DCAH);
    EPWM_setTripZoneDigitalCompareEventCondition(PRIM_LEG2_PWM_BASE,
                                                 EPWM_TZ_DC_OUTPUT_A1,
                                                 EPWM_TZ_EVENT_DCXH_HIGH);
    EPWM_setDigitalCompareEventSource(PRIM_LEG2_PWM_BASE,
                                      EPWM_DC_MODULE_A,
                                      EPWM_DC_EVENT_1,
                                      EPWM_DC_EVENT_SOURCE_ORIG_SIGNAL);
    EPWM_setDigitalCompareEventSyncMode(PRIM_LEG2_PWM_BASE,
                                        EPWM_DC_MODULE_A ,
                                        EPWM_DC_EVENT_1,
                                        EPWM_DC_EVENT_INPUT_NOT_SYNCED);

    EPWM_selectDigitalCompareTripInput(SEC_LEG1_PWM_BASE,
                                       EPWM_DC_TRIP_TRIPIN4,
                                       EPWM_DC_TYPE_DCAH);
    EPWM_setTripZoneDigitalCompareEventCondition(SEC_LEG1_PWM_BASE,
                                                 EPWM_TZ_DC_OUTPUT_A1,
                                                 EPWM_TZ_EVENT_DCXH_HIGH);
    EPWM_setDigitalCompareEventSource(SEC_LEG1_PWM_BASE,
                                      EPWM_DC_MODULE_A,
                                      EPWM_DC_EVENT_1,
                                      EPWM_DC_EVENT_SOURCE_ORIG_SIGNAL);
    EPWM_setDigitalCompareEventSyncMode(SEC_LEG1_PWM_BASE,
                                        EPWM_DC_MODULE_A ,
                                        EPWM_DC_EVENT_1,
                                        EPWM_DC_EVENT_INPUT_NOT_SYNCED);

    EPWM_selectDigitalCompareTripInput(SEC_LEG2_PWM_BASE,
                                       EPWM_DC_TRIP_TRIPIN4,
                                       EPWM_DC_TYPE_DCAH);
    EPWM_setTripZoneDigitalCompareEventCondition(SEC_LEG2_PWM_BASE,
                                                 EPWM_TZ_DC_OUTPUT_A1,
                                                 EPWM_TZ_EVENT_DCXH_HIGH);
    EPWM_setDigitalCompareEventSource(SEC_LEG2_PWM_BASE,
                                      EPWM_DC_MODULE_A,
                                      EPWM_DC_EVENT_1,
                                      EPWM_DC_EVENT_SOURCE_ORIG_SIGNAL);
    EPWM_setDigitalCompareEventSyncMode(SEC_LEG2_PWM_BASE,
                                        EPWM_DC_MODULE_A ,
                                        EPWM_DC_EVENT_1,
                                        EPWM_DC_EVENT_INPUT_NOT_SYNCED);

    //
    // Enable the following trips Emulator Stop, TZ1-3 and DCAEVT1
    //
    EPWM_enableTripZoneSignals(PRIM_LEG1_PWM_BASE,
                               EPWM_TZ_SIGNAL_DCAEVT1);
    EPWM_enableTripZoneSignals(PRIM_LEG2_PWM_BASE,
                               EPWM_TZ_SIGNAL_DCAEVT1);
    EPWM_enableTripZoneSignals(SEC_LEG1_PWM_BASE,
                               EPWM_TZ_SIGNAL_DCAEVT1);
    EPWM_enableTripZoneSignals(SEC_LEG2_PWM_BASE,
                               EPWM_TZ_SIGNAL_DCAEVT1);

#else
    #warning All current comparator based protections are disabled
#endif

    //
    // Enable the following trips Emulator Stop
    //
    EPWM_enableTripZoneSignals(PRIM_LEG1_PWM_BASE,
                               EPWM_TZ_SIGNAL_CBC6);
    EPWM_enableTripZoneSignals(PRIM_LEG2_PWM_BASE,
                               EPWM_TZ_SIGNAL_CBC6);
    EPWM_enableTripZoneSignals(SEC_LEG1_PWM_BASE,
                               EPWM_TZ_SIGNAL_CBC6);
    EPWM_enableTripZoneSignals(SEC_LEG2_PWM_BASE,
                               EPWM_TZ_SIGNAL_CBC6);

    //
    // What do we want the OST/CBC events to do?
    // TZA events can force EPWMxA
    // TZB events can force EPWMxB
    //
    EPWM_setTripZoneAction(PRIM_LEG1_PWM_BASE,
                           EPWM_TZ_ACTION_EVENT_TZA,
                           EPWM_TZ_ACTION_LOW);
    EPWM_setTripZoneAction(PRIM_LEG1_PWM_BASE,
                           EPWM_TZ_ACTION_EVENT_TZB,
                           EPWM_TZ_ACTION_LOW);
    EPWM_setTripZoneAction(PRIM_LEG2_PWM_BASE,
                           EPWM_TZ_ACTION_EVENT_TZA,
                           EPWM_TZ_ACTION_LOW);
    EPWM_setTripZoneAction(PRIM_LEG2_PWM_BASE,
                           EPWM_TZ_ACTION_EVENT_TZB,
                           EPWM_TZ_ACTION_LOW);
    EPWM_setTripZoneAction(SEC_LEG1_PWM_BASE,
                           EPWM_TZ_ACTION_EVENT_TZA,
                           EPWM_TZ_ACTION_LOW);
    EPWM_setTripZoneAction(SEC_LEG1_PWM_BASE,
                           EPWM_TZ_ACTION_EVENT_TZB,
                           EPWM_TZ_ACTION_LOW);
    EPWM_setTripZoneAction(SEC_LEG2_PWM_BASE,
                           EPWM_TZ_ACTION_EVENT_TZA,
                           EPWM_TZ_ACTION_LOW);
    EPWM_setTripZoneAction(SEC_LEG2_PWM_BASE,
                           EPWM_TZ_ACTION_EVENT_TZB,
                           EPWM_TZ_ACTION_LOW);

    //
    // Clear any spurious trip
    //
    EPWM_clearTripZoneFlag(PRIM_LEG1_PWM_BASE ,
                           (EPWM_TZ_INTERRUPT_OST | EPWM_TZ_INTERRUPT_DCAEVT1));
    EPWM_clearTripZoneFlag(PRIM_LEG2_PWM_BASE ,
                           (EPWM_TZ_INTERRUPT_OST | EPWM_TZ_INTERRUPT_DCAEVT1));
    EPWM_clearTripZoneFlag(SEC_LEG1_PWM_BASE ,
                           (EPWM_TZ_INTERRUPT_OST | EPWM_TZ_INTERRUPT_DCAEVT1));
    EPWM_clearTripZoneFlag(SEC_LEG2_PWM_BASE ,
                           (EPWM_TZ_INTERRUPT_OST | EPWM_TZ_INTERRUPT_DCAEVT1));

    EPWM_forceTripZoneEvent(PRIM_LEG1_PWM_BASE, EPWM_TZ_FORCE_EVENT_OST);
    EPWM_forceTripZoneEvent(PRIM_LEG2_PWM_BASE, EPWM_TZ_FORCE_EVENT_OST);
    EPWM_forceTripZoneEvent(SEC_LEG1_PWM_BASE, EPWM_TZ_FORCE_EVENT_OST);
    EPWM_forceTripZoneEvent(SEC_LEG2_PWM_BASE, EPWM_TZ_FORCE_EVENT_OST);
}

void HAL_setupSynchronousRectificationAction(uint16_t powerFlow)
{
    if(powerFlow == POWER_FLOW_PRIM_SEC)
    {
        ASysCtl_selectCMPHPMux(ISEC_TANK_CMPSS_ASYSCTRL_CMPHPMUX,
                               ISEC_TANK_CMPSS_ASYSCTRL_MUX_VALUE);

        ASysCtl_selectCMPLPMux(ISEC_TANK_CMPSS_ASYSCTRL_CMPLPMUX,
                               ISEC_TANK_CMPSS_ASYSCTRL_MUX_VALUE);

        //
        //Enable CMPSS
        //
        CMPSS_enableModule(ISEC_TANK_CMPSS_BASE);

        //
        //Use VDDA as the reference for comparator DACs
        //
        CMPSS_configDAC(ISEC_TANK_CMPSS_BASE,
                       CMPSS_DACVAL_SYSCLK | CMPSS_DACREF_VDDA
                       | CMPSS_DACSRC_SHDW);

        //
        // set DAC H and L values
        //
        CMPSS_setDACValueHigh(ISEC_TANK_CMPSS_BASE,
                              ISEC_TANK_DACHVAL);
        CMPSS_setDACValueLow(ISEC_TANK_CMPSS_BASE,
                             ISEC_TANK_DACLVAL);

        //
        // CMPH comparison is inverted because we want to trip for xA
        // when this signal goes below zero, as the pin is connected to + sign
        // the output needs to be inverted to be the right logical level
        //
        CMPSS_configHighComparator(ISEC_TANK_CMPSS_BASE,
                                   CMPSS_INSRC_DAC | CMPSS_INV_INVERTED);

        //
        // CMPL is not inverted because we want to trip when the
        // signal goes above zero, CMPSS pin is connected to + sign of
        // the comparator, hence no sign inversion required
        //
        CMPSS_configLowComparator(ISEC_TANK_CMPSS_BASE, CMPSS_INSRC_DAC );

        //
        // configure SEC PWM LEG1 PWM to issue the blanking signal
        //
        CMPSS_configBlanking(ISEC_TANK_CMPSS_BASE, SEC_LEG1_PWM_NO);

        CMPSS_enableBlanking(ISEC_TANK_CMPSS_BASE);

        CMPSS_configLatchOnPWMSYNC(ISEC_TANK_CMPSS_BASE, TRUE, TRUE);

        EALLOW;
        HWREGH(ISEC_TANK_CMPSS_BASE + CMPSS_O_COMPDACCTL) |= 0x1E;
        EDIS;

        //
        // configure the filter to the lowest setting
        //
        CMPSS_configFilterHigh(ISEC_TANK_CMPSS_BASE, 0, 1, 1);
        CMPSS_configFilterLow(ISEC_TANK_CMPSS_BASE, 0, 1, 1);

        //
        //Reset filter logic & start filtering
        //
        CMPSS_initFilterHigh(ISEC_TANK_CMPSS_BASE);
        CMPSS_initFilterLow(ISEC_TANK_CMPSS_BASE);


        CMPSS_configOutputsHigh(ISEC_TANK_CMPSS_BASE,
                                CMPSS_TRIP_LATCH | CMPSS_TRIPOUT_LATCH);
        CMPSS_configOutputsLow(ISEC_TANK_CMPSS_BASE,
                               CMPSS_TRIP_LATCH | CMPSS_TRIPOUT_LATCH);

        //
        //Comparator hysteresis control , set to 2x typical value
        //
        CMPSS_setHysteresis(ISEC_TANK_CMPSS_BASE, 2);

        //
        // Clear the latched comparator events
        //
        CMPSS_clearFilterLatchHigh(ISEC_TANK_CMPSS_BASE);
        CMPSS_clearFilterLatchLow(ISEC_TANK_CMPSS_BASE);

        XBAR_setEPWMMuxConfig(XBAR_TRIP5,
                              ISEC_TANK_H_PWM_XBAR_MUX_VAL);
        XBAR_enableEPWMMux(XBAR_TRIP5,
                           ISEC_TANK_H_XBAR_MUX);

        XBAR_setEPWMMuxConfig(XBAR_TRIP7,
                              ISEC_TANK_L_PWM_XBAR_MUX_VAL);
        XBAR_enableEPWMMux(XBAR_TRIP7,
                           ISEC_TANK_L_XBAR_MUX);

        XBAR_clearInputFlag(ISEC_TANK_H_CMPSS_XBAR_FLAG);
        XBAR_clearInputFlag(ISEC_TANK_L_CMPSS_XBAR_FLAG);

        //
        // configure EPWM to issue blanking pulse
        //

        EPWM_setDigitalCompareBlankingEvent(SEC_LEG1_PWM_BASE,
                                        EPWM_DC_WINDOW_START_TBCTR_ZERO_PERIOD);

        EPWM_setDigitalCompareWindowOffset(SEC_LEG1_PWM_BASE,
                                                0);

        EPWM_setDigitalCompareWindowLength(SEC_LEG1_PWM_BASE, 25);

        EPWM_enableDigitalCompareBlankingWindow(SEC_LEG1_PWM_BASE);

        //
        // Now also program the behavior of the PWM to accept
        // the TRIP5 and 7 that are generated by the CMPSS
        // Qualify TRIP5 as DCAL event
        // Qualify TRIP7 as DCBL event
        //

        EPWM_selectDigitalCompareTripInput(SEC_LEG1_PWM_BASE,
                                           EPWM_DC_TRIP_TRIPIN5,
                                           EPWM_DC_TYPE_DCAL);

        EPWM_selectDigitalCompareTripInput(SEC_LEG1_PWM_BASE,
                                           EPWM_DC_TRIP_TRIPIN7,
                                           EPWM_DC_TYPE_DCBL);

        EPWM_selectDigitalCompareTripInput(SEC_LEG2_PWM_BASE,
                                           EPWM_DC_TRIP_TRIPIN7,
                                           EPWM_DC_TYPE_DCAL);

        EPWM_selectDigitalCompareTripInput(SEC_LEG2_PWM_BASE,
                                           EPWM_DC_TRIP_TRIPIN5,
                                           EPWM_DC_TYPE_DCBL);

        //
        // Qualify DCAEVT2 to be when DCAL is high
        // Qualify DCBEVT2 to be when DCBL is high
        //
        EPWM_setTripZoneDigitalCompareEventCondition(SEC_LEG1_PWM_BASE,
                                          EPWM_TZ_DC_OUTPUT_A2,
                                          EPWM_TZ_EVENT_DCXL_HIGH);

        EPWM_setTripZoneDigitalCompareEventCondition(SEC_LEG1_PWM_BASE,
                                          EPWM_TZ_DC_OUTPUT_B2,
                                          EPWM_TZ_EVENT_DCXL_HIGH);

        EPWM_setTripZoneDigitalCompareEventCondition(SEC_LEG2_PWM_BASE,
                                          EPWM_TZ_DC_OUTPUT_A2,
                                          EPWM_TZ_EVENT_DCXL_HIGH);

        EPWM_setTripZoneDigitalCompareEventCondition(SEC_LEG2_PWM_BASE,
                                          EPWM_TZ_DC_OUTPUT_B2,
                                          EPWM_TZ_EVENT_DCXL_HIGH);


        //
        // sets the ETZE bit to 1, to enable advanced actions on the PWM
        //
        EPWM_enableTripZoneAdvAction(SEC_LEG1_PWM_BASE);
        EPWM_enableTripZoneAdvAction(SEC_LEG2_PWM_BASE);

        //
        // first set all the TZCTLDCX registers to do nothing
        //
        EALLOW;
        HWREGH(SEC_LEG1_PWM_BASE + EPWM_O_TZCTLDCA) = 0xFFFF;
        HWREGH(SEC_LEG1_PWM_BASE + EPWM_O_TZCTLDCB) = 0xFFFF;
        HWREGH(SEC_LEG2_PWM_BASE + EPWM_O_TZCTLDCA) = 0xFFFF;
        HWREGH(SEC_LEG2_PWM_BASE + EPWM_O_TZCTLDCB) = 0xFFFF;
        EDIS;

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

        //
        //    clear the cycle by cycle trip on zero and period
        //
        EPWM_selectCycleByCycleTripZoneClearEvent(SEC_LEG1_PWM_BASE,
                                    EPWM_TZ_CBC_PULSE_CLR_CNTR_ZERO_PERIOD);
        EPWM_selectCycleByCycleTripZoneClearEvent(SEC_LEG2_PWM_BASE,
                                    EPWM_TZ_CBC_PULSE_CLR_CNTR_ZERO_PERIOD);
    }
    else if(powerFlow == POWER_FLOW_SEC_PRIM)
    {
        ASysCtl_selectCMPHPMux(IPRIM_TANK_CMPSS_ASYSCTRL_CMPHPMUX,
                               IPRIM_TANK_CMPSS_ASYSCTRL_MUX_VALUE);

        ASysCtl_selectCMPLPMux(IPRIM_TANK_CMPSS_ASYSCTRL_CMPLPMUX,
                               IPRIM_TANK_CMPSS_ASYSCTRL_MUX_VALUE);

        //
        //Enable CMPSS
        //
        CMPSS_enableModule(IPRIM_TANK_CMPSS_BASE);

        //
        //Use VDDA as the reference for comparator DACs
        //
        CMPSS_configDAC(IPRIM_TANK_CMPSS_BASE,
                       CMPSS_DACVAL_SYSCLK | CMPSS_DACREF_VDDA
                       | CMPSS_DACSRC_SHDW);

        //
        // set DAC H and L values
        //
        CMPSS_setDACValueHigh(IPRIM_TANK_CMPSS_BASE,
                              IPRIM_TANK_DACHVAL);
        CMPSS_setDACValueLow(IPRIM_TANK_CMPSS_BASE,
                             IPRIM_TANK_DACLVAL);

        //
        // CMPH comparison is inverted because we want to trip for xA
        // when this signal goes below zero, as the pin is connected to + sign
        // the output needs to be inverted to be the right logical level
        //
        CMPSS_configHighComparator(IPRIM_TANK_CMPSS_BASE,
                                   CMPSS_INSRC_DAC | CMPSS_INV_INVERTED);

        //
        // CMPL is not inverted because we want to trip when the
        // signal goes above zero, CMPSS pin is connected to + sign of
        // the comparator, hence no sign inversion required
        //
        CMPSS_configLowComparator(IPRIM_TANK_CMPSS_BASE,
                                  CMPSS_INSRC_DAC );

        //
        // configure PRIM PWM LEG1 PWM to issue the blanking signal
        //
        CMPSS_configBlanking(IPRIM_TANK_CMPSS_BASE,
                             PRIM_LEG1_PWM_NO);

        CMPSS_enableBlanking(IPRIM_TANK_CMPSS_BASE);

        CMPSS_configLatchOnPWMSYNC(IPRIM_TANK_CMPSS_BASE, TRUE, TRUE);

        EALLOW;
        HWREGH(IPRIM_TANK_CMPSS_BASE + CMPSS_O_COMPDACCTL) |= 0x1E;
        EDIS;

        //
        // configure the filter to the lowest setting
        //
        CMPSS_configFilterHigh(IPRIM_TANK_CMPSS_BASE, 0, 1, 1);
        CMPSS_configFilterLow(IPRIM_TANK_CMPSS_BASE, 0, 1, 1);

        //
        //Reset filter logic & start filtering
        //
        CMPSS_initFilterHigh(IPRIM_TANK_CMPSS_BASE);
        CMPSS_initFilterLow(IPRIM_TANK_CMPSS_BASE);


        CMPSS_configOutputsHigh(IPRIM_TANK_CMPSS_BASE,
                                CMPSS_TRIP_LATCH | CMPSS_TRIPOUT_LATCH);
        CMPSS_configOutputsLow(IPRIM_TANK_CMPSS_BASE,
                               CMPSS_TRIP_LATCH | CMPSS_TRIPOUT_LATCH);

        //
        //Comparator hysteresis control , set to 2x typical value
        //
        CMPSS_setHysteresis(IPRIM_TANK_CMPSS_BASE, 2);

        //
        // Clear the latched comparator events
        //
        CMPSS_clearFilterLatchHigh(IPRIM_TANK_CMPSS_BASE);
        CMPSS_clearFilterLatchLow(IPRIM_TANK_CMPSS_BASE);

        XBAR_setEPWMMuxConfig(XBAR_TRIP5,
                              IPRIM_TANK_H_PWM_XBAR_MUX_VAL);
        XBAR_enableEPWMMux(XBAR_TRIP5,
                           IPRIM_TANK_H_XBAR_MUX);

        XBAR_setEPWMMuxConfig(XBAR_TRIP7,
                              IPRIM_TANK_L_PWM_XBAR_MUX_VAL);
        XBAR_enableEPWMMux(XBAR_TRIP7,
                           IPRIM_TANK_L_XBAR_MUX);

        XBAR_clearInputFlag(IPRIM_TANK_H_CMPSS_XBAR_FLAG);
        XBAR_clearInputFlag(IPRIM_TANK_L_CMPSS_XBAR_FLAG);

        //
        // configure EPWM to issue blanking pulse
        //

        EPWM_setDigitalCompareBlankingEvent(PRIM_LEG1_PWM_BASE,
                                        EPWM_DC_WINDOW_START_TBCTR_ZERO_PERIOD);

        EPWM_setDigitalCompareWindowOffset(PRIM_LEG1_PWM_BASE,
                                                0);

        EPWM_setDigitalCompareWindowLength(PRIM_LEG1_PWM_BASE, 25);

        EPWM_enableDigitalCompareBlankingWindow(PRIM_LEG1_PWM_BASE);

        //
        // Now also program the behavior of the PWM to accept
        // the TRIP5 and 7 that are generated by the CMPSS
        // Qualify TRIP5 as DCAL event
        // Qualify TRIP7 as DCBL event
        //

        EPWM_selectDigitalCompareTripInput(PRIM_LEG1_PWM_BASE,
                                           EPWM_DC_TRIP_TRIPIN5,
                                           EPWM_DC_TYPE_DCAL);

        EPWM_selectDigitalCompareTripInput(PRIM_LEG1_PWM_BASE,
                                           EPWM_DC_TRIP_TRIPIN7,
                                           EPWM_DC_TYPE_DCBL);

        EPWM_selectDigitalCompareTripInput(PRIM_LEG2_PWM_BASE,
                                           EPWM_DC_TRIP_TRIPIN7,
                                           EPWM_DC_TYPE_DCAL);

        EPWM_selectDigitalCompareTripInput(PRIM_LEG2_PWM_BASE,
                                           EPWM_DC_TRIP_TRIPIN5,
                                           EPWM_DC_TYPE_DCBL);

        //
        // Qualify DCAEVT2 to be when DCAL is high
        // Qualify DCBEVT2 to be when DCBL is high
        //
        EPWM_setTripZoneDigitalCompareEventCondition(PRIM_LEG1_PWM_BASE,
                                          EPWM_TZ_DC_OUTPUT_A2,
                                          EPWM_TZ_EVENT_DCXL_HIGH);

        EPWM_setTripZoneDigitalCompareEventCondition(PRIM_LEG1_PWM_BASE,
                                          EPWM_TZ_DC_OUTPUT_B2,
                                          EPWM_TZ_EVENT_DCXL_HIGH);

        EPWM_setTripZoneDigitalCompareEventCondition(PRIM_LEG2_PWM_BASE,
                                          EPWM_TZ_DC_OUTPUT_A2,
                                          EPWM_TZ_EVENT_DCXL_HIGH);

        EPWM_setTripZoneDigitalCompareEventCondition(PRIM_LEG2_PWM_BASE,
                                          EPWM_TZ_DC_OUTPUT_B2,
                                          EPWM_TZ_EVENT_DCXL_HIGH);


        //
        // sets the ETZE bit to 1, to enable advanced actions on the PWM
        //
        EPWM_enableTripZoneAdvAction(PRIM_LEG1_PWM_BASE);
        EPWM_enableTripZoneAdvAction(PRIM_LEG2_PWM_BASE);

        //
        // first set all the TZCTLDCX registers to do nothing
        //
        EALLOW;
        HWREGH(PRIM_LEG1_PWM_BASE + EPWM_O_TZCTLDCA) = 0xFFFF;
        HWREGH(PRIM_LEG1_PWM_BASE + EPWM_O_TZCTLDCB) = 0xFFFF;
        HWREGH(PRIM_LEG2_PWM_BASE + EPWM_O_TZCTLDCA) = 0xFFFF;
        HWREGH(PRIM_LEG2_PWM_BASE + EPWM_O_TZCTLDCB) = 0xFFFF;
        EDIS;

        //
        // now describe the behavior in case when DCAEVT2 and
        //
        EPWM_setTripZoneAdvDigitalCompareActionA(PRIM_LEG1_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_U,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionB(PRIM_LEG1_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_D,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionA(PRIM_LEG2_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_D,
                                        EPWM_TZ_ADV_ACTION_LOW);

        EPWM_setTripZoneAdvDigitalCompareActionB(PRIM_LEG2_PWM_BASE,
                                        EPWM_TZ_ADV_ACTION_EVENT_DCxEVT2_U,
                                        EPWM_TZ_ADV_ACTION_LOW);

        //
        //    clear the cycle by cycle trip on zero and period
        //
        EPWM_selectCycleByCycleTripZoneClearEvent(PRIM_LEG1_PWM_BASE,
                                    EPWM_TZ_CBC_PULSE_CLR_CNTR_ZERO_PERIOD);
        EPWM_selectCycleByCycleTripZoneClearEvent(PRIM_LEG2_PWM_BASE,
                                    EPWM_TZ_CBC_PULSE_CLR_CNTR_ZERO_PERIOD);


    }
    else
    {
        //
        // not a valid option, do nothing
        //

    }

}
