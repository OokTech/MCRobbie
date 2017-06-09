/*
 * This file has the parts that make the i2c stuff work
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

int i;

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
        //We always want to read the buffer to clear it and use the data
        currentByte = SSPBUF;
        //hold the clock
        SSPCON1bits.CKP = 0;
        //Handle errors by throwing everything away
        if ((SSPCON1bits.SSPOV) || (SSPCON1bits.WCOL)) {
            // Clear the overflow flag
            SSPCON1bits.SSPOV = 0;
            // Clear the collision bit
            SSPCON1bits.WCOL = 0;
            //release the clock
            SSPCON1bits.CKP = 1;
        } else if(!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {
            //Set the state to 0 so we know to get the next byte as the state
            state = 0;
            //release the clock
            SSPCON1bits.CKP = 1;
        } else if (SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {
            if (state == 0) {
                //the byte is what type of data is about to be sent
                state = currentByte;                
            } else {
                //If we have a non-zero state than we set the correct values
                switch (state) {
                    case SPEED_ADDRESS:
                        for (i = 0; i < 4; i++) {
                            if (currentByte < MinimumDuty) {
                                Motors[i].target = 0;
                            } else {
                                Motors[i].target = currentByte;
                            }
                        }
                        break;
                    case MOTOR_0_SPEED_ADDRESS:
                        if (currentByte < MinimumDuty) {
                            Motors[0].target = 0;
                        } else {
                            Motors[0].target = currentByte;
                        }
                        break;
                    case MOTOR_1_SPEED_ADDRESS:
                        if (currentByte < MinimumDuty) {
                            Motors[1].target = 0;
                        } else {
                            Motors[1].target = currentByte;
                        }
                        break;
                    case MOTOR_2_SPEED_ADDRESS:
                        if (currentByte < MinimumDuty) {
                            Motors[2].target = 0;
                        } else {
                            Motors[2].target = currentByte;
                        }
                        break;
                    case MOTOR_3_SPEED_ADDRESS:
                        if (currentByte < MinimumDuty) {
                            Motors[3].target = 0;
                        } else {
                            Motors[3].target = currentByte;
                        }
                        break;
                    case DIRECTION_ADDRESS:
                        for (i = 0; i < 4; i++) {
                            if ((currentByte>>(i+1)) & 0b00000001) {
                                Motors[i].targetDirection = 1;
                            } else {
                                Motors[i].targetDirection = 0;
                            }
                        }
                        break;
                    case ENABLE_ADDRESS:
                        PWMEnable = (currentByte & GLOBAL_MASK);
                        for (i = 0; i < 4; i++) {
                            if ((currentByte>>(i+1)) & 0b00000001) {
                                Motors[i].enabled = 1;
                            } else {
                                Motors[i].enabled = 0;
                            }
                        }
                        break;
                    case PAUSE_ADDRESS:
                        PWMPause = currentByte & GLOBAL_MASK;
                        for (i = 0; i < 4; i++) {
                            if ((currentByte>>(i+1)) & 0b00000001) {
                                Motors[i].paused = 1;
                            } else {
                                Motors[i].paused = 0;
                            }
                        }
                        break;
                    case ACCEL_ADDRESS:
                        AccelType = currentByte;
                        break;
                    case ACCEL_RATE_ADDRESS:
                        AccelRate = currentByte;
                        break;
                    case MINIMUM_DUTY_ADDRESS:
                        MinimumDuty = currentByte;
                        break;
                }
                state = 0;
            }
            SSPCON1bits.CKP = 1;
        } else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW) {
            /*
            if (currentByte>>1 == SPEED_ADDRESS) {
                SSPBUF = LeftPWM.duty;
            } else if (currentByte>>1 == LEFT_ADDRESS) {
                SSPBUF = LeftPWM.duty;
            } else if (currentByte>>1 == RIGHT_ADDRESS) {
                SSPBUF = RightPWM.duty;
            } else if (currentByte>>1 == SETTINGS_ADDRESS) {
                SSPBUF = PWMEnable;
            }
            */  
            //release the clock
            SSPCON1bits.CKP = 1;
            //wait until the buffer is cleared
            while(SSPSTATbits.BF);
        }
        //Clear interrupt flag
        PIR1bits.SSPIF = 0;
    }
}
