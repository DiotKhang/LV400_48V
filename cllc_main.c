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
    // as LLC is resonant and frequency changes,
    // for ISR separate fixed frequency PWM is configured
    //
    HAL_setupPWMinUpDownCountMode(ISR2_PWM_BASE,
                               ISR2_FREQUENCY_HZ,
                               PWMSYSCLOCK_FREQ_HZ);
    HAL_setupECAPinPWMMode(ISR2_ECAP_BASE,
                                 ISR2_FREQUENCY_HZ,
                                 PWMSYSCLOCK_FREQ_HZ);
                                 
    //
    // power up ADC on the device
    //                       
    HAL_setupADC();

    //
    // Iprim is sensed by PGA, setup the peripherals
    //
    // HAL_setupIprimSensedSignalChain();

    //
    // Profiling GPIO
    //
    HAL_setupProfilingGPIO();

    //
    // setup CMPSS for synchRect
    //
    HAL_setupSynchronousRectificationAction(
            powerFlowStateActive.PowerFlowState_Enum);

    //
    // brings out the blanked CMPSS signal on GPIO for debug
    //
    HAL_setupSynchronousRectificationActionDebug(
            powerFlowStateActive.PowerFlowState_Enum);

    //
    // clear any spurious flags
    // setup protection and trips for the board
    //
    HAL_setupBoardProtection();

    //
    // set's a global variable that indicates which build level is running
    // This variable can be viewed in the expressions view.
    // Changes to this variable through the expressions view has no effect
    //
    setBuildLevelIndicatorVariable();

    //
    // setup trigger for the ADC conversions
    //
    HAL_setupTrigForADC();

    //
    // setup PWM pins
    //
    HAL_setupPWMpins(pwmSwState_synchronousRectification_active);

    //
    // ISR Mapping
    //
    HAL_setupInterrupt(powerFlowStateActive.PowerFlowState_Enum);

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
    } //END MAIN CODE
}
// 
// ISRs are named by the priority
// ISR1 is the highest priority
// ISR2 has the next highest and so forth
// 
#if ISR1_RUNNING_ON == C28x_CORE
interrupt void ISR1(void)
{
    //   
    HAL_setProfilingGPIO1();   
    //   
    runISR1();   
    HAL_clearISR1InterruputFlag();
    //
    HAL_resetProfilingGPIO1();
    //
    Interrupt_register(ISR1_TRIG, &ISR1_second);
}
#endif

#if ISR1_RUNNING_ON == C28x_CORE
interrupt void ISR1_second(void)
{
    //
    HAL_setProfilingGPIO1();
    //
    runISR1_secondTime();
    HAL_clearISR1InterruputFlag();
    //
    HAL_resetProfilingGPIO1();
    //
    Interrupt_register(ISR1_TRIG, &ISR1);
}
#endif

#if ISR2_RUNNING_ON == C28x_CORE
interrupt void ISR2_primToSecPowerFlow(void)
{
    //
    // enable group 3 interrupt only to interrupt ISR2
    //
    IER |= 0x4;
    IER &= 0x4;
    EINT;
    HAL_setProfilingGPIO2();
    runISR2_primToSecPowerFlow();
    DINT;
    HAL_resetProfilingGPIO2();
    HAL_clearISR2InterruputFlag();
}

interrupt void ISR2_secToPrimPowerFlow(void)
{
    //
    // enable group 3 interrupt only to interrupt ISR2
    //
    IER |= 0x4;
    IER &= 0x4;
    EINT;
    // HAL_setProfilingGPIO2();
    runISR2_secToPrimPowerFlow();
    // HAL_resetProfilingGPIO2();
    DINT;
    HAL_clearISR2InterruputFlag();
}
#endif

interrupt void ISR3(void)
{
    EINT;
    HAL_setProfilingGPIO3();
    runISR3();
    HAL_resetProfilingGPIO3();
    DINT;
    HAL_clearISR3InterruputFlag();
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
    // loop rate synchronizer for A-tasks
    //
    if(GET_TASKA_TIMER_OVERFLOW_STATUS == 1)
    {
        CLEAR_TASKA_TIMER_OVERFLOW_FLAG;    // clear flag

        //
        // jump to an A Task (A1,A2,A3,...)
        //
        (*A_Task_Ptr)();

        vTimer0[0]++;           // virtual timer 0, instance 0 (spare)
    }
    Alpha_State_Ptr = &B0;      // Comment out to allow only A tasks
}

void B0(void)
{
    //
    // loop rate synchronizer for B-tasks
    //
    if(GET_TASKB_TIMER_OVERFLOW_STATUS  == 1)
    {
        CLEAR_TASKB_TIMER_OVERFLOW_FLAG;                // clear flag

        //
        // jump to a B Task (B1,B2,B3,...)
        //
        (*B_Task_Ptr)();

        vTimer1[0]++;           // virtual timer 1, instance 0 (spare)
    }

    //
    // Allow A state tasks
    //
    Alpha_State_Ptr = &A0;
}

//
//=============================================================================
//  A - TASKS (executed in every 1 msec)
//=============================================================================
//

void A1(void)
{
#if SFRA_TYPE != SFRA_DISABLED
    runSFRABackGroundTasks();
#endif

    // changeSynchronousRectifierPwmBehavior(POWER_FLOW);

    //
    //the next time CpuTimer0 'counter' reaches Period value go to A2
    //
    A_Task_Ptr = &A1;

}

//
//=============================================================================
//  B - TASKS (executed in every 5 msec)
//=============================================================================
//

void B1(void)
{

    // updateBoardStatus();

    //
    //the next time CpuTimer1 'counter' reaches Period value go to B2
    //
    B_Task_Ptr = &B2;
}

void B2(void)
{

    //
    //the next time CpuTimer1 'counter' reaches Period value go to B3
    //
    B_Task_Ptr = &B3;

}

void B3(void)
{
    // HAL_toggleLED1();

    //
    //the next time CpuTimer1 'counter' reaches Period value go to B1
    //
    B_Task_Ptr = &B1;

}

// //
// // No more.
// //
