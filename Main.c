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
float PWMfslow = 8000;
int TIMER_SECOND = 0;
int STEPS2;
int STEPS3;

//Function prototypes
void Timer1Setup();
void HbridgeSetup();
void LMcontrol (unsigned int direction, float speed);
void RMcontrol (unsigned int direction, float speed);
void DriveStraight();
void StopRobot();
void TurnLeft();
void TurnRight();
void __attribute__((interrupt, no_auto_psv)) _OC2Interrupt(void);    //This ISR is run every time PWM goes high
void __attribute__((interrupt, no_auto_psv)) _OC3Interrupt(void);

//Main
int main(void) {
    _RCDIV = 0b000;     //Divide Fosc by 1

    //Declare local variables
    enum {STOP, STRAIGHT, RIGHT, LEFT} driveState;
    int n;
    //Setup
    Timer1Setup();
    HbridgeSetup();


    //Initialize state
    driveState = STRAIGHT;
    DriveStraight();
    STEPS2 = 0;
    STEPS3 = 0;
    n = 0;
    TMR1 = 0;
    
    while(1){

        switch(driveState)
        {
                case STOP:
                   
                    break;
                case STRAIGHT:
                    if(STEPS2 >= 1600 && STEPS3 >= 1600 && (n==1)){
                        driveState = RIGHT;
                        TurnRight();
                        STEPS2 = 0;
                        STEPS3 = 0;
                        n = 2;
                    }
                    else if (STEPS2 >= 1600 && STEPS3 >= 1600 && (n==2)){
                        driveState = STOP;
                        StopRobot();
                        STEPS2 = 0;
                        STEPS3 = 0;
                    }
                    else if (STEPS2 >= 1600 && STEPS3 >= 1600 && (n==0)){
                        driveState = LEFT;
                        TurnLeft();
                        STEPS2 = 0;
                        STEPS3 = 0;
                        n = 1;
                    }

                    
                    break;
            case LEFT:
                if (STEPS2 >= 350 && STEPS3 >=350){
                    driveState = STRAIGHT;
                    DriveStraight();
                    STEPS2 = 0;
                    STEPS3 = 0;
                }
                break;
            case RIGHT:
                if (STEPS2 >= 700 && STEPS3 >= 700){
                    driveState = STRAIGHT;
                    DriveStraight();
                    STEPS2 = 0;
                    STEPS3 = 0;
                    
                }
        }
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

    //Setup output compare 2 and 3
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
    
    //Setup OC interrupts
    _OC2IP = 4;     //Sets priority
    _OC3IP = 5;
    
    _OC2IF = 0;     //reset flag to 0
    _OC3IF = 0;
    
    _OC2IE = 1;     //enables interrupt
    _OC3IE = 1;
}

void LMcontrol (unsigned int direction, float speed){
//Pass this function a 1 to go forward or a 0 to go backward,
//and a duty cycle (as a decimal) to control the left motor. 
//This uses "Control method 2" as seen in the H-bridge 
//PSC website specsheet
    _LATB7 = direction;
    OC2RS = speed;
    OC2R = speed*.5;
}

void RMcontrol (unsigned int direction, float speed){
//Pass this function a 1 to go forward or a 0 to go backward,
//and a duty cycle (as a decimal) to control the right motor. 
//This uses "Control method 2" as seen in the H-bridge 
//PSC website specsheet
    
    _LATB8 = direction;
    OC3RS = speed;
    OC3R = speed*.5;
}

void DriveStraight(){
    LMcontrol(1,PWMfslow);
    RMcontrol(1,PWMfslow);
}

void StopRobot(){
    LMcontrol(1,0);
    RMcontrol(1,0);
}

void TurnLeft(){
    LMcontrol(0,PWMfslow);
    RMcontrol(1,PWMfslow);       
}

void TurnRight(){
    LMcontrol(1,PWMfslow);
    RMcontrol(0,PWMfslow);
}

// OC1 Interrupt Service Routine
void __attribute__((interrupt, no_auto_psv)) _OC2Interrupt(void)    //This ISR is run every time PWM goes high
{
    // When the OC1 Interrupt is activated, the code will jump up here
    // each time your PIC generates a PWM step
    // Add code to increment the step count each time this function
    // is called
    _OC2IF = 0;
    STEPS2++;
}

// OC1 Interrupt Service Routine
void __attribute__((interrupt, no_auto_psv)) _OC3Interrupt(void)    //This ISR is run every time PWM goes high
{
    // When the OC1 Interrupt is activated, the code will jump up here
    // each time your PIC generates a PWM step
    // Add code to increment the step count each time this function
    // is called
    _OC3IF = 0;
    STEPS3++;
}
