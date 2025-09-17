//#############################################################################
//
// FILE:  settings.h
//
// TITLE: This is the settings.h file, which contains all the project level
//        settings, in case of powerSUITE , powerSUITE related settings are
//        in this file and the other settings are located in user_settings.h.
//        The User section is not over-written by powerSUITE
//
//#############################################################################
#ifndef _PROJSETTINGS_H
#define _PROJSETTINGS_H

#ifdef __cplusplus

extern "C" {
#endif

//#############################################################################
//
// Includes
//
//#############################################################################
#include <stdint.h>

//#############################################################################
//
// Macro Definitions
//
//#############################################################################
#ifndef C2000_IEEE754_TYPES
    #define C2000_IEEE754_TYPES
    #ifdef __TI_EABI__
        typedef float         float32_t;
        typedef double        float64_t;
    #else // TI COFF
        typedef float         float32_t;
        typedef long double   float64_t;
    #endif // __TI_EABI__
#endif // C2000_IEEE754_TYPES


//
// Defines
//

//
// Device Related Defines
//
#define CPU_SYS_CLOCK_FREQ_HZ ((float32_t)100*1000000)
#define PWMSYSCLOCK_FREQ_HZ   ((float32_t)100*1000000)
#define ECAPSYSCLOCK_FREQ_HZ  ((float32_t)100*1000000)

//
// Project Options
//

//
// CONTROL MODE , voltage or current
// 1 -> Voltage
// 2 -> Current
//
#define VOLTAGE_MODE 1
#define CURRENT_MODE 2

//
// POWER FLOW ,
// 1 -> PRIM to SEC
// 2 -> SEC to PRIM
//
#define POWER_FLOW_PRIM_SEC 1
#define POWER_FLOW_SEC_PRIM 2
#define POWER_FLOW_TRANSTION_STAGE 3

//
// PROTECTION  ,
// 0 -> DISABLED
// 1 -> ENABLED
//
#define PROTECTION_ENABLED 1
#define PROTECTION_DISABLED 0

//
// BUILD
// 1 ->  Open Loop Check
// 2 ->  Closed Loop Check
//
#define OPEN_LOOP_BUILD 1
#define CLOSED_LOOP_BUILD 2

//
// TEST, (which side is output depends on power flow)
// 0 ->  Test with Res load at the output
// 1 ->  Test with Res Load and Voltage source connected at output
//
#define TEST_SETUP_RES_LOAD 0
#define TEST_SETUP_EMULATED_BATTERY 1

//
// CORE running the control loop
// 1 -> C28x
// 2 -> CLA
//
#ifndef C28x_CORE
#define C28x_CORE 1
#define CLA_CORE 2
#endif

//
// SFRA Options
// 0 -> disabled
// 1 -> Current
// 2 -> Voltage
//
#define SFRA_DISABLED 0
#define SFRA_CURRENT 1
#define SFRA_VOLTAGE 2

//
// SFRA injection amplitude, use higher injection in open loop  because plant
// response is low
//
#define SFRA_INJECTION_AMPLITUDE_LEVEL1 0.001
#define SFRA_INJECTION_AMPLITUDE_LEVEL2 0.01
#define SFRA_INJECTION_AMPLITUDE_LEVEL3 0.015

//
// CLLC LAB
// Power Flow Prim -> Sec
// 1 -> Open loop check for PWM drivers,
// 2 -> Open loop check for PWM drivers with protection,
// 3 -> Closed loop check with resistive load, voltage loop,
// 4 -> Closed loop check with resistive load, current loop
// 5 -> Closed loop check with battery emulated, current loop
// Power Flow Sec -> Prim
// 6 -> Open loop check for PWM driver,
// 7 -> Open loop check for PWM driver with protection,
// 8 -> Closed loop voltage with resistive load
//

#define LAB 2

#if LAB == 1
#define CONTROL_RUNNING_ON 1
#define POWER_FLOW POWER_FLOW_PRIM_SEC
#define INCR_BUILD OPEN_LOOP_BUILD
#define TEST_SETUP TEST_SETUP_RES_LOAD
#define PROTECTION PROTECTION_DISABLED
#define SFRA_TYPE  0
#define SFRA_AMPLITUDE (float32_t)SFRA_INJECTION_AMPLITUDE_LEVEL2
#endif

#if LAB == 2
#define CONTROL_RUNNING_ON 1
#define POWER_FLOW POWER_FLOW_PRIM_SEC
#define INCR_BUILD OPEN_LOOP_BUILD
#define TEST_SETUP TEST_SETUP_RES_LOAD
#define PROTECTION PROTECTION_ENABLED
#define SFRA_TYPE  0
#define SFRA_AMPLITUDE (float32_t)SFRA_INJECTION_AMPLITUDE_LEVEL2
#endif

#if LAB == 3
#define CONTROL_RUNNING_ON 1
#define POWER_FLOW POWER_FLOW_PRIM_SEC
#define INCR_BUILD CLOSED_LOOP_BUILD
#define CONTROL_MODE VOLTAGE_MODE
#define TEST_SETUP TEST_SETUP_RES_LOAD
#define PROTECTION PROTECTION_ENABLED
#define SFRA_TYPE  0
#define SFRA_AMPLITUDE (float32_t)SFRA_INJECTION_AMPLITUDE_LEVEL1
#endif

#if LAB == 4
#define CONTROL_RUNNING_ON 1
#define POWER_FLOW POWER_FLOW_PRIM_SEC
#define INCR_BUILD CLOSED_LOOP_BUILD
#define CONTROL_MODE CURRENT_MODE
#define TEST_SETUP TEST_SETUP_RES_LOAD
#define PROTECTION PROTECTION_ENABLED
#define SFRA_TYPE  0
#define SFRA_AMPLITUDE (float32_t)SFRA_INJECTION_AMPLITUDE_LEVEL1
#endif

#if LAB == 5
#define CONTROL_RUNNING_ON 1
#define POWER_FLOW POWER_FLOW_PRIM_SEC
#define INCR_BUILD CLOSED_LOOP_BUILD
#define CONTROL_MODE CURRENT_MODE
#define TEST_SETUP TEST_SETUP_EMULATED_BATTERY
#define PROTECTION PROTECTION_ENABLED
#define SFRA_TYPE  0
#define SFRA_AMPLITUDE (float32_t)SFRA_INJECTION_AMPLITUDE_LEVEL1
#endif


#if LAB == 6
#define CONTROL_RUNNING_ON 1
#define POWER_FLOW POWER_FLOW_SEC_PRIM
#define INCR_BUILD OPEN_LOOP_BUILD
#define CONTROL_MODE VOLTAGE_MODE
#define TEST_SETUP TEST_SETUP_RES_LOAD
#define PROTECTION PROTECTION_DISABLED
#define SFRA_TYPE  0
#define SFRA_AMPLITUDE (float32_t)SFRA_INJECTION_AMPLITUDE_LEVEL2
#endif

#if LAB == 7
#define CONTROL_RUNNING_ON 1
#define POWER_FLOW POWER_FLOW_SEC_PRIM
#define INCR_BUILD OPEN_LOOP_BUILD
#define CONTROL_MODE VOLTAGE_MODE
#define TEST_SETUP TEST_SETUP_RES_LOAD
#define PROTECTION PROTECTION_ENABLED
#define SFRA_TYPE  0
#define SFRA_AMPLITUDE (float32_t)SFRA_INJECTION_AMPLITUDE_LEVEL2
#endif

#if LAB == 8
#define CONTROL_RUNNING_ON 1
#define POWER_FLOW POWER_FLOW_SEC_PRIM
#define INCR_BUILD CLOSED_LOOP_BUILD
#define CONTROL_MODE VOLTAGE_MODE
#define TEST_SETUP TEST_SETUP_RES_LOAD
#define PROTECTION PROTECTION_ENABLED
#define SFRA_TYPE  0
#define SFRA_AMPLITUDE (float32_t)SFRA_INJECTION_AMPLITUDE_LEVEL1
#endif


#define ISR1_RUNNING_ON CONTROL_RUNNING_ON

#define ISR2_FREQUENCY_HZ ((float32_t)100000)
#define ISR3_FREQUENCY_HZ ((float32_t)10000)
#define SFRA_ISR_FREQ_HZ       ISR2_FREQUENCY_HZ

//
// Power Stage Related Values
//
#define NOMINAL_PWM_SWITCHING_FREQUENCY_HZ  ((float32_t)500.8*1000)
#define MAX_PWM_SWITCHING_FREQUENCY_HZ ((float32_t)700*1000)
#define MIN_PWM_SWITCHING_FREQUENCY_HZ ((float32_t)300*1000)

#define PRIM_PWM_DEADBAND_RED_NS ((float32_t)102.3)
#define PRIM_PWM_DEADBAND_FED_NS ((float32_t)102.3)
#define SEC_PWM_DEADBAND_RED_NS  ((float32_t)102)
#define SEC_PWM_DEADBAND_FED_NS  ((float32_t)102)

#define VPRIM_MAX_SENSE_VOLTS    ((float32_t)661.49)
#define VSEC_MAX_SENSE_VOLTS     ((float32_t)480)
#define VSEC_OPTIMAL_RANGE_VOLTS ((float32_t)450)
#define IPRIM_MAX_SENSE_AMPS    ((float32_t)34.375)
#define ISEC_MAX_SENSE_AMPS     ((float32_t)42.438)
#define IPRIM_TANK_MAX_SENSE_AMPS ((float32_t)34.375)
#define ISEC_TANK_MAX_SENSE_AMPS ((float32_t)42.375)

#define VSEC_NOMINAL_VOLTS ((float32_t)300)
#define VPRIM_NOMINAL_VOLTS ((float32_t)400)

#define IPRIM_TRIP_LIMIT_AMPS ((float32_t)30)
#define ISEC_TRIP_LIMIT_AMPS  ((float32_t)40)
#define IPRIM_TANK_TRIP_LIMIT_AMPS ((float32_t)30)
#define ISEC_TANK_TRIP_LIMIT_AMPS  ((float32_t)40)

//
// Control Loop Design
//

//
// LAB3
//
#define GV1_2P2Z_A1    (float32_t) -1.7284895037
#define GV1_2P2Z_A2    (float32_t) 0.7284895037
#define GV1_2P2Z_A3    (float32_t) 0.0000000000
#define GV1_2P2Z_B0    (float32_t) 4.8280130584
#define GV1_2P2Z_B1    (float32_t) 0.1493277469
#define GV1_2P2Z_B2    (float32_t) -4.6786792593
#define GV1_2P2Z_B3    (float32_t) 0.0000000000

//
// LAB4
//
#define GI1_2P2Z_A1    (float32_t) -1.8277396009
#define GI1_2P2Z_A2    (float32_t) 0.8277396009
#define GI1_2P2Z_A3    (float32_t) 0.0000000000
#define GI1_2P2Z_B0    (float32_t) 1.2500036172
#define GI1_2P2Z_B1    (float32_t) 0.2153188876
#define GI1_2P2Z_B2    (float32_t) -1.0346715071
#define GI1_2P2Z_B3    (float32_t) 0.0000000000

//
// LAB5 
//
#define GI2_2P2Z_A1    (float32_t) 0.0341879720
#define GI2_2P2Z_A2    (float32_t) -0.7668017816
#define GI2_2P2Z_A3    (float32_t) -0.2673861903
#define GI2_2P2Z_B0    (float32_t) 1.3436620732
#define GI2_2P2Z_B1    (float32_t) 0.3459370813
#define GI2_2P2Z_B2    (float32_t) -0.7200660800
#define GI2_2P2Z_B3    (float32_t) -0.2790608258

//
// LAB8 
//
#define GV2_2P2Z_A1    (float32_t) -0.4829060140
#define GV2_2P2Z_A2    (float32_t) -0.5170939860
#define GV2_2P2Z_A3    (float32_t) 0.0000000000
#define GV2_2P2Z_B0    (float32_t) 1.3436620732
#define GV2_2P2Z_B1    (float32_t) -0.3488624959
#define GV2_2P2Z_B2    (float32_t) -0.5396713815
#define GV2_2P2Z_B3    (float32_t) 0.0000000000

//=============================================================================
// User code settings file
//=============================================================================
#include "cllc_user_settings.h"

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif //_PROJSETTINGS_H
