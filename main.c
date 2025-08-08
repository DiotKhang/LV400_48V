//#############################################################################
//
// FILE:   clllc_main.c
//
// TITLE: This is the main file for the solution, following is the
//         <solution>.c -> solution source file
//         <solution>.h -> solution header file
//         <solution>_settings.h -> powerSUITE generated settings
//         <boad name>_board.c -> board drivers source file
//         <boad name>_board.h -> board drivers header file
//         <optional>
//         <solution>_cla.cla -> cla task file
//
//#############################################################################
#include "cllc.h"

//
//---  State Machine Related --- (SKIPPED)
//

void main(void)
{
    //
    // This routine sets up the basic device configuration such as
    // initializing PLL, copying code from FLASH to RAM,
    // this routine will also initialize the CPU timers that are used in
    // the background 1, 2 & 3) task for this system (CPU time)
    //

    HAL_setupDevice();

    //
    // Initialize global variables
    //
    initGlobalVariables();

    //
    // Stop all PWM mode clock
    //
    HAL_disablePWMClkCounting();
    
    //
    // Sets up the PWMs for the CLLC prim and sec bridges
    // by default the PWMs are set as battery charging mode
    //
    HAL_setupPWM(powerFlowStateActive.PowerFlowState_Enum);

    //
    // setup PWM pins
    //
    HAL_setupPWMpins(pwmSwState_synchronousRectification_active);

    //
    // Enable PWM Clocks
    //
    HAL_enablePWMClkCounting();
    //
    // IDLE loop. Just sit and loop forever, periodically will branch into
    // A0-A3, B0-B3, C0-C3 tasks
    // Frequency of this branching is set in setupDevice routine:
    //
    for(;;)
    {
        //
        // State machine entry & exit point
        //===========================================================
        //
        // (*Alpha_State_Ptr)();   // jump to an Alpha state (A0,B0,...)
        //
        //===========================================================
        //
    }
}