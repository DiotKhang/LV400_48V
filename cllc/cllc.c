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

//
//--- System Related Globals ---
// Put the variables that are specific to control in the below section
// For example SFRA cannot run on CLA hence it must not be placed
// in the below section, the control verification using SFRA can only
// be carried out on the C28x
// Control Variables
//
#pragma SET_DATA_SECTION("controlVariables")

CLLC_Lab_EnumType CLLC_Lab;

CLLC_TripFlag_EnumType CLLC_tripFlag;

CLLC_PwmSwState_EnumType CLLC_pwmSwStateActive, CLLC_pwmSwState;

CLLC_PowerFlowState_EnumType CLLC_powerFlowStateActive, CLLC_powerFlowState;

CLLC_PrechargeState_EnumType CLLC_PrechargeState;

// CommandSentTo_AC_DC_EnumType commandSentTo_AC_DC;

CLLC_GI CLLC_gi;
float32_t CLLC_giOut;
float32_t CLLC_giError;
float32_t CLLC_giPartialComputedValue;

CLLC_GI CLLC_gv;
float32_t CLLC_gvOut;
float32_t CLLC_gvError;
float32_t CLLC_gvPartialComputedValue;

//
// Flags for clearing trips and closing the loop
//
volatile int32_t CLLC_closeGiLoop;
volatile int32_t CLLC_closeGvLoop;
volatile int32_t CLLC_clearTrip;

volatile float32_t CLLC_pwmFrequencyRef_Hz;
volatile float32_t CLLC_pwmFrequency_Hz;
volatile float32_t CLLC_pwmFrequencyPrev_Hz;

volatile float32_t CLLC_pwmPeriodRef_pu;
float32_t CLLC_pwmPeriod_pu;
float32_t CLLC_pwmPeriodSlewed_pu;
float32_t CLLC_pwmPeriodMin_pu;
float32_t CLLC_pwmPeriodMax_pu;
float32_t CLLC_pwmPeriodMax_ticks;
uint32_t CLLC_pwmPeriod_ticks;

//
// 1- Primary Side (PFC-Inv/Bus)
//
float32_t CLLC_iPrimSensed_Amps;
float32_t CLLC_iPrimSensed_pu;
float32_t CLLC_iPrimSensedOffset_pu;
float32_t CLLC_iPrimSensedCalIntercept_pu;
float32_t CLLC_iPrimSensedCalXvariable_pu;
EMAVG CLLC_iPrimSensedAvg_pu;

float32_t CLLC_iPrimTankSensed_Amps;
float32_t CLLC_iPrimTankSensed_pu;
float32_t CLLC_iPrimTankSensedOffset_pu;
float32_t CLLC_iPrimTankSensedCalIntercept_pu;
float32_t CLLC_iPrimTankSensedCalXvariable_pu;
EMAVG CLLC_iPrimTankSensedAvg_pu;

float32_t CLLC_vPrimSensed_Volts;
float32_t CLLC_vPrimSensed_pu;
float32_t CLLC_vPrimSensedOffset_pu;
EMAVG CLLC_vPrimSensedAvg_pu;

float32_t CLLC_vPrimRef_Volts;
float32_t CLLC_vPrimRef_pu;
float32_t CLLC_vPrimRefSlewed_pu;

volatile float32_t CLLC_pwmDutyPrimRef_pu;
float32_t CLLC_pwmDutyPrim_pu;
uint32_t CLLC_pwmDutyAPrim_ticks;
uint32_t CLLC_pwmDutyBPrim_ticks;

volatile float32_t CLLC_pwmDeadBandREDPrimRef_ns;
uint32_t CLLC_pwmDeadBandREDPrim_ticks;

volatile float32_t CLLC_pwmDeadBandFEDPrimRef_ns;
uint32_t CLLC_pwmDeadBandFEDPrim_ticks;

//
// 2-Secondary side (Battery)
//
float32_t CLLC_iSecSensed_Amps;
float32_t CLLC_iSecSensed_pu;
float32_t CLLC_iSecSensedOffset_pu;
float32_t CLLC_iSecSensedCalIntercept_pu;
float32_t CLLC_iSecSensedCalXvariable_pu;
EMAVG CLLC_iSecSensedAvg_pu;

volatile float32_t CLLC_iSecRef_Amps;
float32_t CLLC_iSecRef_pu;
float32_t CLLC_iSecRefSlewed_pu;

float32_t CLLC_vSecSensed_Volts;
float32_t CLLC_vSecSensed_pu;
float32_t CLLC_vSecSensedOffset_pu;

float32_t CLLC_vSecRef_Volts;
float32_t CLLC_vSecRef_pu;
float32_t CLLC_vSecRefSlewed_pu;
EMAVG CLLC_vSecSensedAvg_pu;

volatile float32_t CLLC_pwmDutySecRef_pu;
float32_t CLLC_pwmDutySec_pu;
uint32_t CLLC_pwmDutyASec_ticks;
uint32_t CLLC_pwmDutyBSec_ticks;

float32_t CLLC_pwmDeadBandREDSec_ns;
uint16_t CLLC_pwmDeadBandREDSec_ticks;

float32_t CLLC_pwmDeadBandFEDSec_ns;
uint16_t CLLC_pwmDeadbandFEDSec_ticks;

volatile float32_t CLLC_pwmPhaseShiftPrimSecRef_ns;
float32_t CLLC_pwmPhaseShiftPrimSec_ns;
int32_t CLLC_pwmPhaseShiftPrimSec_ticks;
int16_t CLLC_pwmPhaseShiftPrimSec_countDirection;


volatile uint16_t CLLC_pwmISRTrig_ticks;

volatile uint32_t CLLC_cla_task_counter;

volatile uint32_t CLLC_precharge_count;

void CLLC_runISR3(void)
{

    EMAVG_run(&CLLC_iSecSensedAvg_pu, CLLC_iSecSensed_pu);
    EMAVG_run(&CLLC_iPrimSensedAvg_pu, CLLC_iPrimSensed_pu);
    EMAVG_run(&CLLC_vSecSensedAvg_pu, CLLC_vSecSensed_pu);
    EMAVG_run(&CLLC_vPrimSensedAvg_pu, CLLC_vPrimSensed_pu);

    CLLC_vPrimSensed_Volts = CLLC_vPrimSensedAvg_pu.out *
                             CLLC_VPRIM_MAX_SENSE_VOLTS;
    CLLC_vSecSensed_Volts = CLLC_vSecSensedAvg_pu.out *
                             CLLC_VSEC_OPTIMAL_RANGE_VOLTS;
    CLLC_iPrimSensed_Amps = CLLC_iPrimSensedAvg_pu.out *
                             CLLC_IPRIM_MAX_SENSE_AMPS;
    CLLC_iSecSensed_Amps = CLLC_iSecSensedAvg_pu.out *
                            CLLC_ISEC_MAX_SENSE_AMPS;

    #if CLLC_CONTROL_MODE == CLLC_VOLTAGE_MODE

        #if CLLC_POWER_FLOW == CLLC_POWER_FLOW_PRIM_SEC
            CLLC_vSecRef_pu = CLLC_vSecRef_Volts /
                               CLLC_VSEC_OPTIMAL_RANGE_VOLTS;

            if((CLLC_vSecRef_pu - CLLC_vSecRefSlewed_pu) >
                (2.0 * CLLC_VOLTS_PER_SECOND_SLEW /
                        CLLC_VSEC_OPTIMAL_RANGE_VOLTS) *
                (1.0 / (float32_t)CLLC_ISR3_FREQUENCY_HZ))
            {
                CLLC_vSecRefSlewed_pu = CLLC_vSecRefSlewed_pu +
                        ((CLLC_VOLTS_PER_SECOND_SLEW /
                                CLLC_VSEC_OPTIMAL_RANGE_VOLTS) *
                      (1.0 / (float32_t)CLLC_ISR3_FREQUENCY_HZ));
            }
            else if((CLLC_vSecRef_pu - CLLC_vSecRefSlewed_pu) <
                    - (2.0 * CLLC_VOLTS_PER_SECOND_SLEW /
                            CLLC_VSEC_OPTIMAL_RANGE_VOLTS)
                    * (1.0 / (float32_t)CLLC_ISR3_FREQUENCY_HZ))
            {
                CLLC_vSecRefSlewed_pu = CLLC_vSecRefSlewed_pu -
                        ((CLLC_VOLTS_PER_SECOND_SLEW /
                                CLLC_VSEC_OPTIMAL_RANGE_VOLTS) *
                     (1.0 / (float32_t)CLLC_ISR3_FREQUENCY_HZ));
            }
            else
            {
                CLLC_vSecRefSlewed_pu = CLLC_vSecRef_pu;
            }
        #elif CLLC_POWER_FLOW == CLLC_POWER_FLOW_SEC_PRIM
            CLLC_vPrimRef_pu = CLLC_vPrimRef_Volts /
                    CLLC_VPRIM_MAX_SENSE_VOLTS;

            if((CLLC_vPrimRef_pu - CLLC_vPrimRefSlewed_pu) >
                (2.0 * CLLC_VOLTS_PER_SECOND_SLEW /
                        CLLC_VPRIM_MAX_SENSE_VOLTS)
                * (1.0 / (float32_t)CLLC_ISR3_FREQUENCY_HZ))
            {
                CLLC_vPrimRefSlewed_pu = CLLC_vPrimRefSlewed_pu +
                        ((CLLC_VOLTS_PER_SECOND_SLEW /
                                CLLC_VPRIM_MAX_SENSE_VOLTS) *
                      (1.0 / (float32_t)CLLC_ISR3_FREQUENCY_HZ));
            }
            else if((CLLC_vPrimRef_pu - CLLC_vPrimRefSlewed_pu) <
                    - (2.0 * CLLC_VOLTS_PER_SECOND_SLEW /
                            CLLC_VPRIM_MAX_SENSE_VOLTS)
                 * (1.0 / (float32_t)CLLC_ISR3_FREQUENCY_HZ))
            {
                CLLC_vPrimRefSlewed_pu = CLLC_vPrimRefSlewed_pu -
                        ((CLLC_VOLTS_PER_SECOND_SLEW /
                                CLLC_VPRIM_MAX_SENSE_VOLTS) *
                     (1.0 / (float32_t)CLLC_ISR3_FREQUENCY_HZ));
            }
            else
            {
                CLLC_vPrimRefSlewed_pu = CLLC_vPrimRef_pu;
            }
        #endif
    #else
        CLLC_iSecRef_pu = CLLC_iSecRef_Amps / CLLC_ISEC_MAX_SENSE_AMPS;

        if((CLLC_iSecRef_pu - CLLC_iSecRefSlewed_pu) >
            (2.0 * CLLC_AMPS_PER_SECOND_SLEW / CLLC_ISEC_MAX_SENSE_AMPS) *
            (1.0 / (float32_t)CLLC_ISR3_FREQUENCY_HZ))
        {
            CLLC_iSecRefSlewed_pu = CLLC_iSecRefSlewed_pu +
              ((CLLC_AMPS_PER_SECOND_SLEW / CLLC_ISEC_MAX_SENSE_AMPS) *
               (1.0 / (float32_t)CLLC_ISR3_FREQUENCY_HZ));
        }
        else if((CLLC_iSecRef_pu - CLLC_iSecRefSlewed_pu) <
             - (2.0 * CLLC_AMPS_PER_SECOND_SLEW /
                     CLLC_ISEC_MAX_SENSE_AMPS) *
               (1.0 / (float32_t)CLLC_ISR3_FREQUENCY_HZ))
        {
            CLLC_iSecRefSlewed_pu = CLLC_iSecRefSlewed_pu -
                 ((CLLC_AMPS_PER_SECOND_SLEW / CLLC_ISEC_MAX_SENSE_AMPS) *
                 (1.0 / (float32_t)CLLC_ISR3_FREQUENCY_HZ));
        }
        else
        {
            CLLC_iSecRefSlewed_pu = CLLC_iSecRef_pu;
        }
    #endif

    CLLC_calculatePWMDeadBandPrimTicks();

    CLLC_HAL_updatePWMDeadBandPrim(CLLC_pwmDeadBandREDPrim_ticks,
                                    CLLC_pwmDeadBandFEDPrim_ticks);

}

void CLLC_initGlobalVariables(void)
{
    #if CLLC_POWER_FLOW == CLLC_POWER_FLOW_SEC_PRIM
        CLLC_powerFlowStateActive.CLLC_PowerFlowState_Enum =
                CLLC_powerFlow_SecToPrim;
        CLLC_powerFlowState.CLLC_PowerFlowState_Enum =
                CLLC_powerFlow_SecToPrim;
    #else
        CLLC_powerFlowStateActive.CLLC_PowerFlowState_Enum =
                CLLC_powerFlow_PrimToSec;
        CLLC_powerFlowState.CLLC_PowerFlowState_Enum =
                CLLC_powerFlow_PrimToSec;
    #endif

    DCL_resetDF13(&CLLC_gi);
    #if CLLC_SEC_CONNECTED_IN_BATTERY_EMULATION_MODE == 1
        CLLC_gi.a1 = CLLC_GI2_2P2Z_A1;
        CLLC_gi.a2 = CLLC_GI2_2P2Z_A2;
        CLLC_gi.a3 = CLLC_GI2_2P2Z_A3;
        CLLC_gi.b0 = CLLC_GI2_2P2Z_B0;
        CLLC_gi.b1 = CLLC_GI2_2P2Z_B1;
        CLLC_gi.b2 = CLLC_GI2_2P2Z_B2;
        CLLC_gi.b3 = CLLC_GI2_2P2Z_B3;
    #else
        CLLC_gi.a1 = CLLC_GI1_2P2Z_A1;
        CLLC_gi.a2 = CLLC_GI1_2P2Z_A2;
        CLLC_gi.a3 = CLLC_GI1_2P2Z_A3;
        CLLC_gi.b0 = CLLC_GI1_2P2Z_B0;
        CLLC_gi.b1 = CLLC_GI1_2P2Z_B1;
        CLLC_gi.b2 = CLLC_GI1_2P2Z_B2;
        CLLC_gi.b3 = CLLC_GI1_2P2Z_B3;
    #endif

    DCL_resetDF13(&CLLC_gv);
    #if CLLC_POWER_FLOW == CLLC_POWER_FLOW_PRIM_SEC
        CLLC_gv.a1 = CLLC_GV1_2P2Z_A1;
        CLLC_gv.a2 = CLLC_GV1_2P2Z_A2;
        CLLC_gv.a3 = CLLC_GV1_2P2Z_A3;
        CLLC_gv.b0 = CLLC_GV1_2P2Z_B0;
        CLLC_gv.b1 = CLLC_GV1_2P2Z_B1;
        CLLC_gv.b2 = CLLC_GV1_2P2Z_B2;
        CLLC_gv.b3 = CLLC_GV1_2P2Z_B3;
    #elif CLLC_POWER_FLOW == CLLC_POWER_FLOW_SEC_PRIM
        CLLC_gv.a1 = CLLC_GV2_2P2Z_A1;
        CLLC_gv.a2 = CLLC_GV2_2P2Z_A2;
        CLLC_gv.a3 = CLLC_GV2_2P2Z_A3;
        CLLC_gv.b0 = CLLC_GV2_2P2Z_B0;
        CLLC_gv.b1 = CLLC_GV2_2P2Z_B1;
        CLLC_gv.b2 = CLLC_GV2_2P2Z_B2;
        CLLC_gv.b3 = CLLC_GV2_2P2Z_B3;
    #endif

//     DLOG_4CH_reset(&dLog1);
//     DLOG_4CH_config(&dLog1,
//                     &dVal1, &dVal2, &dVal3, &dVal4,
//                     dBuff1, dBuff2, dBuff3, dBuff4,
//                     100, 0.5, 1);

//     dlogTrigger = 0;

    EMAVG_reset(&CLLC_iSecSensedAvg_pu);
    EMAVG_config(&CLLC_iSecSensedAvg_pu, 0.01);

    EMAVG_reset(&CLLC_iPrimSensedAvg_pu);
    EMAVG_config(&CLLC_iPrimSensedAvg_pu, 0.01);

    EMAVG_reset(&CLLC_iPrimTankSensedAvg_pu);
    EMAVG_config(&CLLC_iPrimTankSensedAvg_pu, 0.01);

    EMAVG_reset(&CLLC_vPrimSensedAvg_pu);
    EMAVG_config(&CLLC_vPrimSensedAvg_pu, 0.01);

    EMAVG_reset(&CLLC_vSecSensedAvg_pu);
    EMAVG_config(&CLLC_vSecSensedAvg_pu, 0.01);

    CLLC_iPrimSensed_Amps = 0;
    CLLC_vPrimSensed_Volts = 0;
    CLLC_iSecSensed_Amps = 0;
    CLLC_vSecSensed_Volts = 0;

    CLLC_vSecRef_Volts = CLLC_VSEC_NOMINAL_VOLTS;
    CLLC_vSecRef_pu = CLLC_VSEC_NOMINAL_VOLTS /
                       CLLC_VSEC_OPTIMAL_RANGE_VOLTS;
    CLLC_vSecRefSlewed_pu = 0;

    CLLC_vPrimRef_Volts = CLLC_VPRIM_NOMINAL_VOLTS;
    CLLC_vPrimRef_pu = CLLC_VPRIM_NOMINAL_VOLTS /
                        CLLC_VPRIM_MAX_SENSE_VOLTS;

    CLLC_pwmPeriod_pu = (CLLC_MIN_PWM_SWITCHING_FREQUENCY_HZ /
                          CLLC_NOMINAL_PWM_SWITCHING_FREQUENCY_HZ);
    CLLC_pwmPeriodSlewed_pu = CLLC_pwmPeriod_pu;
    CLLC_pwmPeriodRef_pu = CLLC_pwmPeriod_pu;
    CLLC_pwmPeriodMin_pu = (CLLC_MIN_PWM_SWITCHING_FREQUENCY_HZ /
                            CLLC_MAX_PWM_SWITCHING_FREQUENCY_HZ);
    CLLC_pwmPeriodMax_ticks = CLLC_PWMSYSCLOCK_FREQ_HZ /
                                CLLC_MIN_PWM_SWITCHING_FREQUENCY_HZ;

    CLLC_pwmFrequencyRef_Hz = CLLC_NOMINAL_PWM_SWITCHING_FREQUENCY_HZ;
    CLLC_pwmFrequency_Hz = CLLC_NOMINAL_PWM_SWITCHING_FREQUENCY_HZ;
    CLLC_pwmFrequencyPrev_Hz = CLLC_pwmFrequency_Hz - 1.0;

    CLLC_pwmPhaseShiftPrimSec_ns = 81;
    CLLC_pwmPhaseShiftPrimSecRef_ns = 81;

    
    CLLC_pwmDeadBandREDPrimRef_ns = CLLC_PRIM_PWM_DEADBAND_RED_NS;
    CLLC_pwmDeadBandFEDPrimRef_ns = CLLC_PRIM_PWM_DEADBAND_FED_NS;

    CLLC_iPrimSensed_pu = 0;
    CLLC_iPrimTankSensed_pu = 0;
    CLLC_iPrimSensedOffset_pu = 0.5;
    CLLC_iPrimTankSensedOffset_pu = 0.5;
    CLLC_vPrimSensed_pu = 0;
    CLLC_vPrimSensedOffset_pu = 0;
    CLLC_iSecSensed_pu = 0;
    CLLC_iSecSensedOffset_pu = 0.5;
    CLLC_vSecSensed_pu = 0;
    CLLC_vSecSensedOffset_pu = 0;

    if(CLLC_powerFlowState.CLLC_PowerFlowState_Enum ==
            CLLC_powerFlow_PrimToSec)
    {
        CLLC_pwmDutyPrim_pu = 0.5;
        CLLC_pwmDutyPrimRef_pu = 0.5;
        CLLC_pwmDutySec_pu = 0.45;
        CLLC_pwmDutySecRef_pu = 0.45;
    }
    else if(CLLC_powerFlowState.CLLC_PowerFlowState_Enum ==
            CLLC_powerFlow_SecToPrim)
    {
        CLLC_pwmDutyPrim_pu = 0.45;
        CLLC_pwmDutyPrimRef_pu = 0.45;
        CLLC_pwmDutySec_pu = 0.5;
        CLLC_pwmDutySecRef_pu = 0.5;
    }

    CLLC_iSecSensedCalIntercept_pu = -0.00026;
    CLLC_iSecSensedCalXvariable_pu = 0.882981;

    CLLC_iPrimSensedCalIntercept_pu = -0.01934;
    CLLC_iPrimSensedCalXvariable_pu = 1.02331;

    CLLC_iPrimTankSensedCalIntercept_pu = 0.009197;
    CLLC_iPrimTankSensedCalXvariable_pu = 0.95455;

    CLLC_pwmSwState.CLLC_PwmSwState_Enum =
            CLLC_pwmSwState_synchronousRectification_active;
    CLLC_pwmSwStateActive.CLLC_PwmSwState_Enum =
            CLLC_pwmSwState_synchronousRectification_active;

//     commandSentTo_AC_DC.CommandSentTo_AC_DC_Enum = ac_dc_OFF;
    CLLC_tripFlag.CLLC_TripFlag_Enum = CLLC_noTrip;

//     slewSCIcommand = 0;
    CLLC_vPrimRef_Volts = 400;

    CLLC_closeGiLoop = 0;
    CLLC_closeGvLoop = 0;
    CLLC_clearTrip = 0;

    CLLC_cla_task_counter = 0;

}

void CLLC_updateBoardStatus(void)
{
    int16_t tripStatusRead;
    tripStatusRead = CLLC_HAL_readTripFlags();

    if(CLLC_tripFlag.CLLC_TripFlag_Enum == CLLC_noTrip)
    {
        if(tripStatusRead == (int16_t)CLLC_primOverCurrentTrip)
        {
            CLLC_tripFlag.CLLC_TripFlag_Enum = CLLC_primOverCurrentTrip;
        }
        else if(tripStatusRead == (int16_t)CLLC_secOverCurrentTrip)
        {
            CLLC_tripFlag.CLLC_TripFlag_Enum = CLLC_secOverCurrentTrip;
        }
        else if(tripStatusRead == (int16_t)CLLC_primTankOverCurrentTrip)
        {
            CLLC_tripFlag.CLLC_TripFlag_Enum = CLLC_primTankOverCurrentTrip;
        }
    }
}

void CLLC_setBuildLevelIndicatorVariable(void)
{
    #if CLLC_LAB == 1
        #if CLLC_CONTROL_RUNNING_ON == CLA_CORE
           CLLC_Lab.CLLC_Lab_Enum =
                    CLLC_Lab1_CLA;
        #else
           CLLC_Lab.CLLC_Lab_Enum =
                    CLLC_Lab1;
        #endif
    #elif CLLC_LAB == 2
        #if CLLC_CONTROL_RUNNING_ON == CLA_CORE
           CLLC_Lab.CLLC_Lab_Enum =
                    CLLC_Lab2_CLA;
        #else
           CLLC_Lab.CLLC_Lab_Enum =
                    CLLC_Lab2;
        #endif
    #elif CLLC_LAB == 3
        #if CLLC_CONTROL_RUNNING_ON == CLA_CORE
           CLLC_Lab.CLLC_Lab_Enum =
                    CLLC_Lab3_CLA;
        #else
           CLLC_Lab.CLLC_Lab_Enum =
                    CLLC_Lab3;
        #endif
    #elif CLLC_LAB == 4
        #if CLLC_CONTROL_RUNNING_ON == CLA_CORE
           CLLC_Lab.CLLC_Lab_Enum =
                    CLLC_Lab4_CLA;
        #else
           CLLC_Lab.CLLC_Lab_Enum =
                    CLLC_Lab4;
        #endif
    #elif LAB == 5
        #if CLLC_CONTROL_RUNNING_ON == CLA_CORE
           CLLC_Lab.CLLC_Lab_Enum =
                    CLLC_Lab5_CLA;
        #else
            CLLC_Lab.CLLC_Lab_Enum =
                   CLLC_Lab5;
        #endif
    #elif CLLC_LAB == 6
        #if CLLC_CONTROL_RUNNING_ON == CLA_CORE
           CLLC_Lab.CLLC_Lab_Enum =
                    CLLC_Lab6_CLA;
        #else
           CLLC_Lab.CLLC_Lab_Enum =
                    CLLC_Lab6;
        #endif
    #elif LAB == 7
       #if CLLC_CONTROL_RUNNING_ON == CLA_CORE
          CLLC_Lab.CLLC_Lab_Enum =
                   CLLC_Lab7_CLA;
       #else
          CLLC_Lab.CLLC_Lab_Enum =
                   CLLC_Lab7;
       #endif
    #elif LAB == 8
       #if CLLC_CONTROL_RUNNING_ON == CLA_CORE
          CLLC_Lab.CLLC_Lab_Enum =
                  CLLC_Lab8_CLA;
       #else
          CLLC_Lab.CLLC_Lab_Enum =
                  CLLC_Lab8;
   #endif
   #else

           CLLC_Lab.CLLC_Lab_Enum = CLLC_undefinedLab;
    #endif

//
//    #if INCR_BUILD == 1
//        #if SEC_CONNECTED_IN_BATTERY_EMULATION_MODE == 0
//            #if CLLC_CONTROL_RUNNING_ON == CLA_CORE
//                buildLevel.BuildLevel_Enum = openLoopCheck_CLA;
//            #else
//                buildLevel.BuildLevel_Enum = openLoopCheck;
//            #endif
//        #else
//                buildLevel..BuildLevel_Enum = undefinedState;
//        #endif
//    #elif INCR_BUILD == 2
//        #if CONTROL_MODE == VOLTAGE_MODE
//            #if SEC_CONNECTED_IN_BATTERY_EMULATION_MODE == 0
//                #if CLLC_CONTROL_RUNNING_ON == CLA_CORE
//                    buildLevel.BuildLevel_Enum =
//                            closedLoopCheck_Voltage_CLA;
//                #else
//                    buildLevel.BuildLevel_Enum =
//                            closedLoopCheck_Voltage;
//                #endif
//            #else
//                buildLevel.BuildLevel_Enum = undefinedState;
//            #endif
//
//        #elif CONTROL_MODE == CURRENT_MODE
//            #if SEC_CONNECTED_IN_BATTERY_EMULATION_MODE == 0
//                #if CLLC_CONTROL_RUNNING_ON == CLA_CORE
//                    buildLevel.BuildLevel_Enum =
//                            closedLoopCheck_Current_CLA;
//                #else
//                    buildLevel.BuildLevel_Enum =
//                            closedLoopCheck_Current;
//                #endif
//            #else
//                #if CLLC_CONTROL_RUNNING_ON == CLA_CORE
//                    buildLevel.BuildLevel_Enum =
//                            closedLoopCheck_Current_BatteryMode_CLA;
//                #else
//                    buildLevel.BuildLevel_Enum =
//                            closedLoopCheck_Current_BatteryMode;
//                #endif
//            #endif
//        #endif
//    #endif
//

}

void CLLC_changeSynchronousRectifierPwmBehavior(uint16_t powerFlow)
{
    if(CLLC_pwmSwState.CLLC_PwmSwState_Enum !=
            CLLC_pwmSwStateActive.CLLC_PwmSwState_Enum)
    {

        if(CLLC_pwmSwState.CLLC_PwmSwState_Enum ==
                CLLC_pwmSwState_synchronousRectification_fixedDuty)
        {
            CLLC_HAL_resetSynchronousRectifierTripAction(powerFlow);

            CLLC_pwmSwStateActive.CLLC_PwmSwState_Enum =
                    CLLC_pwmSwState_synchronousRectification_fixedDuty;

            //
            // the below is emperical value for this design
            // accounting for delay in digital isolators
            //

            CLLC_pwmPhaseShiftPrimSecRef_ns = 81;
        }
        else if(CLLC_pwmSwState.CLLC_PwmSwState_Enum ==
                CLLC_pwmSwState_synchronousRectification_active)
        {
           CLLC_HAL_setupSynchronousRectifierTripAction(powerFlow);

            CLLC_pwmSwStateActive.CLLC_PwmSwState_Enum =
                    CLLC_pwmSwState_synchronousRectification_active;

            //
            // CMPSS trip is cleared on PWM period and zero,
            // however this causes some delay in PWM coming out of trip
            // this is compensated by reducing the phase shift as needed
            // the below is emperical value for this design
            //
            CLLC_pwmPhaseShiftPrimSecRef_ns = 81;
        }
        else
        {

            CLLC_pwmSwStateActive.CLLC_PwmSwState_Enum =
                    CLLC_pwmSwState.CLLC_PwmSwState_Enum;

           //
           // do nothing, simply change the IOs which is done in the
            // routine outside this if statement
           //
        }

        CLLC_HAL_setupPWMpins(CLLC_pwmSwStateActive.CLLC_PwmSwState_Enum);
    }

}
