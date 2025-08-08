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
#ifndef PROJSETTINGS_H
#define PROJSETTINGS_H

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
// #define VOLTAGE_MODE 1
// #define CURRENT_MODE 2

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

#define LAB 1

#if LAB == 1
#define CONTROL_RUNNING_ON 1
#define POWER_FLOW POWER_FLOW_PRIM_SEC
#define INCR_BUILD OPEN_LOOP_BUILD
#define TEST_SETUP TEST_SETUP_RES_LOAD
#define PROTECTION PROTECTION_DISABLED
#define SFRA_TYPE  0
#define SFRA_AMPLITUDE (float32_t)SFRA_INJECTION_AMPLITUDE_LEVEL2
#endif

#define ISR1_RUNNING_ON CONTROL_RUNNING_ON

#define ISR2_FREQUENCY_HZ ((float32_t)100000)
#define ISR3_FREQUENCY_HZ ((float32_t)10000)
// #define SFRA_ISR_FREQ_HZ       ISR2_FREQUENCY_HZ

//
// Power Stage Related Values
//
// #define NOMINAL_PWM_SWITCHING_FREQUENCY_HZ  ((float32_t)500.8*1000)
#define NOMINAL_PWM_SWITCHING_FREQUENCY_HZ  ((float32_t)100.0*1000)
#define MAX_PWM_SWITCHING_FREQUENCY_HZ ((float32_t)700*1000)
#define MIN_PWM_SWITCHING_FREQUENCY_HZ ((float32_t)300*1000)

#define PRIM_PWM_DEADBAND_RED_NS ((float32_t)200)
#define PRIM_PWM_DEADBAND_FED_NS ((float32_t)200)
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

//=============================================================================
// User code settings file
//=============================================================================
#include "user_settings.h"

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif 
