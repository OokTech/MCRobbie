/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MOTOR_CONTROLLER_H
#define	MOTOR_CONTROLLER_H

#include <xc.h> // include processor files - each processor file is guarded.  

/*
 * By default the clock is running an 1MHz
 * 
 * PWM_PERIOD is the period for the PWM in us, it should be about 2000us
 * since we are using a 1MHz clock we can use this as our target value for the 
 * timer.
 * 
 */

#define __XTAL_FREQUENCY 16000000 //hz
#define PWM_PERIOD (0.002) //s
#define TIMER_0_PRESCALER 16

#define TIMER_0_PERIOD (PWM_PERIOD*__XTAL_FREQUENCY/TIMER_0_PRESCALER) //cycles

#define I2C_ADDRESS 0x23

//Addresses for the different parts
#define SPEED_ADDRESS 1
#define LEFT_ADDRESS 2
#define RIGHT_ADDRESS 3
#define SETTINGS_ADDRESS 4
#define DIRECTION_ADDRESS 5
#define PAUSE_ADDRESS 6
#define ENABLE_ADDRESS 7
#define ACCEL_ADDRESS 8
#define ACCEL_RATE_ADDRESS 9
#define MINIMUM_DUTY_ADDRESS 10


//bitmasks for different parts of the settings bytes
#define PWM_ENABLE 0b00000001
#define ACCEL_TYPE 0b00000110
#define GLOBAL_MASK 0b00000001
#define LEFT_MASK 0b00000010
#define RIGHT_MASK 0b00000100

#define ACCEL_INSTANT 0
#define ACCEL_LINEAR 1
#define ACCEL_EXPONENT 2

//Enable boolean for PWM outputs
unsigned char PWMEnable = 1;
//This is the pause state, when this is 1 the motors will slow to stopped and 
//will not speed up regardless of what the speed is set to.
unsigned char PWMPause = 0;

//This is used to set the different types of acceleration
unsigned char AccelType = ACCEL_EXPONENT;
unsigned int AccelRate = 1000;
unsigned int AccelCount = 0;

unsigned char MinimumDuty = 0;

void InitI2C(void);
void InitPWM(void);
void CheckPWMOutput(void);

struct PWM {
    unsigned char state:1;
    unsigned char enabled:1;
    unsigned char paused:1;
    unsigned char direction:1;
    unsigned char targetDirection:1;
    unsigned char duty;
    unsigned char target;
};

struct PWM LeftPWM;
struct PWM RightPWM;

#endif	/* MOTOR_CONTROLLER_H */

