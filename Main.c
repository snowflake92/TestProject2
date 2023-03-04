/*
 * File:   Main.c
 * Author: cklomp92
 *
 * Created on February 22, 2023, 5:06 PM
 */


#include "xc.h"
#pragma config FNOSC = FRCDIV     //8 MHz Internal Oscillator with post-scaler

int main(void) {
    _RCDIV = 0b000;     //Divide Fosc by 1

    //Setup H-bridge IN1 - IN4 pins (pins 15-18)
    _ANSB12 = 0;    //Digital
    _ANSB13 = 0;
    _ANSB14 = 0;
    _ANSB15 = 0;
    _TRISB12 = 0;   //Output
    _TRISB13 = 0;
    _TRISB14 = 0;
    _TRISB15 = 0;

    //Setup H-bridge ENA/ENB pins (PWM pins 4 and 5)
    _ANSB0 = 0;     //Digital
    _ANSB1 = 0;
    _TRISB0 = 0;    //Output
    _TRISB1 = 0;    

    OC2CON1 = 0;    //Clear pin 4
    OC2CON2 = 0;
    OC3CON1 = 0;    //Clear pin 5
    OC3CON2 = 0;

    OC2CON1bits.OCTSEL = 0b111;     //Use system clock
    OC3CON1bits.OCTSEL = 0b111;

    OC2CON1bits.OCM = 0b110;    //Edge-aligned PWM
    OC3CON1bits.OCM = 0b110;

    OC2CON2bits.OCTRIG = 0;     //Sync mode
    OC3CON2bits.OCTRIG = 0;

    OC2CON2bits.SYNSEL = 0b11111;   //Set sync source to OCX
    OC3CON2bits.SYNSEL = 0b11111;

    OC2RS = 4000;       //1000 Hz PWM Frequency
    OC3RS = 4000;

    return 0;
}
