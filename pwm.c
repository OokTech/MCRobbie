/*
 * This file has everything that is required for the pwm part
 */

#include "parameters.h"

/*
 * This helper function lets you name a pin and set it as high (1) or low (0)
 */
void SetPin(char pin, char value) {
    switch (pin) {
        case 0:
            LATCbits.LC0 = value;
            break;
        case 1:
            LATCbits.LC1 = value;
            break;
        case 2:
            LATCbits.LC2 = value;
            break;
        case 3:
            LATCbits.LC3 = value;
            break;
        case 4:
            LATCbits.LC4 = value;
            break;
        case 5:
            LATCbits.LC5 = value;
            break;
        case 6:
            LATCbits.LC6 = value;
            break;
        case 7:
            LATCbits.LC7 = value;
            break;
        case 8:
            LATBbits.LB5 = value;
            break;
        case 9:
            LATBbits.LB7 = value;
            break;
        case 10:
            LATAbits.LA4 = value;
            break;
        case 11:
            LATAbits.LA5 = value;
            break;
    }
}

/*
 * This sets up Timer0 to be used by the pwm modules.
 * It also initialises the Motors array that holds the structs for the state of
 * each motor
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
    
    //Initialise all the motor structs
    int n;
    for (n = 0; n < 4; n++) {
        Motors[n].state = (unsigned char)0;
        Motors[n].paused = (unsigned char)0;
        Motors[n].enabled = (unsigned char)1;
        Motors[n].direction = (unsigned char)1;
        Motors[n].targetDirection = (unsigned char)1;
        Motors[n].duty = (unsigned char)0;
        Motors[n].target = (unsigned char)0;
        Motors[n].motorType = (unsigned char)0;
    }
    
    //Set which pins each motor uses, look at SetPin for definitions
    //The pins are picked so that the output on the physical chip makes sense
    //and is consistent across each output.
    Motors[0].PWMPin = 0;
    Motors[0].dirPin = 1;
    Motors[0].cdirPin = 2;
    
    Motors[1].PWMPin = 5;
    Motors[1].dirPin = 4;
    Motors[1].cdirPin = 3;
    
    Motors[2].PWMPin = 6;
    Motors[2].dirPin = 7;
    Motors[2].cdirPin = 9;
    
    Motors[3].PWMPin = 11;
    Motors[3].dirPin = 10;
    Motors[3].cdirPin = 8;
    
    //Set initial direction on the pins
    int i;
    for (i = 0; i < 4; i++) {
        SetPin(Motors[i].dirPin,HIGH);
        SetPin(Motors[i].cdirPin,LOW);
    }
}

/*
 * This function takes the current value and the minimum duty cycle as inputs 
 * and returns the rate of change at that point for the exponential growth or 
 * decay.
 * It isn't a very accurate curve, it should be updated. And maybe more segments
 * added.
 * It has different checks for growing and decaying because the check itself
 * is only based on the current value.
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
        //make sure that the change doesn't bring the speed lower than the 
        //target
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
        //Make sure that the change doesn't overshoot the target value
        if (target-current < change) {
            change = target-current;
        }
    }
    return change;
}

/*
 * This function makes the PWM go to zero, once it is at zero set the 
 * direction to targetDirection. If it is supposed to change direction this will
 * do it, if not than this just keeps things the same.
 */

void StopMotor(int index) {
    //If the motor has stopped and it is not set to the targetDirection, set the
    //motor to the target direction
    if (Motors[index].duty == 0 && Motors[index].direction != Motors[index].targetDirection) {
        //Set the direction flag
        Motors[index].direction = Motors[index].targetDirection;
        //Actually change the pin values.
        SetPin(Motors[index].dirPin, Motors[index].direction);
        SetPin(Motors[index].cdirPin, !Motors[index].direction);
    } else if (Motors[index].duty > 0) {
        //Slow the motor down using the desired acceleration profile
        //See AccelerateMotor function for descriptions of the acceleration 
        //types
        switch (AccelType) {
            case ACCEL_INSTANT:
                Motors[index].duty = 0;
                break;
            case ACCEL_LINEAR:
                if (Motors[index].duty > MinimumDuty) {
                    Motors[index].duty -= 1;
                } else {
                    Motors[index].duty = 0;
                }
                break;
            case ACCEL_EXPONENT:
                if (Motors[index].duty > MinimumDuty) {
                    Motors[index].duty -= ExponentialProfile(Motors[index].duty, MinimumDuty);
                } else {
                    Motors[index].duty = 0;
                }
                break;
            default:
                break;
        }
    }
}

/*
 * This changes the speed of a motor.
 */
void AccelerateMotor(int index) {
    if (Motors[index].duty < MinimumDuty && Motors[index].target >= MinimumDuty) {
        Motors[index].duty = MinimumDuty;
    } else if (Motors[index].duty <= MinimumDuty && Motors[index].target  < MinimumDuty) {
        Motors[index].duty = 0;
    }
    switch (AccelType) {
        case ACCEL_INSTANT:
            Motors[index].duty = Motors[index].target;
            break;
        case ACCEL_LINEAR:
            if (Motors[index].duty > Motors[index].target) {
                Motors[index].duty -= 1;
            } else if (Motors[index].duty < Motors[index].target) {
                Motors[index].duty += 1;
            }
            break;
        case ACCEL_EXPONENT:
            if (Motors[index].duty > Motors[index].target) {
                Motors[index].duty -= ExponentialProfile(Motors[index].duty, Motors[index].target);
            } else if (Motors[index].duty < Motors[index].target) {
                Motors[index].duty += ExponentialProfile(Motors[index].duty, Motors[index].target);
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
    int i;
    for (i = 0; i < 4; i++) {
        if (PWMPause || Motors[i].paused) {
            StopMotor(i);
        } else {
            //If the direction isn't equal to the targetDirection than reduce 
            //duty according to the current acceleration, otherwise if the duty 
            //isn't at the target accelerate
            if (Motors[i].direction != Motors[i].targetDirection) {
                StopMotor(i);
            } else if (Motors[i].duty != Motors[i].target) {
                AccelerateMotor(i);
            }
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
    int i;
    for (i = 0; i < 4; i++) {
        if (PWMEnable && Motors[i].enabled) {
            if (Motors[i].motorType) {
                if (TMR0 < Motors[i].duty && Motors[i].state == 0 && Motors[i].servoCount < 19) {
                    Motors[i].servoCount++;
                    Motors[i].state = 1;
                } else if (TMR0 < Motors[i].duty && Motors[i].state == 0 && Motors[i].servoCount == 20) {
                    Motors[i].state = 1;
                    SetPin(Motors[i].PWMPin,1);
                } else if (TMR0 >= Motors[i].duty && Motors[i].state == 1 && Motors[i].servoCount < 19) {
                    Motors[i].state = 0;
                } else if (TMR0 < Motors[i].duty && Motors[i].state == 0 && Motors[i].servoCount == 20) {
                    Motors[i].state = 0;
                    Motors[i].servoCount = 0;
                    SetPin(Motors[i].PWMPin,0);
                }
            } else {
                //Check if the right state should be changed
                if (TMR0 < Motors[i].duty && Motors[i].state == 0) {
                    //Set the state as high
                    Motors[i].state = 1;
                    //Set the pin as high
                    SetPin(Motors[i].PWMPin,1);
                } else if (TMR0 >= Motors[i].duty && Motors[i].state == 1) {
                    //Set the state as low
                    Motors[i].state = 0;
                    //Set the pin as low
                    SetPin(Motors[i].PWMPin,0);
                }
            }
        } else if (Motors[i].state) {
            //If it isn't enabled check the left and right state, turn off the PWM
            //on both if either is on.
            Motors[i].state = 0;
            SetPin(Motors[i].PWMPin,0);
        }
    }
    //Keep a count to see when we should update the pwm acceleration.
    if (AccelCount >= AccelRate) {
        AcceleratePWM();
        AccelCount = 0;
    } else {
        AccelCount++;
    }
}
