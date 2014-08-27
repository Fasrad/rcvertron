/*-------|---------|---------|---------|---------|---------|---------|---------|
pulsecap.c

A program to read input pulses of between 1 and 2 ms and output 0-5V signal

Typical use case is decoding RC receiver signals

by chaz miller 

This is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 or any later
version. This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
******************************************************************/
/*
Hardware: Arduino Pro Mini with fuses purged of Arduino nonsense
Servo attached to PB0. 
Optional LEDs on PB3-5 to indicate undersize pulse, oversize pulse, 
and timer overflow respectively.
PWM is output on PD6 at ~8kHz. 
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <serial.h>

#define pulse_max 4095 //these correspond to the "spec" for RC servo
#define pulse_min 2048 //signals, but could be tweaked if so desired

void delay(uint16_t);
void blink (uint8_t);

volatile uint16_t capture_val;

int main(){

    DDRB=0xFE;
    DDRD=0xFF;

    //set up timer1 for input capture to read RC signals
    TCCR1B = (1<<CS11);        //clk/8 = 2MHz
    TIMSK1|=1<<ICIE1;           //enable input capture interrupt

    //set up pwm timer0
    TCCR0A = 0b10100011;       //fast pwm, page 103
    TCCR0B |= 1<<CS01;                //fcpu / 8 = ~8khz PWM

//    volatile extern uint16_t capture_val;
    uint8_t ledcnt;

    /****************************************
    *****main loop***************************
    ****************************************/
    for(;;){  
        
        if(TIFR1&1<<TOV1){
            PORTB|=1<<5;          //turn on LED if timer has overflowed
        }

        //at 16MHz, capture_val should be 2000-4000 ticks

        if(capture_val>pulse_max){      //trim down
            capture_val=pulse_max;
            PORTB|=1<<4;                //warning LED
        }
        if(capture_val<pulse_min){      //pad up
            capture_val=pulse_min+1;
            PORTB|=1<<3;                //warning LED
        }
        capture_val-=pulse_min;
        OCR0A=capture_val<<3;           //map to PWM

        ledcnt++;                       //reset warningLEDs
        if(!ledcnt){PORTB=0;}  //ok with PB0 as input??

    } //infty
}//main

ISR(TIMER1_CAPT_vect){
    if(TCCR1B&(1<<ICES1)){         //1 is rising edge;
        TCNT1=0;
        TCCR1B&=~(1<<ICES1);       //switch to falling
    }else{
        capture_val=ICR1;
        TCCR1B|=(1<<ICES1);        //set back to rising edge
    }
}


