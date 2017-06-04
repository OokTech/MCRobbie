/*
 * This file has everything that is required for the pwm part
 */

#include "parameters.h"

/*
 * This sets up Timer0 to be used by the pwm modules.
 */
void InitPWM(void) {
    //Use a 1:16 prescaler value
    T0CONbits.T0PS = 0b011;
    //Use prescaler
    T0CONbits.PSA = 0;
    //Use internal instruction clock
    T0CONbits.T0CS = 0;
    //Make Timer0 a 16 bit counter
    T0CONbits.T08BIT = 1;
    //Turn on Timer0
    T0CONbits.TMR0ON = 1;
    
    //We are using RA4 and RA5 as the pwm output pins
    //Configure them as outputs
    TRISAbits.RA4 = 0;
    TRISAbits.RA5 = 0;
    //Initialize the pins as off
    LATAbits.LATA4 = 0;
    LATAbits.LATA5 = 0;
    
    LeftPWM.state = 0;
    LeftPWM.paused = 0;
    LeftPWM.enabled = 1;
    LeftPWM.direction = 1;
    LeftPWM.targetDirection = 1;
    LeftPWM.duty = 0;
    LeftPWM.target = 0;
    
    RightPWM.state = 0;
    RightPWM.paused = 0;
    RightPWM.enabled = 1;
    RightPWM.direction = 1;
    RightPWM.targetDirection = 1;
    RightPWM.duty = 0;
    RightPWM.target = 0;
}

/*
 * This function takes the current value as input and returns the rate of change
 * at that point for the exponential growth and decay.
 */
char ExponentialProfile(unsigned char current, unsigned char target) {
    unsigned char change = 0;
    if (current > target) {
        //decay
        if (current-MinimumDuty > 200) {
            change = 50;
        } else if (current-MinimumDuty > 150) {
            change = 25;
        } else if (current-MinimumDuty > 100) {
            change = 20;
        } else if (current-MinimumDuty > 75) {
            change = 10;
        } else if (current-MinimumDuty > 50) {
            change = 5;
        } else {
            change = 1;
        }
        if (current-target < change) {
            change = current-target;
        }
    } else {
        //growth
        if (current-MinimumDuty > 200) {
            change = current-target;
        } else if (current-MinimumDuty > 150) {
            change = 25;
        } else if (current-MinimumDuty > 100) {
            change = 20;
        } else if (current-MinimumDuty > 75) {
            change = 10;
        } else if (current-MinimumDuty > 50) {
            change = 5;
        } else {
            change = 1;
        }
        if (target-current < change) {
            change = target-current;
        }
    }
    return change;
}

/*
 * This function makes the left PWM go to zero, once it is at zero set the 
 * direction to targetDirection. If it is supposed to change direction this will
 * do it, if not than this just keeps things the same.
 */
void StopLeft(void) {
    //If the duty is already 0 than update direction, otherwise slow down
    //according to the selected acceleration profile.
    if (LeftPWM.duty == 0 && LeftPWM.direction != LeftPWM.targetDirection) {
        LeftPWM.direction = LeftPWM.targetDirection;
        LATCbits.LATC4 = LeftPWM.direction;
        LATCbits.LATC5 = !LeftPWM.direction;
    } else {
        switch (AccelType) {
            case ACCEL_INSTANT:
                LeftPWM.duty = 0;
                break;
            case ACCEL_LINEAR:
                if (LeftPWM.duty > MinimumDuty) {
                    LeftPWM.duty -= 1;
                } else {
                    LeftPWM.duty = 0;
                }
                break;
            case ACCEL_EXPONENT:
                if (MinimumDuty < LeftPWM.duty) {
                    LeftPWM.duty -= ExponentialProfile(LeftPWM.duty, MinimumDuty);
                } else {
                    LeftPWM.duty = 0;
                }
                break;
            default:
                break;
        }
    }
}

void StopRight(void) {
    //If the duty is already 0 than update direction, otherwise slow down
    //according to the selected acceleration profile.
    if (RightPWM.duty == 0 && RightPWM.direction != RightPWM.targetDirection) {
        RightPWM.direction = RightPWM.targetDirection;
        LATCbits.LATC3 = RightPWM.direction;
        LATCbits.LATC2 = !RightPWM.direction;
    } else {
        switch (AccelType) {
            case ACCEL_INSTANT:
                RightPWM.duty = 0;
                break;
            case ACCEL_LINEAR:
                if (RightPWM.duty > MinimumDuty) {
                    RightPWM.duty -= 1;
                } else {
                    RightPWM.duty = 0;
                }
                break;
            case ACCEL_EXPONENT:
                if (MinimumDuty < RightPWM.duty) {
                    RightPWM.duty -= ExponentialProfile(RightPWM.duty, MinimumDuty);
                } else {
                    RightPWM.duty = 0;
                }
                break;
            default:
                break;
        }
    }
}

void AccelerateLeft(void) {
    if (LeftPWM.duty < MinimumDuty && LeftPWM.target >= MinimumDuty) {
        LeftPWM.duty = MinimumDuty;
    } else if (LeftPWM.duty <= MinimumDuty && LeftPWM.target  < MinimumDuty) {
        LeftPWM.duty = 0;
    }
    switch (AccelType) {
        case ACCEL_INSTANT:
            LeftPWM.duty = LeftPWM.target;
            break;
        case ACCEL_LINEAR:
            if (LeftPWM.duty > LeftPWM.target) {
                LeftPWM.duty -= 1;
            } else if (LeftPWM.duty < LeftPWM.target) {
                LeftPWM.duty += 1;
            }
            break;
        case ACCEL_EXPONENT:
            if (LeftPWM.duty > LeftPWM.target) {
                LeftPWM.duty -= ExponentialProfile(LeftPWM.duty, LeftPWM.target);
            } else if (LeftPWM.duty < LeftPWM.target) {
                LeftPWM.duty += ExponentialProfile(LeftPWM.duty, LeftPWM.target);
            }
            break;
        default:
            break;
    }
}

void AccelerateRight(void) {
    if (RightPWM.duty < MinimumDuty && RightPWM.target >= MinimumDuty) {
        RightPWM.duty = MinimumDuty;
    } else if (RightPWM.duty <= MinimumDuty && RightPWM.target  < MinimumDuty) {
        RightPWM.duty = 0;
    }
    switch (AccelType) {
        case ACCEL_INSTANT:
            RightPWM.duty = RightPWM.target;
            break;
        case ACCEL_LINEAR:
            if (RightPWM.duty > RightPWM.target) {
                RightPWM.duty -= 1;
            } else if (RightPWM.duty < RightPWM.target) {
                RightPWM.duty += 1;
            }
            break;
        case ACCEL_EXPONENT:
            if (RightPWM.duty > RightPWM.target) {
                RightPWM.duty -= ExponentialProfile(RightPWM.duty, RightPWM.target);
            } else if (RightPWM.duty < RightPWM.target) {
                RightPWM.duty += ExponentialProfile(RightPWM.duty, RightPWM.target);
            }
            break;
        default:
            break;
    }    
}

/*
 * This function checks the current value and the target value, if they are 
 * different it applies the selected acceleration profile to change the values.
 * 
 * This is also where soft pauses are implemented. If PWMPause is set than the
 * pwm slows down and stops the same as if the speed were set to 0;
 */
void AcceleratePWM(void) {
    if (PWMPause) {
        StopLeft();
        StopRight();
    } else {
        //If the direction isn't equal to the targetDirection than reduce duty 
        //according to the current acceleration, otherwise if the duty isn't at the
        //target accelerate
        if (LeftPWM.direction != LeftPWM.targetDirection) {
            StopLeft();
        } else if (LeftPWM.duty != LeftPWM.target) {
            AccelerateLeft();
        }

        if (RightPWM.direction != RightPWM.targetDirection) {
            StopRight();
        } else if (RightPWM.duty != RightPWM.target) {
            AccelerateRight();
        }
    }
}

/*
 * We are going to make out own pwm using timers because this only has one real
 * pwm module.
 * So we just have the start and stop times for the left and right, and we 
 * toggle a digital pin state based on that.
 * We have two, one for left and one for right.
 * 
 * To make this work we have a single timer (Timer0) that gives us the period of
 * the generated pwm signal. Then we have two variables that control the duty
 * cycle for the left and right outputs.
 * 
 * Each time this function is called it checks to see if the value of Timer0 is 
 * greater than the value in either of the variables. When the value is less
 * than the output pin is high, when the value is greater than the output pin is
 * low.
 * To prevent glitches we are going to have a state variable that we check
 * against that lets us know the value without having to read the pin. 
 */
void CheckPWMOutput(void) {
    //Check to make sure that the PWM in enabled
    if (PWMEnable) {
        if (LeftPWM.enabled) {
            //Check if the right state should be changed
            if (TMR0 < LeftPWM.duty && LeftPWM.state == 0) {
                //Set the state as high
                LeftPWM.state = 1;
                //Set the pin as high
                LATAbits.LATA4 = 1;
                
                LATCbits.LATC1 = 1;
            } else if (TMR0 >= LeftPWM.duty && LeftPWM.state == 1) {
            //} else if (TMR0 > ((TIMER_0_PERIOD*leftDuty)/256) && leftState == 1) {
                //Set the state as low
                LeftPWM.state = 0;
                //Set the pin as low
                LATAbits.LATA4 = 0;
                
                LATCbits.LATC1 = 0;
            }
        }
        
        //Check if the left state should be changed
        if (TMR0 < RightPWM.duty && RightPWM.state == 0) {
            RightPWM.state = 1;
            //Set the pin as high
            LATAbits.LATA5 = 1;
            
            LATCbits.LATC0 = 1;
        } else if (TMR0 >= RightPWM.duty && RightPWM.state == 1) {
            RightPWM.state = 0;
            //Set the pin as low
            LATAbits.LATA5 = 0;
            
            LATCbits.LATC0 = 0;
        }
    } else if (RightPWM.state || LeftPWM.state) {
        //If it isn't enabled check the left and right state, turn off the PWM
        //on both if either is on.
        LATAbits.LATA4 = 0;
        LATAbits.LATA5 = 0;
        RightPWM.state = 0;
        LeftPWM.state = 0;
    }
    //After the timer runs out reset it, and check if any values need to be 
    //updated for the PWMs. This means that the updates happen at known 
    //intervals so we can do the acceleration properly.
    //if (TMR0 >= 2048) {
    //    TMR0 = 0x00;
        if (AccelCount >= AccelRate) {
            AcceleratePWM();
            AccelCount = 0;
        } else {
            AccelCount++;
        }
    //}
}
