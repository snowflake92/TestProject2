/*
 * File:   Main.c
 * Author: cklomp92
 *
 * Created on February 22, 2023, 5:06 PM
 */


#include "xc.h"
#pragma config FNOSC = FRCDIV     //8 MHz Internal Oscillator with post-scaler
#pragma config ICS = PGx2         //Changes programming pins from 8/9 to 2/3
//Declare global variables
float PWMfrequency = 4000;
int TIMER_SECOND = 0;

//Function prototypes
void Timer1Setup();
void HbridgeSetup();
void LMcontrol (unsigned int direction, float dutyCycle);
void RMcontrol (unsigned int direction, float dutyCycle);
void DriveStraight();
void TurnLeft();
void TurnRight();

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
    
    TMR1 = 0;
    while(1){
        
        if(TMR1 >= TIMER_SECOND && TMR1 <= TIMER_SECOND*3){
            //DriveStraight();
            OC2R = 0.5 * PWMfrequency;
            OC3R = 0.5 * PWMfrequency;

            _LATB13 = 1;    //Set IN1 high and IN2 low to go forward
            _LATB14 = 0;
            _LATA4 = 0;    //Set IN3 high and IN4 low to go forward
            _LATB7 = 1;
        }
//        else if(TMR1 > TIMER_SECOND*2 && TMR1 <= TIMER_SECOND *3){
//            TurnLeft();
//        }
//        else if(TMR1 > TIMER_SECOND*3 && TMR1 <= TIMER_SECOND*5){
//            DriveStraight();
//        }
//        else if(TMR1 > TIMER_SECOND*5 && TMR1 <= TIMER_SECOND*6){
//            TurnRight();
//        }
    }

    return 0;
}

void Timer1Setup(){
    //By using the 256 prescaler and the LPRC oscillator
    //Timer1 has a frequency of 500,000/(2*256) = 976 Hz.

    TIMER_SECOND = 976/5;         //Update global variable

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

    //PR1 = TIMER_SECOND * 10;    //Resets timer every 10 seconds
    
}

void HbridgeSetup (){
    //Setup H-bridge IN1 - IN4 pins (pins 8-11)
    ANSA = 0;   //Digital
    ANSB = 0;
    TRISA = 0;  //Output
    TRISB = 0;
    
//    _ANSA3 = 0;    //Digital       A3 = IN1(Left motor)
//    _ANSB4 = 0;    //              B4 = IN2(Left motor)
//    _ANSA4 = 0;    //              A4 = IN3(Right motor)
//    _ANSB7 = 0;    //              B7 = IN4(Right motor)
//    _TRISA3 = 0;   //Output
//    _TRISB4 = 0;   //
//    _TRISA4 = 0;   //
//    _TRISB7 = 0;   //

    //Setup H-bridge ENA/ENB pins (PWM pins 4 and 5)
//    _ANSB0 = 0;     //Digital
//    _ANSB1 = 0;     
//    _TRISB0 = 0;    //Output
//    _TRISB1 = 0;    

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

    OC2CON2bits.SYNCSEL = 0b11111;   //Set sync source to OCX
    OC3CON2bits.SYNCSEL = 0b11111;

    OC2RS = PWMfrequency;       //1000 Hz PWM Frequency
    OC3RS = PWMfrequency;
}

void LMcontrol (unsigned int direction, float dutyCycle){
//Pass this function a 1 to go forward or a 0 to go backward,
//and a duty cycle (as a decimal) to control the left motor. 
//This uses "Control method 2" as seen in the H-bridge 
//PSC website specsheet
    OC2R = PWMfrequency * dutyCycle;
    if (direction = 1){
        _LATA3 = 1;    //Set IN1 high and IN2 low to go forward
        _LATB4 = 0;
    }
    else if (direction = 0){
        _LATA3 = 0;    //Set IN1 low and IN2 high to go backward
        _LATB4 = 1;
    }
    else{
        OC2R = 0;       //Stop motor if incorrect value is passed
    }
    
}

void RMcontrol (unsigned int direction, float dutyCycle){
//Pass this function a 1 to go forward or a 0 to go backward,
//and a duty cycle (as a decimal) to control the right motor. 
//This uses "Control method 2" as seen in the H-bridge 
//PSC website specsheet
    
    OC3R = PWMfrequency * dutyCycle;
    if (direction = 1){
        _LATA4 = 1;    //Set IN3 high and IN4 low to go forward
        _LATB7 = 0;
    }
    else if (direction = 0){
        _LATA4 = 0;    //Set IN3 low and IN4 high to go backward
        _LATB7 = 1;
    }
    else{
        OC3R = 0;       //Stop motor if incorrect value is passed
    }
    
}

void DriveStraight(){
    LMcontrol(1,.5);
    RMcontrol(1,.5);
}

void TurnLeft(){
    LMcontrol(1,.25);
    RMcontrol(1,.5);       
}

void TurnRight(){
    LMcontrol(1,.5);
    RMcontrol(1,.25);
}
