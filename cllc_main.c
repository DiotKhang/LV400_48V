//#############################################################################
//
// FILE:   main.c
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
//---  State Machine Related ---
//
int16_t vTimer0[4];         // Virtual Timers slaved off CPU Timers (A events)
int16_t vTimer1[4];         // Virtual Timers slaved off CPU Timers (B events)

//
// Variable declarations for state machine
//
void (*Alpha_State_Ptr)(void);  // Base States pointer
void (*A_Task_Ptr)(void);       // State pointer A branch
void (*B_Task_Ptr)(void);       // State pointer B branch
void (*C_Task_Ptr)(void);       // State pointer C branch

//
// State Machine function prototypes
//------------------------------------
// Alpha states
//
void A0(void);  //state A0
void B0(void);  //state B0

//
// A branch states
//
void A1(void);  //state A1

//
// B branch states
//
void B1(void);  //state B1
void B2(void);  //state B2
void B3(void);  //state B3
uint32_t  CLLC_countcheckISR1;
uint32_t  CLLC_countcheckISR2;
uint32_t  CLLC_countcheckISR3;
//
// Note that the watchdog is disabled in codestartbranch.asm
// for this project. This is to prevent it from expiring while
// c_init routine initializes the global variables before
// reaching the main()
//

void main(void)
{
    //
    // This routine sets up the basic device configuration such as
    // initializing PLL, copying code from FLASH to RAM,
    // this routine will also initialize the CPU timers that are used in
    // the background 1, 2 & 3) task for this system (CPU time)
    //
    CLLC_HAL_setupDevice();

    CLLC_initGlobalVariables();
    CLLC_setBuildLevelIndicatorVariable();

    //  z
    // Stop all PWM mode clock
    //
    CLLC_HAL_disablePWMClkCounting();
    
    //
    // Set up peripherals
    //                       
    CLLC_HAL_setupADC();

    //
    // setup trigger for the ADC conversions
    //
    CLLC_HAL_setupTrigForADC();

    //
    // Profiling GPIO
    //
    CLLC_HAL_setupProfilingGPIO();

    //
    // clear any spurious flags
    // setup protection and trips for the board
    //
    // CLLC_HAL_setupBoardProtection();
    
    // Sets up the PWMs for the CLLC prim and sec bridges
    // by default the PWMs are set as battery charging mode
    CLLC_HAL_setupPWM(CLLC_powerFlowStateActive.CLLC_PowerFlowState_Enum);
    EALLOW;
    //
    // Set global load to one-shot mode
    //
    HWREGH(CLLC_PRIM_LEG1_PWM_BASE + EPWM_O_GLDCTL) = 0xA1;
    HWREGH(CLLC_PRIM_LEG2_PWM_BASE + EPWM_O_GLDCTL) = 0xA7;
    //
    // Link EPWM2 to EPWM1
    //
    HWREG(CLLC_PRIM_LEG2_PWM_BASE + EPWM_O_XLINK) &= ~(0xF0000000);
    EDIS;

    //
    // setup PWM pins
    //
    CLLC_HAL_setupPWMpins(CLLC_pwmSwState_synchronousRectification_active);

    //
    // setup CMPSS for synchRect
    //
    CLLC_HAL_setupSynchronousRectificationAction(
            CLLC_powerFlowStateActive.CLLC_PowerFlowState_Enum);

    //
    // brings out the blanked CMPSS signal on GPIO for debug
    //
    CLLC_HAL_setupSynchronousRectificationActionDebug(
            CLLC_powerFlowStateActive.CLLC_PowerFlowState_Enum);

    //
    // Enable PWM Clocks
    //
    CLLC_HAL_enablePWMClkCounting(); 
    
    //
    // as LLC is resonant and frequency changes,
    // for ISR separate fixed frequency PWM is configured
    //
    CLLC_HAL_setupPWMinUpDownCountMode(CLLC_ISR2_PWM_BASE,
                               CLLC_ISR2_FREQUENCY_HZ,
                               CLLC_PWMSYSCLOCK_FREQ_HZ);
    CLLC_HAL_setupECAPinPWMMode(CLLC_ISR2_ECAP_BASE,
                                 CLLC_ISR2_FREQUENCY_HZ,
                                 CLLC_PWMSYSCLOCK_FREQ_HZ);

    //
    // ISR Mapping
    //
    CLLC_HAL_setupInterrupt(CLLC_powerFlowStateActive.CLLC_PowerFlowState_Enum);

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
    } //END MAIN CODE
}
// 
// ISRs are named by the priority
// ISR1 is the highest priority
// ISR2 has the next highest and so forth
// 
#if CLLC_ISR1_RUNNING_ON == C28x_CORE
interrupt void CLLC_ISR1(void)
{
    CLLC_HAL_setProfilingGPIO1();

    CLLC_runISR1();

    CLLC_HAL_clearISR1InterruputFlag();
    CLLC_countcheckISR1+=1;
    CLLC_HAL_resetProfilingGPIO1();

}
#endif

#if CLLC_ISR1_RUNNING_ON == C28x_CORE
interrupt void CLLC_ISR1_second(void)
{
    //
    CLLC_HAL_setProfilingGPIO1();
    //
    CLLC_runISR1_secondTime();
    CLLC_HAL_clearISR1InterruputFlag();
    //
    CLLC_HAL_resetProfilingGPIO1();
    //
    Interrupt_register(CLLC_ISR1_TRIG, &CLLC_ISR1);
}
#endif

#if CLLC_ISR2_RUNNING_ON == C28x_CORE
interrupt void CLLC_ISR2_primToSecPowerFlow(void)
{
    //
    // enable group 3 interrupt only to interrupt ISR2
    //
    IER |= 0x4;
    IER &= 0x4;
    EINT;
    CLLC_countcheckISR2+=1;
    CLLC_HAL_setProfilingGPIO2();
    CLLC_runISR2_primToSecPowerFlow();
    CLLC_HAL_resetProfilingGPIO2();
    DINT;
    CLLC_HAL_clearISR2PeripheralInterruptFlag();
    CLLC_HAL_clearISR2InterruputFlag();
}

interrupt void CLLC_ISR2_secToPrimPowerFlow(void)
{
    //
    // enable group 3 interrupt only to interrupt ISR2
    //
    IER |= 0x4;
    IER &= 0x4;
    EINT;
    CLLC_runISR2_secToPrimPowerFlow();
    DINT;
    CLLC_HAL_clearISR2InterruputFlag();
}
#endif

interrupt void CLLC_ISR3(void)
{
    EINT;
    CLLC_HAL_setProfilingGPIO3();
    CLLC_runISR3();
    CLLC_HAL_resetProfilingGPIO3();
    CLLC_countcheckISR3+=1;
    DINT;
    CLLC_HAL_clearISR3InterruputFlag();
}

//
//=============================================================================
//  STATE-MACHINE SEQUENCING AND SYNCRONIZATION FOR SLOW BACKGROUND TASKS
//=============================================================================
//
//
//--------------------------------- FRAME WORK --------------------------------
//
void A0(void)
{
    //
    // // loop rate synchronizer for A-tasks
    // //
    // if(GET_TASKA_TIMER_OVERFLOW_STATUS == 1)
    // {
    //     CLEAR_TASKA_TIMER_OVERFLOW_FLAG;    // clear flag

    //     //
    //     // jump to an A Task (A1,A2,A3,...)
    //     //
    //     (*A_Task_Ptr)();

    //     vTimer0[0]++;           // virtual timer 0, instance 0 (spare)
    // }
    // Alpha_State_Ptr = &B0;      // Comment out to allow only A tasks
}

void B0(void)
{
    //
    // loop rate synchronizer for B-tasks
    //
    // if(GET_TASKB_TIMER_OVERFLOW_STATUS  == 1)
    // {
    //     CLEAR_TASKB_TIMER_OVERFLOW_FLAG;                // clear flag

    //     //
    //     // jump to a B Task (B1,B2,B3,...)
    //     //
    //     (*B_Task_Ptr)();

    //     vTimer1[0]++;           // virtual timer 1, instance 0 (spare)
    // }

    // //
    // // Allow A state tasks
    // //
    // Alpha_State_Ptr = &A0;
}

//
//=============================================================================
//  A - TASKS (executed in every 1 msec)
//=============================================================================
//

void A1(void)
{
// #if SFRA_TYPE != SFRA_DISABLED
//     runSFRABackGroundTasks();
// #endif

//     // changeSynchronousRectifierPwmBehavior(POWER_FLOW);

//     //
//     //the next time CpuTimer0 'counter' reaches Period value go to A2
//     //
//     A_Task_Ptr = &A1;

}

//
//=============================================================================
//  B - TASKS (executed in every 5 msec)
//=============================================================================
//

void B1(void)
{

    // CLLC_updateBoardStatus();

    //
    //the next time CpuTimer1 'counter' reaches Period value go to B2
    //
    // B_Task_Ptr = &B2;
}

void B2(void)
{

    //
    //the next time CpuTimer1 'counter' reaches Period value go to B3
    //
    // B_Task_Ptr = &B3;

}

void B3(void)
{
    // HAL_toggleLED1();

    //
    //the next time CpuTimer1 'counter' reaches Period value go to B1
    //
    // B_Task_Ptr = &B1;

}

// //
// // No more.
// //
