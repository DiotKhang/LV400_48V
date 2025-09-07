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

Lab_EnumType Lab;

TripFlag_EnumType tripFlag;

PwmSwState_EnumType pwmSwStateActive, pwmSwState;

PowerFlowState_EnumType powerFlowStateActive, powerFlowState;

// CommandSentTo_AC_DC_EnumType commandSentTo_AC_DC;

GI gi;
float32_t giOut;
float32_t giError;
float32_t giPartialComputedValue;

GI gv;
float32_t gvOut;
float32_t gvError;
float32_t gvPartialComputedValue;

//
// Flags for clearing trips and closing the loop
//
volatile int32_t closeGiLoop;
volatile int32_t closeGvLoop;
volatile int32_t clearTrip;

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
uint32_t countcheckISR2, countcheckISR1;
//
// 1- Primary Side (PFC-Inv/Bus)
//
float32_t iPrimSensed_Amps;
float32_t iPrimSensed_pu;
float32_t iPrimSensedOffset_pu;
float32_t iPrimSensedCalIntercept_pu;
float32_t iPrimSensedCalXvariable_pu;
EMAVG iPrimSensedAvg_pu;

float32_t iPrimTankSensed_Amps;
float32_t iPrimTankSensed_pu;
float32_t iPrimTankSensedOffset_pu;
float32_t iPrimTankSensedCalIntercept_pu;
float32_t iPrimTankSensedCalXvariable_pu;
EMAVG iPrimTankSensedAvg_pu;

float32_t vPrimSensed_Volts;
float32_t vPrimSensed_pu;
float32_t vPrimSensedOffset_pu;
EMAVG vPrimSensedAvg_pu;

float32_t vPrimRef_Volts;
float32_t vPrimRef_pu;
float32_t vPrimRefSlewed_pu;

volatile float32_t pwmDutyPrimRef_pu;
float32_t pwmDutyPrim_pu;
uint32_t pwmDutyAPrim_ticks;
uint32_t pwmDutyBPrim_ticks;

volatile float32_t pwmDeadBandREDPrimRef_ns;
uint32_t pwmDeadBandREDPrim_ticks;

volatile float32_t pwmDeadBandFEDPrimRef_ns;
uint32_t pwmDeadBandFEDPrim_ticks;

//
// 2-Secondary side (Battery)
//
float32_t iSecSensed_Amps;
float32_t iSecSensed_pu;
float32_t iSecSensedOffset_pu;
float32_t iSecSensedCalIntercept_pu;
float32_t iSecSensedCalXvariable_pu;
EMAVG iSecSensedAvg_pu;

volatile float32_t iSecRef_Amps;
float32_t iSecRef_pu;
float32_t iSecRefSlewed_pu;

float32_t vSecSensed_Volts;
float32_t vSecSensed_pu;
float32_t vSecSensedOffset_pu;

float32_t vSecRef_Volts;
float32_t vSecRef_pu;
float32_t vSecRefSlewed_pu;
EMAVG vSecSensedAvg_pu;

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


volatile uint16_t pwmISRTrig_ticks;

volatile uint32_t cla_task_counter;

void runISR3(void)
{

    EMAVG_run(&iSecSensedAvg_pu, iSecSensed_pu);
    EMAVG_run(&iPrimSensedAvg_pu, iPrimSensed_pu);
    EMAVG_run(&vSecSensedAvg_pu, vSecSensed_pu);
    EMAVG_run(&vPrimSensedAvg_pu, vPrimSensed_pu);

    vPrimSensed_Volts = vPrimSensedAvg_pu.out *
                             VPRIM_MAX_SENSE_VOLTS;
    vSecSensed_Volts = vSecSensedAvg_pu.out *
                             VSEC_OPTIMAL_RANGE_VOLTS;
    iPrimSensed_Amps = iPrimSensedAvg_pu.out *
                             IPRIM_MAX_SENSE_AMPS;
    iSecSensed_Amps = iSecSensedAvg_pu.out *
                            ISEC_MAX_SENSE_AMPS;

    #if CONTROL_MODE == VOLTAGE_MODE

        #if POWER_FLOW == POWER_FLOW_PRIM_SEC
            vSecRef_pu = vSecRef_Volts /
                               VSEC_OPTIMAL_RANGE_VOLTS;

            if((vSecRef_pu - vSecRefSlewed_pu) >
                (2.0 * VOLTS_PER_SECOND_SLEW /
                        VSEC_OPTIMAL_RANGE_VOLTS) *
                (1.0 / (float32_t)ISR3_FREQUENCY_HZ))
            {
                vSecRefSlewed_pu = vSecRefSlewed_pu +
                        ((VOLTS_PER_SECOND_SLEW /
                                VSEC_OPTIMAL_RANGE_VOLTS) *
                      (1.0 / (float32_t)ISR3_FREQUENCY_HZ));
            }
            else if((vSecRef_pu - vSecRefSlewed_pu) <
                    - (2.0 * VOLTS_PER_SECOND_SLEW /
                            VSEC_OPTIMAL_RANGE_VOLTS)
                    * (1.0 / (float32_t)ISR3_FREQUENCY_HZ))
            {
                vSecRefSlewed_pu = vSecRefSlewed_pu -
                        ((VOLTS_PER_SECOND_SLEW /
                                VSEC_OPTIMAL_RANGE_VOLTS) *
                     (1.0 / (float32_t)ISR3_FREQUENCY_HZ));
            }
            else
            {
                vSecRefSlewed_pu = vSecRef_pu;
            }
        #elif POWER_FLOW == POWER_FLOW_SEC_PRIM
            vPrimRef_pu = vPrimRef_Volts /
                    VPRIM_MAX_SENSE_VOLTS;

            if((vPrimRef_pu - vPrimRefSlewed_pu) >
                (2.0 * VOLTS_PER_SECOND_SLEW /
                        VPRIM_MAX_SENSE_VOLTS)
                * (1.0 / (float32_t)ISR3_FREQUENCY_HZ))
            {
                vPrimRefSlewed_pu = vPrimRefSlewed_pu +
                        ((VOLTS_PER_SECOND_SLEW /
                                VPRIM_MAX_SENSE_VOLTS) *
                      (1.0 / (float32_t)ISR3_FREQUENCY_HZ));
            }
            else if((vPrimRef_pu - vPrimRefSlewed_pu) <
                    - (2.0 * VOLTS_PER_SECOND_SLEW /
                            VPRIM_MAX_SENSE_VOLTS)
                 * (1.0 / (float32_t)ISR3_FREQUENCY_HZ))
            {
                vPrimRefSlewed_pu = vPrimRefSlewed_pu -
                        ((VOLTS_PER_SECOND_SLEW /
                                VPRIM_MAX_SENSE_VOLTS) *
                     (1.0 / (float32_t)ISR3_FREQUENCY_HZ));
            }
            else
            {
                vPrimRefSlewed_pu = vPrimRef_pu;
            }
        #endif
    #else
        iSecRef_pu = iSecRef_Amps / ISEC_MAX_SENSE_AMPS;

        if((iSecRef_pu - iSecRefSlewed_pu) >
            (2.0 * AMPS_PER_SECOND_SLEW / ISEC_MAX_SENSE_AMPS) *
            (1.0 / (float32_t)ISR3_FREQUENCY_HZ))
        {
            iSecRefSlewed_pu = iSecRefSlewed_pu +
              ((AMPS_PER_SECOND_SLEW / ISEC_MAX_SENSE_AMPS) *
               (1.0 / (float32_t)ISR3_FREQUENCY_HZ));
        }
        else if((iSecRef_pu - iSecRefSlewed_pu) <
             - (2.0 * AMPS_PER_SECOND_SLEW /
                     ISEC_MAX_SENSE_AMPS) *
               (1.0 / (float32_t)ISR3_FREQUENCY_HZ))
        {
            iSecRefSlewed_pu = iSecRefSlewed_pu -
                 ((AMPS_PER_SECOND_SLEW / ISEC_MAX_SENSE_AMPS) *
                 (1.0 / (float32_t)ISR3_FREQUENCY_HZ));
        }
        else
        {
            iSecRefSlewed_pu = iSecRef_pu;
        }
    #endif

    calculatePWMDeadBandPrimTicks();

    HAL_updatePWMDeadBandPrim(pwmDeadBandREDPrim_ticks,
                                    pwmDeadBandFEDPrim_ticks);

}

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

    DCL_resetDF13(&gi);
    #if SEC_CONNECTED_IN_BATTERY_EMULATION_MODE == 1
        gi.a1 = GI2_2P2Z_A1;
        gi.a2 = GI2_2P2Z_A2;
        gi.a3 = GI2_2P2Z_A3;
        gi.b0 = GI2_2P2Z_B0;
        gi.b1 = GI2_2P2Z_B1;
        gi.b2 = GI2_2P2Z_B2;
        gi.b3 = GI2_2P2Z_B3;
    #else
        gi.a1 = GI1_2P2Z_A1;
        gi.a2 = GI1_2P2Z_A2;
        gi.a3 = GI1_2P2Z_A3;
        gi.b0 = GI1_2P2Z_B0;
        gi.b1 = GI1_2P2Z_B1;
        gi.b2 = GI1_2P2Z_B2;
        gi.b3 = GI1_2P2Z_B3;
    #endif

    DCL_resetDF13(&gv);
    #if POWER_FLOW == POWER_FLOW_PRIM_SEC
        gv.a1 = GV1_2P2Z_A1;
        gv.a2 = GV1_2P2Z_A2;
        gv.a3 = GV1_2P2Z_A3;
        gv.b0 = GV1_2P2Z_B0;
        gv.b1 = GV1_2P2Z_B1;
        gv.b2 = GV1_2P2Z_B2;
        gv.b3 = GV1_2P2Z_B3;
    #elif POWER_FLOW == POWER_FLOW_SEC_PRIM
        gv.a1 = GV2_2P2Z_A1;
        gv.a2 = GV2_2P2Z_A2;
        gv.a3 = GV2_2P2Z_A3;
        gv.b0 = GV2_2P2Z_B0;
        gv.b1 = GV2_2P2Z_B1;
        gv.b2 = GV2_2P2Z_B2;
        gv.b3 = GV2_2P2Z_B3;
    #endif

//     DLOG_4CH_reset(&dLog1);
//     DLOG_4CH_config(&dLog1,
//                     &dVal1, &dVal2, &dVal3, &dVal4,
//                     dBuff1, dBuff2, dBuff3, dBuff4,
//                     100, 0.5, 1);

//     dlogTrigger = 0;

    EMAVG_reset(&iSecSensedAvg_pu);
    EMAVG_config(&iSecSensedAvg_pu, 0.01);

    EMAVG_reset(&iPrimSensedAvg_pu);
    EMAVG_config(&iPrimSensedAvg_pu, 0.01);

    EMAVG_reset(&iPrimTankSensedAvg_pu);
    EMAVG_config(&iPrimTankSensedAvg_pu, 0.01);

    EMAVG_reset(&vPrimSensedAvg_pu);
    EMAVG_config(&vPrimSensedAvg_pu, 0.01);

    EMAVG_reset(&vSecSensedAvg_pu);
    EMAVG_config(&vSecSensedAvg_pu, 0.01);

    iPrimSensed_Amps = 0;
    vPrimSensed_Volts = 0;
    iSecSensed_Amps = 0;
    vSecSensed_Volts = 0;

    vSecRef_Volts = VSEC_NOMINAL_VOLTS;
    vSecRef_pu = VSEC_NOMINAL_VOLTS /
                       VSEC_OPTIMAL_RANGE_VOLTS;
    vSecRefSlewed_pu = 0;

    vPrimRef_Volts = VPRIM_NOMINAL_VOLTS;
    vPrimRef_pu = VPRIM_NOMINAL_VOLTS /
                        VPRIM_MAX_SENSE_VOLTS;

    pwmPeriod_pu = (MIN_PWM_SWITCHING_FREQUENCY_HZ /
                          NOMINAL_PWM_SWITCHING_FREQUENCY_HZ);
    pwmPeriodSlewed_pu = pwmPeriod_pu;
    pwmPeriodRef_pu = pwmPeriod_pu;
    pwmPeriodMin_pu = (MIN_PWM_SWITCHING_FREQUENCY_HZ /
                            MAX_PWM_SWITCHING_FREQUENCY_HZ);
    pwmPeriodMax_ticks = PWMSYSCLOCK_FREQ_HZ /
                                MIN_PWM_SWITCHING_FREQUENCY_HZ;

    pwmFrequencyRef_Hz = NOMINAL_PWM_SWITCHING_FREQUENCY_HZ;
    pwmFrequency_Hz = NOMINAL_PWM_SWITCHING_FREQUENCY_HZ;
    pwmFrequencyPrev_Hz = pwmFrequency_Hz - 1.0;

    pwmPhaseShiftPrimSec_ns = 81;
    pwmPhaseShiftPrimSecRef_ns = 81;

    
    pwmDeadBandREDPrimRef_ns = PRIM_PWM_DEADBAND_RED_NS;
    pwmDeadBandFEDPrimRef_ns = PRIM_PWM_DEADBAND_FED_NS;

    iPrimSensed_pu = 0;
    iPrimTankSensed_pu = 0;
    iPrimSensedOffset_pu = 0.5;
    iPrimTankSensedOffset_pu = 0.5;
    vPrimSensed_pu = 0;
    vPrimSensedOffset_pu = 0;
    iSecSensed_pu = 0;
    iSecSensedOffset_pu = 0.5;
    vSecSensed_pu = 0;
    vSecSensedOffset_pu = 0;

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

    iSecSensedCalIntercept_pu = -0.00026;
    iSecSensedCalXvariable_pu = 0.882981;

    iPrimSensedCalIntercept_pu = -0.01934;
    iPrimSensedCalXvariable_pu = 1.02331;

    iPrimTankSensedCalIntercept_pu = 0.009197;
    iPrimTankSensedCalXvariable_pu = 0.95455;

    pwmSwState.PwmSwState_Enum =
            pwmSwState_synchronousRectification_active;
    pwmSwStateActive.PwmSwState_Enum =
            pwmSwState_synchronousRectification_active;

//     commandSentTo_AC_DC.CommandSentTo_AC_DC_Enum = ac_dc_OFF;
    tripFlag.TripFlag_Enum = noTrip;

//     slewSCIcommand = 0;
    vPrimRef_Volts = 400;

    closeGiLoop = 0;
    closeGvLoop = 0;
    clearTrip = 0;

    cla_task_counter = 0;

}

void updateBoardStatus(void)
{
    int16_t tripStatusRead;
    tripStatusRead = HAL_readTripFlags();

    if(tripFlag.TripFlag_Enum == noTrip)
    {
        if(tripStatusRead == (int16_t)primOverCurrentTrip)
        {
            tripFlag.TripFlag_Enum = primOverCurrentTrip;
        }
        else if(tripStatusRead == (int16_t)secOverCurrentTrip)
        {
            tripFlag.TripFlag_Enum = secOverCurrentTrip;
        }
        else if(tripStatusRead == (int16_t)primTankOverCurrentTrip)
        {
            tripFlag.TripFlag_Enum = primTankOverCurrentTrip;
        }
    }
}

void setBuildLevelIndicatorVariable(void)
{
    #if LAB == 1
        #if CONTROL_RUNNING_ON == CLA_CORE
           Lab.Lab_Enum =
                    Lab1_CLA;
        #else
           Lab.Lab_Enum =
                    Lab1;
        #endif
    #elif LAB == 2
        #if CONTROL_RUNNING_ON == CLA_CORE
           Lab.Lab_Enum =
                    Lab2_CLA;
        #else
           Lab.Lab_Enum =
                    Lab2;
        #endif
    #elif LAB == 3
        #if CONTROL_RUNNING_ON == CLA_CORE
           Lab.Lab_Enum =
                    Lab3_CLA;
        #else
           Lab.Lab_Enum =
                    Lab3;
        #endif
    #elif LAB == 4
        #if CONTROL_RUNNING_ON == CLA_CORE
           Lab.Lab_Enum =
                    Lab4_CLA;
        #else
           Lab.Lab_Enum =
                    Lab4;
        #endif
    #elif LAB == 5
        #if CONTROL_RUNNING_ON == CLA_CORE
           Lab.Lab_Enum =
                    Lab5_CLA;
        #else
            Lab.Lab_Enum =
                    Lab5;
        #endif
    #elif LAB == 6
        #if CONTROL_RUNNING_ON == CLA_CORE
           Lab.Lab_Enum =
                    Lab6_CLA;
        #else
           Lab.Lab_Enum =
                    Lab6;
        #endif
    #elif LAB == 7
       #if CONTROL_RUNNING_ON == CLA_CORE
          Lab.Lab_Enum =
                   Lab7_CLA;
       #else
          Lab.Lab_Enum =
                   Lab7;
       #endif
    #elif LAB == 8
       #if CONTROL_RUNNING_ON == CLA_CORE
          Lab.Lab_Enum =
                  Lab8_CLA;
       #else
          Lab.Lab_Enum =
                  Lab8;
   #endif
   #else

           Lab.Lab_Enum = undefinedLab;
    #endif

//
//    #if INCR_BUILD == 1
//        #if SEC_CONNECTED_IN_BATTERY_EMULATION_MODE == 0
//            #if CONTROL_RUNNING_ON == CLA_CORE
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
//                #if CONTROL_RUNNING_ON == CLA_CORE
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
//                #if CONTROL_RUNNING_ON == CLA_CORE
//                    buildLevel.BuildLevel_Enum =
//                            closedLoopCheck_Current_CLA;
//                #else
//                    buildLevel.BuildLevel_Enum =
//                            closedLoopCheck_Current;
//                #endif
//            #else
//                #if CONTROL_RUNNING_ON == CLA_CORE
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

