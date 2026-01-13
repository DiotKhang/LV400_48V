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
#define CLLC_CPU_SYS_CLOCK_FREQ_HZ ((float32_t)120*1000000)
#define CLLC_PWMSYSCLOCK_FREQ_HZ   ((float32_t)120*1000000)
#define CLLC_ECAPSYSCLOCK_FREQ_HZ  ((float32_t)120*1000000)

//
// Project Options
//

//
// CONTROL MODE , voltage or current
// 1 -> Voltage
// 2 -> Current
//
#define CLLC_VOLTAGE_MODE 1
#define CLLC_CURRENT_MODE 2

//
// POWER FLOW ,
// 1 -> PRIM to SEC
// 2 -> SEC to PRIM
//
#define CLLC_POWER_FLOW_PRIM_SEC 1
#define CLLC_POWER_FLOW_SEC_PRIM 2
#define CLLC_POWER_FLOW_TRANSTION_STAGE 3

//
// PROTECTION  ,
// 0 -> DISABLED
// 1 -> ENABLED
//
#define CLLC_PROTECTION_ENABLED 1
#define CLLC_PROTECTION_DISABLED 0

//
// BUILD
// 1 ->  Open Loop Check
// 2 ->  Closed Loop Check
//
#define CLLC_OPEN_LOOP_BUILD 1
#define CLLC_CLOSED_LOOP_BUILD 2

//
// TEST, (which side is output depends on power flow)
// 0 ->  Test with Res load at the output
// 1 ->  Test with Res Load and Voltage source connected at output
//
#define CLLC_TEST_SETUP_RES_LOAD 0
#define CLLC_TEST_SETUP_EMULATED_BATTERY 1

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
#define CLLC_SFRA_DISABLED 0
#define CLLC_SFRA_CURRENT 1
#define CLLC_SFRA_VOLTAGE 2

//
// SFRA injection amplitude, use higher injection in open loop  because plant
// response is low
//
#define CLLC_SFRA_INJECTION_AMPLITUDE_LEVEL1 0.001
#define CLLC_SFRA_INJECTION_AMPLITUDE_LEVEL2 0.01
#define CLLC_SFRA_INJECTION_AMPLITUDE_LEVEL3 0.015

//
// CLLLC LAB
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

#define CLLC_LAB 1

#if CLLC_LAB == 1
#define CLLC_CONTROL_RUNNING_ON C28x_CORE
#define CLLC_POWER_FLOW CLLC_POWER_FLOW_PRIM_SEC
#define CLLC_INCR_BUILD CLLC_OPEN_LOOP_BUILD
#define CLLC_TEST_SETUP CLLC_TEST_SETUP_RES_LOAD
#define CLLC_PROTECTION CLLC_PROTECTION_DISABLED
#define CLLC_SFRA_TYPE  0
#define CLLC_SFRA_AMPLITUDE (float32_t)CLLC_SFRA_INJECTION_AMPLITUDE_LEVEL2
#endif

#if CLLC_LAB == 2
#define CLLC_CONTROL_RUNNING_ON 1
#define CLLC_POWER_FLOW CLLC_POWER_FLOW_PRIM_SEC
#define CLLC_INCR_BUILD CLLC_OPEN_LOOP_BUILD
#define CLLC_TEST_SETUP CLLC_TEST_SETUP_RES_LOAD
#define CLLC_PROTECTION CLLC_PROTECTION_ENABLED
#define CLLC_SFRA_TYPE  0
#define CLLC_SFRA_AMPLITUDE (float32_t)CLLC_SFRA_INJECTION_AMPLITUDE_LEVEL2
#endif

#if CLLC_LAB == 3
#define CLLC_CONTROL_RUNNING_ON 1
#define CLLC_POWER_FLOW CLLC_POWER_FLOW_PRIM_SEC
#define CLLC_INCR_BUILD CLLC_CLOSED_LOOP_BUILD
#define CLLC_CONTROL_MODE CLLC_VOLTAGE_MODE
#define CLLC_TEST_SETUP CLLC_TEST_SETUP_RES_LOAD
#define CLLC_PROTECTION CLLC_PROTECTION_ENABLED
#define CLLC_SFRA_TYPE  0
#define CLLC_SFRA_AMPLITUDE (float32_t)CLLC_SFRA_INJECTION_AMPLITUDE_LEVEL1
#endif

#if CLLC_LAB == 4
#define CLLC_CONTROL_RUNNING_ON 1
#define CLLC_POWER_FLOW CLLC_POWER_FLOW_PRIM_SEC
#define CLLC_INCR_BUILD CLLC_CLOSED_LOOP_BUILD
#define CLLC_CONTROL_MODE CLLC_CURRENT_MODE
#define CLLC_TEST_SETUP CLLC_TEST_SETUP_RES_LOAD
#define CLLC_PROTECTION CLLC_PROTECTION_ENABLED
#define CLLC_SFRA_TYPE  0
#define CLLC_SFRA_AMPLITUDE (float32_t)CLLC_SFRA_INJECTION_AMPLITUDE_LEVEL1
#endif

#if CLLC_LAB == 5
#define CLLC_CONTROL_RUNNING_ON 1
#define CLLC_POWER_FLOW CLLC_POWER_FLOW_PRIM_SEC
#define CLLC_INCR_BUILD CLLC_CLOSED_LOOP_BUILD
#define CLLC_CONTROL_MODE CLLC_CURRENT_MODE
#define CLLC_TEST_SETUP CLLC_TEST_SETUP_EMULATED_BATTERY
#define CLLC_PROTECTION CLLC_PROTECTION_ENABLED
#define CLLC_SFRA_TYPE  0
#define CLLC_SFRA_AMPLITUDE (float32_t)CLLC_SFRA_INJECTION_AMPLITUDE_LEVEL1
#endif


#if CLLC_LAB == 6
#define CLLC_CONTROL_RUNNING_ON 1
#define CLLC_POWER_FLOW CLLC_POWER_FLOW_SEC_PRIM
#define CLLC_INCR_BUILD CLLC_OPEN_LOOP_BUILD
#define CLLC_CONTROL_MODE CLLC_VOLTAGE_MODE
#define CLLC_TEST_SETUP CLLC_TEST_SETUP_RES_LOAD
#define CLLC_PROTECTION CLLC_PROTECTION_DISABLED
#define CLLC_SFRA_TYPE  0
#define CLLC_SFRA_AMPLITUDE (float32_t)CLLC_SFRA_INJECTION_AMPLITUDE_LEVEL2
#endif

#if CLLC_LAB == 7
#define CLLC_CONTROL_RUNNING_ON 1
#define CLLC_POWER_FLOW CLLC_POWER_FLOW_SEC_PRIM
#define CLLC_INCR_BUILD CLLC_OPEN_LOOP_BUILD
#define CLLC_CONTROL_MODE CLLC_VOLTAGE_MODE
#define CLLC_TEST_SETUP CLLC_TEST_SETUP_RES_LOAD
#define CLLC_PROTECTION CLLC_PROTECTION_ENABLED
#define CLLC_SFRA_TYPE  0
#define CLLC_SFRA_AMPLITUDE (float32_t)CLLC_SFRA_INJECTION_AMPLITUDE_LEVEL2
#endif

#if CLLC_LAB == 8
#define CLLC_CONTROL_RUNNING_ON 1
#define CLLC_POWER_FLOW CLLC_POWER_FLOW_SEC_PRIM
#define CLLC_INCR_BUILD CLLC_CLOSED_LOOP_BUILD
#define CLLC_CONTROL_MODE CLLC_VOLTAGE_MODE
#define CLLC_TEST_SETUP CLLC_TEST_SETUP_RES_LOAD
#define CLLC_PROTECTION CLLC_PROTECTION_ENABLED
#define CLLC_SFRA_TYPE  0
#define CLLC_SFRA_AMPLITUDE (float32_t)CLLC_SFRA_INJECTION_AMPLITUDE_LEVEL1
#endif


#define CLLC_ISR1_RUNNING_ON CLLC_CONTROL_RUNNING_ON

#define CLLC_ISR2_FREQUENCY_HZ ((float32_t)120000)
#define CLLC_ISR3_FREQUENCY_HZ ((float32_t)10000)
#define CLLC_SFRA_ISR_FREQ_HZ       CLLC_ISR2_FREQUENCY_HZ

//
// Power Stage Related Values
//
#define CLLC_NOMINAL_PWM_SWITCHING_FREQUENCY_HZ  ((float32_t)200*1000)
#define CLLC_MAX_PWM_SWITCHING_FREQUENCY_HZ ((float32_t)400*1000)
#define CLLC_MIN_PWM_SWITCHING_FREQUENCY_HZ ((float32_t)100*1000)

#define CLLC_PRIM_PWM_DEADBAND_RED_NS ((float32_t)200)
#define CLLC_PRIM_PWM_DEADBAND_FED_NS ((float32_t)200)
#define CLLC_SEC_PWM_DEADBAND_RED_NS  ((float32_t)200)
#define CLLC_SEC_PWM_DEADBAND_FED_NS  ((float32_t)200)

#define CLLC_VPRIM_MAX_SENSE_VOLTS    ((float32_t)508.685)
#define CLLC_VSEC_MAX_SENSE_VOLTS     ((float32_t)482.863)
#define CLLC_VSEC_OPTIMAL_RANGE_VOLTS ((float32_t)450)
#define CLLC_IPRIM_MAX_SENSE_AMPS    ((float32_t)55.000)
#define CLLC_ISEC_MAX_SENSE_AMPS     ((float32_t)33.951)
#define CLLC_IPRIM_TANK_MAX_SENSE_AMPS ((float32_t)34.375)
#define CLLC_ISEC_TANK_MAX_SENSE_AMPS ((float32_t)42.375)

#define CLLC_VSEC_NOMINAL_VOLTS ((float32_t)350)
#define CLLC_VPRIM_NOMINAL_VOLTS ((float32_t)400)

#define CLLC_IPRIM_TRIP_LIMIT_AMPS ((float32_t)30)
#define CLLC_ISEC_TRIP_LIMIT_AMPS  ((float32_t)30)
#define CLLC_IPRIM_TANK_TRIP_LIMIT_AMPS ((float32_t)30)
#define CLLC_ISEC_TANK_TRIP_LIMIT_AMPS  ((float32_t)40)
#define CLLC_VSEC_TRIP_LIMIT_VOLTS  ((float32_t)450)

#define CLLC_CONTROL_PRECHARGE_TIME 2    // 2 seconds
#define CLLC_CONTROL_PRECHARGE_COUNT (CLLC_CONTROL_PRECHARGE_TIME * CLLC_ISR2_FREQUENCY_HZ)
#define CLLC_CONTROL_PRECHARGE_TPBRD_MAX (CLLC_CPU_SYS_CLOCK_FREQ_HZ / CLLC_NOMINAL_PWM_SWITCHING_FREQUENCY_HZ / 2.0f)

//
// Control Loop Design
//

//
// LAB3
//
#define CLLC_GV1_2P2Z_A1    (float32_t) -1.7284895037
#define CLLC_GV1_2P2Z_A2    (float32_t) 0.7284895037
#define CLLC_GV1_2P2Z_A3    (float32_t) 0.0000000000
#define CLLC_GV1_2P2Z_B0    (float32_t) 4.8280130584
#define CLLC_GV1_2P2Z_B1    (float32_t) 0.1493277469
#define CLLC_GV1_2P2Z_B2    (float32_t) -4.6786792593
#define CLLC_GV1_2P2Z_B3    (float32_t) 0.0000000000

//
// LAB4
//
#define CLLC_GI1_2P2Z_A1    (float32_t) -1.8277396009
#define CLLC_GI1_2P2Z_A2    (float32_t) 0.8277396009
#define CLLC_GI1_2P2Z_A3    (float32_t) 0.0000000000
#define CLLC_GI1_2P2Z_B0    (float32_t) 1.2500036172
#define CLLC_GI1_2P2Z_B1    (float32_t) 0.2153188876
#define CLLC_GI1_2P2Z_B2    (float32_t) -1.0346715071
#define CLLC_GI1_2P2Z_B3    (float32_t) 0.0000000000

//
// LAB5 
//
#define CLLC_GI2_2P2Z_A1    (float32_t) 0.0341879720
#define CLLC_GI2_2P2Z_A2    (float32_t) -0.7668017816
#define CLLC_GI2_2P2Z_A3    (float32_t) -0.2673861903
#define CLLC_GI2_2P2Z_B0    (float32_t) 1.3436620732
#define CLLC_GI2_2P2Z_B1    (float32_t) 0.3459370813
#define CLLC_GI2_2P2Z_B2    (float32_t) -0.7200660800
#define CLLC_GI2_2P2Z_B3    (float32_t) -0.2790608258

//
// LAB8 
//
#define CLLC_GV2_2P2Z_A1    (float32_t) -0.4829060140
#define CLLC_GV2_2P2Z_A2    (float32_t) -0.5170939860
#define CLLC_GV2_2P2Z_A3    (float32_t) 0.0000000000
#define CLLC_GV2_2P2Z_B0    (float32_t) 1.3436620732
#define CLLC_GV2_2P2Z_B1    (float32_t) -0.3488624959
#define CLLC_GV2_2P2Z_B2    (float32_t) -0.5396713815
#define CLLC_GV2_2P2Z_B3    (float32_t) 0.0000000000

//=============================================================================
// User code settings file
//=============================================================================
#include "cllc_user_settings.h"

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif //_PROJSETTINGS_H
