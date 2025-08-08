//#############################################################################
//
// FILE:   cllc.c
//
// TITLE: This is the solution file.
//
//#############################################################################

//*****************************************************************************
// the includes
//*****************************************************************************

#include "cllc.h"

PowerFlowState_EnumType powerFlowStateActive, powerFlowState;

volatile float32_t pwmFrequencyRef_Hz;
volatile float32_t pwmFrequency_Hz;
volatile float32_t pwmFrequencyPrev_Hz;

volatile float32_t pwmPeriodRef_pu;
float32_t pwmPeriod_pu;
float32_t pwmPeriodSlewed_pu;
float32_t pwmPeriodMin_pu;
float32_t pwmPeriodMax_pu;
float32_t pwmPeriodMax_ticks;
uint32_t pwmPeriod_ticks;

// //
// // 1- Primary Side (PFC-Inv/Bus)
// //
// float32_t iPrimSensed_Amps;
// float32_t iPrimSensed_pu;
// float32_t iPrimSensedOffset_pu;
// float32_t iPrimSensedCalIntercept_pu;
// float32_t iPrimSensedCalXvariable_pu;
// EMAVG iPrimSensedAvg_pu;

// float32_t iPrimTankSensed_Amps;
// float32_t iPrimTankSensed_pu;
// float32_t iPrimTankSensedOffset_pu;
// float32_t iPrimTankSensedCalIntercept_pu;
// float32_t iPrimTankSensedCalXvariable_pu;
// EMAVG iPrimTankSensedAvg_pu;

// float32_t vPrimSensed_Volts;
// float32_t vPrimSensed_pu;
// float32_t vPrimSensedOffset_pu;
// EMAVG vPrimSensedAvg_pu;

// float32_t vPrimRef_Volts;
// float32_t vPrimRef_pu;
// float32_t vPrimRefSlewed_pu;

volatile float32_t pwmDutyPrimRef_pu;
float32_t pwmDutyPrim_pu;
uint32_t pwmDutyAPrim_ticks;
uint32_t pwmDutyBPrim_ticks;

volatile float32_t pwmDeadBandREDPrimRef_ns;
uint32_t pwmDeadBandREDPrim_ticks;

volatile float32_t pwmDeadBandFEDPrimRef_ns;
uint32_t pwmDeadBandFEDPrim_ticks;

// //
// // 2-Secondary side (Battery)
// //
// float32_t iSecSensed_Amps;
// float32_t iSecSensed_pu;
// float32_t iSecSensedOffset_pu;
// float32_t iSecSensedCalIntercept_pu;
// float32_t iSecSensedCalXvariable_pu;
// EMAVG iSecSensedAvg_pu;

// volatile float32_t iSecRef_Amps;
// float32_t iSecRef_pu;
// float32_t iSecRefSlewed_pu;

// float32_t vSecSensed_Volts;
// float32_t vSecSensed_pu;
// float32_t vSecSensedOffset_pu;

// float32_t vSecRef_Volts;
// float32_t vSecRef_pu;
// float32_t vSecRefSlewed_pu;
// EMAVG vSecSensedAvg_pu;

volatile float32_t pwmDutySecRef_pu;
float32_t pwmDutySec_pu;
uint32_t pwmDutyASec_ticks;
uint32_t pwmDutyBSec_ticks;

float32_t pwmDeadBandREDSec_ns;
uint16_t pwmDeadBandREDSec_ticks;

float32_t pwmDeadBandFEDSec_ns;
uint16_t pwmDeadbandFEDSec_ticks;

volatile float32_t pwmPhaseShiftPrimSecRef_ns;
float32_t pwmPhaseShiftPrimSec_ns;
int32_t pwmPhaseShiftPrimSec_ticks;
int16_t pwmPhaseShiftPrimSec_countDirection;


// volatile uint16_t pwmISRTrig_ticks;

// volatile uint32_t cla_task_counter;

void initGlobalVariables(void)
{
    #if POWER_FLOW == POWER_FLOW_SEC_PRIM
        powerFlowStateActive.PowerFlowState_Enum =
                powerFlow_SecToPrim;
        powerFlowState.PowerFlowState_Enum =
                powerFlow_SecToPrim;
    #else
        powerFlowStateActive.PowerFlowState_Enum =
                powerFlow_PrimToSec;
        powerFlowState.PowerFlowState_Enum =
                powerFlow_PrimToSec;
    #endif

    // EMAVG_reset(&iSecSensedAvg_pu);
    // EMAVG_config(&iSecSensedAvg_pu, 0.01);

    // EMAVG_reset(&iPrimSensedAvg_pu);
    // EMAVG_config(&iPrimSensedAvg_pu, 0.01);

    // EMAVG_reset(&iPrimTankSensedAvg_pu);
    // EMAVG_config(&iPrimTankSensedAvg_pu, 0.01);

    // EMAVG_reset(&vPrimSensedAvg_pu);
    // EMAVG_config(&vPrimSensedAvg_pu, 0.01);

    // EMAVG_reset(&vSecSensedAvg_pu);
    // EMAVG_config(&vSecSensedAvg_pu, 0.01);

    // iPrimSensed_Amps = 0;
    // vPrimSensed_Volts = 0;
    // iSecSensed_Amps = 0;
    // vSecSensed_Volts = 0;

    // vSecRef_Volts = VSEC_NOMINAL_VOLTS;
    // vSecRef_pu = VSEC_NOMINAL_VOLTS /
    //                    VSEC_OPTIMAL_RANGE_VOLTS;
    // vSecRefSlewed_pu = 0;

    // vPrimRef_Volts = VPRIM_NOMINAL_VOLTS;
    // vPrimRef_pu = VPRIM_NOMINAL_VOLTS /
    //                     VPRIM_MAX_SENSE_VOLTS;

    // pwmPeriod_pu = (MIN_PWM_SWITCHING_FREQUENCY_HZ /
    //                       NOMINAL_PWM_SWITCHING_FREQUENCY_HZ);
    // pwmPeriodSlewed_pu = pwmPeriod_pu;
    // pwmPeriodRef_pu = pwmPeriod_pu;
    // pwmPeriodMin_pu = (MIN_PWM_SWITCHING_FREQUENCY_HZ /
    //                         MAX_PWM_SWITCHING_FREQUENCY_HZ);
    // pwmPeriodMax_ticks = PWMSYSCLOCK_FREQ_HZ /
    //                             MIN_PWM_SWITCHING_FREQUENCY_HZ;

    // pwmFrequencyRef_Hz = NOMINAL_PWM_SWITCHING_FREQUENCY_HZ;
    // pwmFrequency_Hz = NOMINAL_PWM_SWITCHING_FREQUENCY_HZ;
    // pwmFrequencyPrev_Hz = pwmFrequency_Hz - 1.0;

    // pwmPhaseShiftPrimSec_ns = 81;
    // pwmPhaseShiftPrimSecRef_ns = 81;

    
    // pwmDeadBandREDPrimRef_ns = PRIM_PWM_DEADBAND_RED_NS;
    // pwmDeadBandFEDPrimRef_ns = PRIM_PWM_DEADBAND_FED_NS;

    // iPrimSensed_pu = 0;
    // iPrimTankSensed_pu = 0;
    // iPrimSensedOffset_pu = 0.5;
    // iPrimTankSensedOffset_pu = 0.5;
    // vPrimSensed_pu = 0;
    // vPrimSensedOffset_pu = 0;
    // iSecSensed_pu = 0;
    // iSecSensedOffset_pu = 0.5;
    // vSecSensed_pu = 0;
    // vSecSensedOffset_pu = 0;

    if(powerFlowState.PowerFlowState_Enum ==
            powerFlow_PrimToSec)
    {
        pwmDutyPrim_pu = 0.5;
        pwmDutyPrimRef_pu = 0.5;
        pwmDutySec_pu = 0.45;
        pwmDutySecRef_pu = 0.45;
    }
    else if(powerFlowState.PowerFlowState_Enum ==
            powerFlow_SecToPrim)
    {
        pwmDutyPrim_pu = 0.45;
        pwmDutyPrimRef_pu = 0.45;
        pwmDutySec_pu = 0.5;
        pwmDutySecRef_pu = 0.5;
    }

    // iSecSensedCalIntercept_pu = -0.00026;
    // iSecSensedCalXvariable_pu = 0.882981;

    // iPrimSensedCalIntercept_pu = -0.01934;
    // iPrimSensedCalXvariable_pu = 1.02331;

    // iPrimTankSensedCalIntercept_pu = 0.009197;
    // iPrimTankSensedCalXvariable_pu = 0.95455;

    // pwmSwState.PwmSwState_Enum =
    //         pwmSwState_synchronousRectification_active;
    // pwmSwStateActive.PwmSwState_Enum =
    //         pwmSwState_synchronousRectification_active;

    // tripFlag.TripFlag_Enum = noTrip;

    // slewSCIcommand = 0;
    // vPrimRef_Volts = 400;

}