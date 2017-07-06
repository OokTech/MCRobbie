/*
 * file: i2c.c
 * author: inmysocks (inmysocks@fastmail.com)
 * 
 * Copyright 2017 OokTech
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 * 
 * This file has the parts that make the i2c interface work
 */

#include "parameters.h"

void InitI2C(void) {
    //Enable internal pull-ups
    INTCON2bits.NOT_RABPU = 0;
    //Turn on pull-ups for RB4 and RB6
    WPUBbits.WPUB4 = 1;
    WPUBbits.WPUB6 = 1;
   
    //Slew rate control disabled, I don't know if this is needed. Test it.
    SSPSTATbits.SMP = 1;
    
    //Set the mode to i2c slave without interrupts enabled for start and stop bits
    SSPCON1bits.SSPM = 0b0110;
    //Enable clock stretching
    SSPCON2bits.SEN = 1;
    //Disable clock stretching
    //SSPCON2bits.SEN = 0;
    
    //Set the bitmask for the address so that the controller responds to 
    //multiple addresses.
    //SSPMSKbits.MSK = 0b01000110;
    
    //This is the i2c address of the controller. It should be between 0x00 and 0x7F
    //The 7 bit address is 0x23, but it needs to be shifted to the left by one 
    //because SSPADD is an 8 bit register and the address is stored in bits<7:1>
    SSPADD = I2C_ADDRESS<<1;
    
    //Clear the interrupt flag to ensure that it is cleared to start.
    PIR1bits.SSPIF = 0;
    //Enable i2c interrupts
    PIE1bits.SSPIE = 1;
    
    //Globally enable interrupts
    INTCONbits.GIE = 1;
    //Enable prepherial interrupts
    INTCONbits.PEIE = 1;
    
    //Enable the module
    SSPCON1bits.SSPEN = 1;   
}

/**
 * 
 * 
 * Relevant registers for the I2C module:
 * SSPCON1
 * SSPCON2
 * SSPSTAT
 * SSPBUF
 * SSPADD
 * SSPMSK
 * 
 * Tasks to initialise the I2C module:
 * 
 * Set up the i2c pins as inputs pin 11 (RB6) is SCL and pin 13 (RB4) is SDA
 * 
 * Set SSPEN in register SSPCON1
 * 
 * Set up the bitmask to allow multiple addresses on the same chip in register
 * SSPMSK. 
 * Ignore this for now, when we start using multiple things through the same 
 * chip than we will use it. Only the upper 7 bits have any effect
 * 
 * Set the address of this controller using SSPADD
 * The I2C address is 0x23. The highest 7 bits of SSPADD are the ones used.
 * 
 * 
 */

//This holds the most recent information in the i2c buffer
unsigned char currentByte = 0;
unsigned char state = 0;

unsigned char readOrWrite = 0;

unsigned int i;

void ReadI2CByte(void) {
    if (state) {
        switch (state) {
            case SPEED_ADDRESS:
                SSPBUF = Motors[0].duty;
                break;
            case MOTOR_0_SPEED_ADDRESS:
                SSPBUF = Motors[0].duty;
                break;
            case MOTOR_1_SPEED_ADDRESS:
                SSPBUF = Motors[1].duty;
                break;
            case MOTOR_2_SPEED_ADDRESS:
                SSPBUF = Motors[2].duty;
                break;
            case MOTOR_3_SPEED_ADDRESS:
                SSPBUF = Motors[3].duty;
                break;
            case MOTOR_TYPE_ADDRESS:
                //Each motor has two bits to determine the motor type
                SSPBUF = (unsigned)((Motors[0].motorType & 0b00000011) | (Motors[1].motorType & 0b00000011)<<2 | (Motors[2].motorType & 0b00000011)<<4 | (Motors[3].motorType & 0b00000011)<<6);
                break;
            case MOTOR0_TYPE_ADDRESS:
                SSPBUF = Motors[0].motorType;
                break;
            case MOTOR1_TYPE_ADDRESS:
                SSPBUF = Motors[1].motorType;
                break;
            case MOTOR2_TYPE_ADDRESS:
                SSPBUF = Motors[2].motorType;
                break;
            case MOTOR3_TYPE_ADDRESS:
                SSPBUF = Motors[3].motorType;
                break;
            case DIRECTION_ADDRESS:
                SSPBUF = (unsigned)((Motors[0].direction & 0b00000011) | (Motors[1].direction & 0b00000011)<<2 | (Motors[2].direction & 0b00000011)<<4 | (Motors[3].direction & 0b00000011)<<6);
                break;
            case MOTOR0_DIRECTION_ADDRESS:
                SSPBUF = Motors[0].direction;
                break;
            case MOTOR1_DIRECTION_ADDRESS:
                SSPBUF = Motors[1].direction;
                break;
            case MOTOR2_DIRECTION_ADDRESS:
                SSPBUF = Motors[2].direction;
                break;
            case MOTOR3_DIRECTION_ADDRESS:
                SSPBUF = Motors[3].direction;
                break;
            case ENABLE_ADDRESS:
                SSPBUF = PWMEnable;
                break;
            case MOTOR0_ENABLE_ADDRESS:
                SSPBUF = Motors[0].enabled;
                break;
            case MOTOR1_ENABLE_ADDRESS:
                SSPBUF = Motors[1].enabled;
                break;
            case MOTOR2_ENABLE_ADDRESS:
                SSPBUF = Motors[2].enabled;
                break;
            case MOTOR3_ENABLE_ADDRESS:
                SSPBUF = Motors[3].enabled;
                break;
            case PAUSE_ADDRESS:
                SSPBUF = PWMPause;
                break;
            case MOTOR0_PAUSE_ADDRESS:
                SSPBUF = Motors[0].paused;
                break;
            case MOTOR1_PAUSE_ADDRESS:
                SSPBUF = Motors[1].paused;
                break;
            case MOTOR2_PAUSE_ADDRESS:
                SSPBUF = Motors[2].paused;
                break;
            case MOTOR3_PAUSE_ADDRESS:
                SSPBUF = Motors[3].paused;
                break;
            case ACCEL_ADDRESS:
                SSPBUF = Motors[0].accelType;
                break;
            case MOTOR0_ACCEL_TYPE_ADDRESS:
                SSPBUF = Motors[0].accelType;
                break;
            case MOTOR1_ACCEL_TYPE_ADDRESS:
                SSPBUF = Motors[1].accelType;
                break;
            case MOTOR2_ACCEL_TYPE_ADDRESS:
                SSPBUF = Motors[2].accelType;
                break;
            case MOTOR3_ACCEL_TYPE_ADDRESS:
                SSPBUF = Motors[3].accelType;
                break;
            case ACCEL_RATE_ADDRESS:
                SSPBUF = Motors[0].accelRate;
                break;
            case MOTOR0_ACCEL_RATE_ADDRESS:
                SSPBUF = Motors[0].accelRate;
                break;
            case MOTOR1_ACCEL_RATE_ADDRESS:
                SSPBUF = Motors[1].accelRate;
                break;
            case MOTOR2_ACCEL_RATE_ADDRESS:
                SSPBUF = Motors[2].accelRate;
                break;
            case MOTOR3_ACCEL_RATE_ADDRESS:
                SSPBUF = Motors[3].accelRate;
                break;
            case MINIMUM_DUTY_ADDRESS:
                SSPBUF = Motors[0].minimumDuty;
                break;
            case MOTOR0_MINIMUM_DUTY_ADDRESS:
                SSPBUF = Motors[0].minimumDuty;
                break;
            case MOTOR1_MINIMUM_DUTY_ADDRESS:
                SSPBUF = Motors[1].minimumDuty;
                break;
            case MOTOR2_MINIMUM_DUTY_ADDRESS:
                SSPBUF = Motors[2].minimumDuty;
                break;
            case MOTOR3_MINIMUM_DUTY_ADDRESS:
                SSPBUF = Motors[3].minimumDuty;
                break;
            case MOTOR0_TARGET_ADDRESS:
                SSPBUF = Motors[0].target;
                break;
            case MOTOR1_TARGET_ADDRESS:
                SSPBUF = Motors[1].target;
                break;
            case MOTOR2_TARGET_ADDRESS:
                SSPBUF = Motors[2].target;
                break;
            case MOTOR3_TARGET_ADDRESS:
                SSPBUF = Motors[3].target;
                break;
            default:
                //Send 255 whenever an invalid read is requested
                SSPBUF = 0xFF;
                break;
        }
    } else {
        //Send 255 whenever an invalid read is requested
        SSPBUF = 0xFF;
    }
}

/*
 * This is the ISR (Interrupt Service Routine) that is called whenever an 
 * interrupt is triggered. With the pic18 we are using there are multiple 
 * interrupt sources, but at most a high and low priority interrupt service
 * routine. So when it is called you need to check the source of the interrupt
 * to see what to do. Priority of the interrupts is handled by if else 
 * statements.
 * 
 * For now we only have one interrupt enabled, but checking is still good 
 * practice unless you have some great need to shave off ever cycle.
 */
void interrupt I2C_Slave_Read(void)
{
    if(PIR1bits.SSPIF == 1) {
        //hold the clock
        SSPCON1bits.CKP = 0;
        //We always want to read the buffer to clear it and use the data
        currentByte = SSPBUF;
        //Handle errors by throwing everything away
        if ((SSPCON1bits.SSPOV) || (SSPCON1bits.WCOL)) {
            // Clear the overflow flag
            SSPCON1bits.SSPOV = 0;
            // Clear the collision bit
            SSPCON1bits.WCOL = 0;
        } else if(!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {
            //When we have received an address byte that is set to 'write'
            //Set the state to 0 so we know to get the next byte as the state
            state = 0;
        } else if (SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {
            //When we receive a data byte that is set to 'write'
            if (state == 0) {
                //If we don't know what we have yet than the byte is the address 
                //to write to. We are calling this the state.
                state = currentByte;                
            } else {
                //If we have a non-zero state than we set the correct values
                //Check what address the state matches and set the values 
                //accordingly.
                switch (state) {
                    case SPEED_ADDRESS:
                        for (i = 0; i < 4; i++) {
                            if (currentByte < Motors[i].minimumDuty) {
                                Motors[i].target = 0;
                            } else {
                                Motors[i].target = currentByte;
                            }
                        }
                        break;
                    case MOTOR_0_SPEED_ADDRESS:
                        if (currentByte < Motors[0].minimumDuty) {
                            Motors[0].target = 0;
                        } else {
                            Motors[0].target = currentByte;
                        }
                        break;
                    case MOTOR_1_SPEED_ADDRESS:
                        if (currentByte < Motors[1].minimumDuty) {
                            Motors[1].target = 0;
                        } else {
                            Motors[1].target = currentByte;
                        }
                        break;
                    case MOTOR_2_SPEED_ADDRESS:
                        if (currentByte < Motors[2].minimumDuty) {
                            Motors[2].target = 0;
                        } else {
                            Motors[2].target = currentByte;
                        }
                        break;
                    case MOTOR_3_SPEED_ADDRESS:
                        if (currentByte < Motors[3].minimumDuty) {
                            Motors[3].target = 0;
                        } else {
                            Motors[3].target = currentByte;
                        }
                        break;
                    case MOTOR_TYPE_ADDRESS:
                        //Each motor has two bits to determine the motor type
                        for (i = 0; i < 4; i++) {
                            Motors[i].motorType = (unsigned) (currentByte>>(2*i)) & 0b00000011;
                        }
                        break;
                    case MOTOR0_TYPE_ADDRESS:
                        Motors[0].motorType = currentByte;
                        break;
                    case MOTOR1_TYPE_ADDRESS:
                        Motors[1].motorType = currentByte;
                        break;
                    case MOTOR2_TYPE_ADDRESS:
                        Motors[2].motorType = currentByte;
                        break;
                    case MOTOR3_TYPE_ADDRESS:
                        Motors[3].motorType = currentByte;
                        break;
                    case DIRECTION_ADDRESS:
                        for (i = 0; i < 4; i++) {
                            if ((currentByte>>(2*i)) & 0b00000011) {
                                Motors[i].targetDirection = 1;
                            } else {
                                Motors[i].targetDirection = 0;
                            }
                        }
                        break;
                    case MOTOR0_DIRECTION_ADDRESS:
                        Motors[0].targetDirection = currentByte;
                        break;
                    case MOTOR1_DIRECTION_ADDRESS:
                        Motors[1].targetDirection = currentByte;
                        break;
                    case MOTOR2_DIRECTION_ADDRESS:
                        Motors[2].targetDirection = currentByte;
                        break;
                    case MOTOR3_DIRECTION_ADDRESS:
                        Motors[3].targetDirection = currentByte;
                        break;
                    case ENABLE_ADDRESS:
                        PWMEnable = currentByte;
                        break;
                    case MOTOR0_ENABLE_ADDRESS:
                        Motors[0].enabled = currentByte;
                        break;
                    case MOTOR1_ENABLE_ADDRESS:
                        Motors[1].enabled = currentByte;
                        break;
                    case MOTOR2_ENABLE_ADDRESS:
                        Motors[2].enabled = currentByte;
                        break;
                    case MOTOR3_ENABLE_ADDRESS:
                        Motors[3].enabled = currentByte;
                        break;
                    case PAUSE_ADDRESS:
                        PWMPause = currentByte;
                        break;
                    case MOTOR0_PAUSE_ADDRESS:
                        Motors[0].paused = currentByte;
                        break;
                    case MOTOR1_PAUSE_ADDRESS:
                        Motors[1].paused = currentByte;
                        break;
                    case MOTOR2_PAUSE_ADDRESS:
                        Motors[2].paused = currentByte;
                        break;
                    case MOTOR3_PAUSE_ADDRESS:
                        Motors[3].paused = currentByte;
                        break;
                    case ACCEL_ADDRESS:
                        for (i = 0; i < 4; i++) {
                            Motors[i].accelType = currentByte;
                        }
                        break;
                    case MOTOR0_ACCEL_TYPE_ADDRESS:
                        Motors[0].accelType = currentByte;
                        break;
                    case MOTOR1_ACCEL_TYPE_ADDRESS:
                        Motors[1].accelType = currentByte;
                        break;
                    case MOTOR2_ACCEL_TYPE_ADDRESS:
                        Motors[2].accelType = currentByte;
                        break;
                    case MOTOR3_ACCEL_TYPE_ADDRESS:
                        Motors[3].accelType = currentByte;
                        break;
                    case ACCEL_RATE_ADDRESS:
                        for (i = 0; i < 4; i++) {
                            Motors[i].accelRate = currentByte;
                        }
                        break;
                    case MOTOR0_ACCEL_RATE_ADDRESS:
                        Motors[0].accelRate = currentByte;
                        break;
                    case MOTOR1_ACCEL_RATE_ADDRESS:
                        Motors[1].accelRate = currentByte;
                        break;
                    case MOTOR2_ACCEL_RATE_ADDRESS:
                        Motors[2].accelRate = currentByte;
                        break;
                    case MOTOR3_ACCEL_RATE_ADDRESS:
                        Motors[3].accelRate = currentByte;
                        break;
                    case MINIMUM_DUTY_ADDRESS:
                      for (i = 0; i < 4; i++) {
                          Motors[i].minimumDuty = currentByte;
                      }
                      break;
                    case MOTOR0_MINIMUM_DUTY_ADDRESS:
                        Motors[0].minimumDuty = currentByte;
                        break;
                    case MOTOR1_MINIMUM_DUTY_ADDRESS:
                        Motors[1].minimumDuty = currentByte;
                        break;
                    case MOTOR2_MINIMUM_DUTY_ADDRESS:
                        Motors[2].minimumDuty = currentByte;
                        break;
                    case MOTOR3_MINIMUM_DUTY_ADDRESS:
                        Motors[3].minimumDuty = currentByte;
                        break;
                }
                //increment the state to allow for writing multiple bytes
                state += 1;
            }
        } else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW) {
            //readOrWrite = 1;
            //We are going to read from the controller, so send the byte
            //determined by state, which was set by the previous write
            ReadI2CByte();
        } else if (SSPSTATbits.D_nA && SSPSTATbits.R_nW) {
            //this is for reading multiple bytes in sequence
            //increment the state
            state += 1;
            //send the next byte
            ReadI2CByte();
        }
        //release the clock
        SSPCON1bits.CKP = 1;
        //wait until the buffer is cleared
        while(SSPSTATbits.BF);
    }
    //Clear interrupt flag
    PIR1bits.SSPIF = 0;
}
