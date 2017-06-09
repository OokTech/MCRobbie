/*
 * File:   main.c
 * Author: inmysocks
 *
 * Created on May 31, 2017, 2:27 PM
 * This is going to be used as an i2c interface between a raspberry pi and motor
 * drivers.
 * 
 * 
 * This can control up to 4 separate motors using a shared pwm period.
 * To allow for 2 or 3 wire controllers each motor has 3 pins associated with it
 * a pwm pin, a dir pin and a cdir pin. Only the pwm and dir pins are used for
 * 2 wire controllers. Only the pwm pin is used when controlling a servo motor.
 * 
 * The 4 sets of pins available are:
 * 
 * Motor 1:
 * PWM: RC0
 * DIR: RC1
 * CDIR: RC2
 * 
 * Motor 2:
 * PWM: RC3
 * DIR: RC4
 * CDIR: RC5
 * 
 * Motor 3:
 * PWM: RC6
 * DIR: RB7
 * CDIR: RC7
 * 
 * Motor 4:
 * PWM: RA4
 * DIR: RA5
 * CDIR: RB5
 */


// PIC18F14K50 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1L
#pragma config CPUDIV = NOCLKDIV// CPU System Clock Selection bits (No CPU System Clock divide)
#pragma config USBDIV = OFF     // USB Clock Selection bit (USB clock comes directly from the OSC1/OSC2 oscillator block; no divide)

// CONFIG1H
#pragma config FOSC = IRC       // Oscillator Selection bits (Internal RC oscillator)
#pragma config PLLEN = OFF      // 4 X PLL Enable bit (PLL is under software control)
#pragma config PCLKEN = ON      // Primary Clock Enable bit (Primary clock enabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRTEN = OFF     // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 19        // Brown-out Reset Voltage bits (VBOR set to 1.9 V nominal)

// CONFIG2H
#pragma config WDTEN = OFF      // Watchdog Timer Enable bit (WDT is controlled by SWDTEN bit of the WDTCON register)
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config HFOFST = ON      // HFINTOSC Fast Start-up bit (HFINTOSC starts clocking the CPU without waiting for the oscillator to stablize.)
#pragma config MCLRE = OFF      // MCLR Pin Enable bit (RA3 input pin enabled; MCLR disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config BBSIZ = OFF      // Boot Block Size Select bit (1kW boot block size)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Table Write Protection bit (Block 0 not write-protected)
#pragma config WRT1 = OFF       // Table Write Protection bit (Block 1 not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include "parameters.h"

/*
 * This sets up the ports used by the motors and by the i2c
 */
void InitPorts(void) {
    //Set all port A pins to outputs
    TRISA = 0x00;
    //Set all port B pins to outputs aside from 4 and 6, which are needed by I2C
    TRISB = 0b01010000;
    //Set all port C pins to outputs
    TRISC = 0x00;
}

void main(void) {
    //This sets the internal oscillator to 16MHz
    OSCCONbits.IRCF = 0b111;
    
    //These set up the components
    InitPorts();
    InitI2C();
    InitPWM();
    
    //Every loop it checks the PWMs and updates as needed
    //I2C is interrupt driven so we don't need anything for it here.
    while(1) {
        CheckPWMOutput();
    }
    return;
}
