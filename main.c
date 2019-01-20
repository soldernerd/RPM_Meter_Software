/*
 * File:   main.c
 * Author: Luke
 *
 * Created on 21. July 2018, 22:30
 */

/** INCLUDES *******************************************************/


#include <xc.h>
#include <stdint.h>

#include "config.h"
#include "display.h"
#include "encoder.h"

/*
 * Function prototypes
 */

static void _system_init(void);
static void _calculate_speed(void);


void main(void)
{ 
    _system_init();
    
    display_startup();

    while(1)
    {
        if(os.ready)
        {
            _calculate_speed();
            os.ready = 0;

            //Update display
            display_set(os.speed, 0x00);
        }
    }//end while(1)
    
}//end main


void __interrupt() _isr(void)
{
    //Timer1 interrupt: Measurements
    if(PIR4bits.TMR1IF)
    {
        timer_1_isr();
    }
    
    //Encoder A signal changed
    if(PIR6bits.CCP1IF)
    {
        capture_A_isr();
    }
    
    //Encoder B signal changed
    if(PIR6bits.CCP2IF)
    {
        capture_B_isr();
    }

    //Timer2 interrupt: Display update
    if(PIR4bits.TMR2IF)
    {
        display_isr();
    }
}

static void _system_init(void)
{
    DISPLAY_A_TRIS = PIN_OUTPUT;
    DISPLAY_B_TRIS = PIN_OUTPUT;
    DISPLAY_C_TRIS = PIN_OUTPUT;
    DISPLAY_D_TRIS = PIN_OUTPUT;
    DISPLAY_E_TRIS = PIN_OUTPUT;
    DISPLAY_F_TRIS = PIN_OUTPUT;
    DISPLAY_G_TRIS = PIN_OUTPUT;
    DISPLAY_DP_TRIS = PIN_OUTPUT;
    DISPLAY_SEG1_TRIS = PIN_OUTPUT;
    DISPLAY_SEG2_TRIS = PIN_OUTPUT;
    DISPLAY_SEG3_TRIS = PIN_OUTPUT;
    DISPLAY_SEG4_TRIS = PIN_OUTPUT;

    ENCODER_A_TRIS = PIN_INPUT;
    ENCODER_A_ANSEL = PIN_DIGITAL;
    ENCODER_B_TRIS = PIN_INPUT;
    ENCODER_B_ANSEL = PIN_DIGITAL;
    
    PPSUnLock();
    CCP1PPS = ENCODER_A_PPS_VALUE;
    CCP2PPS = ENCODER_B_PPS_VALUE;
    PPSLock();
    
    display_init();
    encoder_init();
    
}

static void _calculate_speed(void)
{
    int32_t distance;
    uint32_t time_difference;
    float speed;
    
    if((os.time_sum_last==0) || (os.time_sum==0))
    {
        //Time measurements cannot be trusted
        time_difference = 67108864; //2^16 * 32 * 32
    }
    else
    {
        //Use captured time
        time_difference = os.time_sum - os.time_sum_last;
    }

    distance = (uint32_t) os.encoder_difference;
    //distance *= 60; //Seconds per minute
    speed = (float) distance;
    speed /= time_difference;
    //speed /= 256; //Pulses per rotation
    //speed *= 32; //Averaging
    //speed *= 8000000.0; //Timer frequency
    speed *= 60000000; // 60 seconds per minute * 32 measurements * 8000000 timer frequency / 256 pulses per rotation
    os.speed = (int16_t) speed;  
}
