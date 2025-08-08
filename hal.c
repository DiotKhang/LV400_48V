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
}

void HAL_disablePWMClkCounting(void)
{
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
}

void HAL_enablePWMClkCounting(void)
{
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
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