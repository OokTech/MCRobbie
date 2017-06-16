/* 
 * File: 
 * Author: inmysocks
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MOTOR_CONTROLLER_H
#define	MOTOR_CONTROLLER_H

#include <xc.h> // include processor files - each processor file is guarded.  

/*
 * By default the clock is running at, we set it to 16MHz in main.c
 * 
 * PWM_PERIOD is the period for the PWM in us, it should be about 2000us
 * since we are using a 1MHz clock we can use this as our target value for the 
 * timer.
 * 
 */

#define __XTAL_FREQUENCY 16000000 //hz
//#define PWM_PERIOD (0.002) //s
//#define TIMER_0_PRESCALER 16

//#define TIMER_0_PERIOD (PWM_PERIOD*__XTAL_FREQUENCY/TIMER_0_PRESCALER) //cycles

#define I2C_ADDRESS 0x23

#define HIGH 1
#define LOW 0

//Addresses for the different parts
#define SPEED_ADDRESS 1
#define MOTOR_0_SPEED_ADDRESS 2
#define MOTOR_1_SPEED_ADDRESS 3
#define MOTOR_2_SPEED_ADDRESS 4
#define MOTOR_3_SPEED_ADDRESS 5
#define MOTOR_TYPE_ADDRESS 6
#define MOTOR0_TYPE_ADDRESS 7
#define MOTOR1_TYPE_ADDRESS 8
#define MOTOR2_TYPE_ADDRESS 9
#define MOTOR3_TYPE_ADDRESS 10
#define DIRECTION_ADDRESS 11
#define MOTOR0_DIRECTION_ADDRESS 12
#define MOTOR1_DIRECTION_ADDRESS 13
#define MOTOR2_DIRECTION_ADDRESS 14
#define MOTOR3_DIRECTION_ADDRESS 15
#define PAUSE_ADDRESS 16
#define MOTOR0_PAUSE_ADDRESS 17
#define MOTOR1_PAUSE_ADDRESS 18
#define MOTOR2_PAUSE_ADDRESS 19
#define MOTOR3_PAUSE_ADDRESS 20
#define ENABLE_ADDRESS 21
#define MOTOR0_ENABLE_ADDRESS 22
#define MOTOR1_ENABLE_ADDRESS 23
#define MOTOR2_ENABLE_ADDRESS 24
#define MOTOR3_ENABLE_ADDRESS 25
#define ACCEL_ADDRESS 26
#define MOTOR0_ACCEL_TYPE_ADDRESS 27
#define MOTOR1_ACCEL_TYPE_ADDRESS 28
#define MOTOR2_ACCEL_TYPE_ADDRESS 29
#define MOTOR3_ACCEL_TYPE_ADDRESS 30
#define ACCEL_RATE_ADDRESS 31
#define MOTOR0_ACCEL_RATE_ADDRESS 32
#define MOTOR1_ACCEL_RATE_ADDRESS 33
#define MOTOR2_ACCEL_RATE_ADDRESS 34
#define MOTOR3_ACCEL_RATE_ADDRESS 35
#define MINIMUM_DUTY_ADDRESS 36
#define MOTOR0_MINIMUM_DUTY_ADDRESS 37
#define MOTOR1_MINIMUM_DUTY_ADDRESS 38
#define MOTOR2_MINIMUM_DUTY_ADDRESS 39
#define MOTOR3_MINIMUM_DUTY_ADDRESS 40

//Different acceleration types
#define ACCEL_INSTANT 0
#define ACCEL_LINEAR 1
#define ACCEL_EXPONENT 2

//Motor type definitions, there are two unused options for later when we add
//linear actuators and stepper motors
#define MOTOR_TYPE_DC 0
#define MOTOR_TYPE_SERVO 1

//Enable boolean for PWM outputs, if this is set to 0 than all motors will stop
//immediately, ignoring acceleration.
unsigned char PWMEnable = 1;

//This is the pause state, when this is 1 the motors will slow to stopped and 
//will not speed up regardless of what the speed is set to.
unsigned char PWMPause = 0;

//This is used to set the different types of acceleration.
unsigned char AccelType = ACCEL_INSTANT;
//This value controls the rate of acceleration.
unsigned int AccelRate = 150;
//This is used as a counter for the acceleration.
unsigned int AccelCount = 0;
//This keeps track of the minimum duty cycle needed to make a motor move.
unsigned char MinimumDuty = 0;

//Function prototypes
void InitI2C(void);
void InitPWM(void);
void CheckPWMOutput(void);

//This defines the struct that is used to hold information about each one of the
//motors
struct Motor {
    unsigned char state:1;
    unsigned char enabled:1;
    unsigned char paused:1;
    unsigned char direction:1;
    unsigned char targetDirection:1;
    unsigned char motorType;
    unsigned char PWMPin;
    unsigned char dirPin;
    unsigned char cdirPin;
    unsigned char duty;
    unsigned char target;
    unsigned char servoCount;
    unsigned char accelType;
    unsigned char accelRate;
    unsigned char minimumDuty;
    unsigned char accelCount;
};

//This is the actual array of Motor structs
struct Motor Motors[4];

#endif	/* MOTOR_CONTROLLER_H */

