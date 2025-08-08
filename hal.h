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
#include "settings.h"

//
// the function prototypes
//
void HAL_setupDevice(void);
void HAL_disablePWMClkCounting(void);
void HAL_enablePWMClkCounting(void);
void HAL_setupPWM(uint16_t powerFlowDir);
void HAL_setupEPWMinUpDownCountModeWithDeadBand(uint32_t base1,
                                float32_t pwmFreq_Hz,
                                float32_t pwmSysClkFreq_Hz,
                                float32_t red_ns,
                                float32_t fed_ns);
void HAL_setupEPWMinUpDownCount2ChAsymmetricMode(uint32_t base1,
                            float32_t pwmFreq_Hz,
                            float32_t pwmSysClkFreq_Hz);
void HAL_setupPWMpins(uint16_t mode);
#endif