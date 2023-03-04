/*
 * File:   Main.c
 * Author: cklomp92
 *
 * Created on February 22, 2023, 5:06 PM
 */


#include "xc.h"
#pragma config FNOSC = FRCDIV     //8 MHz Internal Oscillator with post-scaler

//Declare global variables
float PWMfrequency = 4000;
int TIMER_SECOND = 0;

//Function prototypes
int Timer1Setup();
void HbridgeSetup ();
void LMcontrol (unsigned int direction, float dutyCycle);
void RMcontrol (unsigned int direction, float dutyCycle);

//Main
int main(void) {
    _RCDIV = 0b000;     //Divide Fosc by 1

    //Declare local variables
    enum {STOP, STRAIGHT, RIGHT, LEFT} driveState;

    //Setup
    Timer1Setup();
    HbridgeSetup();


    //Initialize state
    driveState = STOP;

    while(1){
        switch(driveState)
        {
            case STOP:
                if(TMR1 >= 0 && TMR1 <= TIMER_SECOND * 2){

                }

        }
    }

    return 0;
}

int Timer1Setup(){
    //By using the 256 prescaler and the LPRC oscillator
    //Timer1 has a frequency of 500,000/(2*256) = 976 Hz.

    TIMER_SECOND = 976;         //Update global variable

    T1CONbits.TON = 1;          //Turn timer on
    T1CONbits.TCKPS = 0b11;     //256 prescaler
    T1CONbits.TCS = 1;          //Clock source set by ECS
    T1CONbits.T1ECS = 0b10;     /*Timer1 uses LPRC (500 kHz) 
                                as clock source*/

    //T1CONbits.TGATE = 0;  //unnecessary because TCS = 1

    T1CONbits.TSYNC = 1;    /*POSSIBLE FIXME not sure what 
                            this does exactly.
                            Says it "synchronizes external
                            clock input"*/

    PR1 = TIMER_SECOND * 10;    //Resets timer every 10 seconds
    
}

void HbridgeSetup (){
    //Setup H-bridge IN1 - IN4 pins (pins 15-18)
    _ANSB12 = 0;    //Digital       B12 = IN1(Left motor)
    _ANSB13 = 0;    //              B13 = IN2(Left motor)
    _ANSB14 = 0;    //              B14 = IN3(Right motor)
    _ANSB15 = 0;    //              B15 = IN4(Right motor)
    _TRISB12 = 0;   //Output
    _TRISB13 = 0;   //
    _TRISB14 = 0;   //
    _TRISB15 = 0;   //

    //Setup H-bridge ENA/ENB pins (PWM pins 4 and 5)
    _ANSB0 = 0;     //Digital
    _ANSB1 = 0;     
    _TRISB0 = 0;    //Output
    _TRISB1 = 0;    

    OC2CON1 = 0;    //Clear pin 4   OC2 = Left motor
    OC2CON2 = 0;    //
    OC3CON1 = 0;    //Clear pin 5   OC3 = Right motor
    OC3CON2 = 0;

    OC2CON1bits.OCTSEL = 0b111;     //Use system clock
    OC3CON1bits.OCTSEL = 0b111;

    OC2CON1bits.OCM = 0b110;    //Edge-aligned PWM
    OC3CON1bits.OCM = 0b110;

    OC2CON2bits.OCTRIG = 0;     //Sync mode
    OC3CON2bits.OCTRIG = 0;

    OC2CON2bits.SYNSEL = 0b11111;   //Set sync source to OCX
    OC3CON2bits.SYNSEL = 0b11111;

    OC2RS = PWMfrequency;       //1000 Hz PWM Frequency
    OC3RS = PWMfrequency;
}

void LMcontrol (unsigned int direction, float dutyCycle){
//Pass this function a 1 to go forward or a 0 to go backward,
//and a duty cycle (as a decimal) to control the left motor. 
//This uses "Control method 2" as seen in the H-bridge 
//PSC website specsheet

    if (direction = 1){
        _LATB12 = 1;    //Set IN1 high and IN2 low to go forward
        _LATB13 = 0;
    }
    else if (direction = 0){
        _LATB12 = 0;    //Set IN1 low and IN2 high to go backward
        _LATB13 = 1;
    }
    else{
        OC2R = 0;       //Stop motor if incorrect value is passed
    }
    OC2R = PWMfrequency * dutyCycle;
}

void RMcontrol (unsigned int direction, float dutyCycle){
//Pass this function a 1 to go forward or a 0 to go backward,
//and a duty cycle (as a decimal) to control the right motor. 
//This uses "Control method 2" as seen in the H-bridge 
//PSC website specsheet

    if (direction = 1){
        _LATB14 = 1;    //Set IN3 high and IN4 low to go forward
        _LATB15 = 0;
    }
    else if (direction = 0){
        _LATB14 = 0;    //Set IN3 low and IN4 high to go backward
        _LATB15 = 1;
    }
    else{
        OC3R = 0;       //Stop motor if incorrect value is passed
    }
    OC3R = PWMfrequency * dutyCycle;
}
