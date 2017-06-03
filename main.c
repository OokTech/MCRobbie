/*
 * File:   main.c
 * Author: inmysocks
 *
 * Created on May 31, 2017, 2:27 PM
 * This is going to be used as an i2c interface between a raspberry pi and a set 
 * of sensors.
 * Each sensor is going to use a different i2c address so that they can be 
 * handled by separate controllers if we need that in the future.
 * I would like to make the controller configurable at runtime so that you can 
 * set which sensors it uses while it is running.
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


/*
 * By default the clock is running an 1MHz
 * 
 * PWM_PERIOD is the period for the PWM in us, it should be about 2000us
 * since we are using a 1MHz clock we can use this as our target value for the 
 * timer.
 * 
 */

#define __XTAL_FREQUENCY 1000000 //hz
#define PWM_PERIOD (0.002) //s

#define TIMER_0_PERIOD (PWM_PERIOD*__XTAL_FREQUENCY) //cycles

//This needs to take TIMER_0_PERIOD = 2^8*TIMER_0_PRESCALER
// TIMER_0_PRESCALER = TIMER_0_PERIOD/(2^8)
//If we use a 16 bit timer for more precision we change this to 
// TIMER_0_PRESCALER = TIMER_0_PERIOD/(2^16), but then we need some other things
//that check for the target value. Having it work by having the timer overflow 
//and reset is better.
//2000 = 2^8*8, so we use an 8 bit timer with this prescaler to get a period of about 2000 instruction cycles
#define TIMER_0_PRESCALER 1

#define LEFT_ADDRESS 0b0100011
#define RIGHT_ADDRESS 0b0100111
#define ENABLE_ADDRESS 0b0101011

//State variables for the left and right fake pwm outputs
unsigned short short leftState = 0;
unsigned short short rightState = 0;

//Enable booleans for left and right PWM outputs
unsigned short short PWMEnable = 1;

//These are the duty cycle values for the left and right side
unsigned int rightDuty = 200;
unsigned int leftDuty = 128;

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
 * We will use 0x23 for the moment. the highest 7 bits are the ones used.
 * 
 * 
 */

//This holds the most recent information in the i2c buffer
unsigned short z = 0;
unsigned short count = 0;
unsigned short state = 0;

void InitI2C(void) {
    //Make SDA and SCL pins inputs
    TRISBbits.RB4 = 1;
    TRISBbits.RB6 = 1;
    
    //Enable internal pull-ups
    INTCON2bits.NOT_RABPU = 0;
    //Turn on pull-ups for RB4 and RB6
    WPUBbits.WPUB4 = 1;
    WPUBbits.WPUB6 = 1;
   
    SSPSTATbits.SMP = 1;
    
    //Set the mode to i2c slave with interrupts enabled for start and stop bits
    SSPCON1bits.SSPM = 0b0110;
    //Enable clock stretching
    SSPCON2bits.SEN = 1;
    
    //Set the bitmask for the address (currently don't ignore any bits)
    //This is a placeholder for later
    //SSPMSKbits.MSK = 0x00;
    SSPMSKbits.MSK = 0b01000110;
    
    //This is the i2c address of the controller. It should be between 0x00 and 0x7F
    //0b01000110
    SSPADD = 0x23<<1;
    
    //Clear the interrupt flag
    PIR1bits.SSPIF = 0;
    //Enable i2c interrupts, may be unneeded
    PIE1bits.SSPIE = 1;
    
    //Globally enable interrupts
    INTCONbits.GIE = 1;
    //Enable prepherial interrupts
    INTCONbits.PEIE = 1;
    
    //Enable the module
    SSPCON1bits.SSPEN = 1;   
}

void interrupt I2C_Slave_Read(void)
{
    //LATCbits.LATC0 ^= 1;
  if(PIR1bits.SSPIF == 1)
  {
        z = SSPBUF;
        
        //hold the clock
        SSPCON1bits.CKP = 0;
        
    //Handle errors by throwing everything away
    if ((SSPCON1bits.SSPOV) || (SSPCON1bits.WCOL))
    {
        SSPCON1bits.SSPOV = 0; // Clear the overflow flag
        SSPCON1bits.WCOL = 0;  // Clear the collision bit
        //release the clock
        SSPCON1bits.CKP = 1;
    } else if(!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {
        //Save the address so we know what to do with the data that follows
        state = z>>1;
        //release the clock
        SSPCON1bits.CKP = 1;
    } else if (SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {
        if (state == LEFT_ADDRESS) {
            leftDuty = z;
        } else if (state == RIGHT_ADDRESS) {
            rightDuty = z;
        } else if (state == ENABLE_ADDRESS) {
            PWMEnable = z;
        }
        SSPCON1bits.CKP = 1;
    } else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW) {
        if (z>>1 == LEFT_ADDRESS){
            //LATCbits.LATC0 ^= 1;
            SSPBUF = leftDuty;
        } else if (z>>1 == RIGHT_ADDRESS){
            //LATCbits.LATC1 ^= 1;
            SSPBUF = rightDuty;
        } else if (z>>1 == ENABLE_ADDRESS){
            //LATCbits.LATC2 ^= 1;
            SSPBUF = PWMEnable;
        } else if (z>>1 == 0b0101111){
            //LATCbits.LATC3 ^= 1;
            SSPBUF = TMR0;
        }
        //release the clock
        SSPCON1bits.CKP = 1;
        //wait until the buffer is cleared
        while(SSPSTATbits.BF);
    }
    //Clear interrupt
    PIR1bits.SSPIF = 0;
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
        //Check if the right state should be changed
        if (TMR0 < leftDuty && leftState == 0) {
        //if (TMR0 < ((TIMER_0_PERIOD*leftDuty)/256) && leftState == 0) {
            //Set the state as high
            leftState = 1;
            //Set the pin as high
            LATAbits.LATA4 = 1;
            
            LATCbits.LATC0 = 1;
        } else if (TMR0 > leftDuty && leftState == 1) {
        //} else if (TMR0 > ((TIMER_0_PERIOD*leftDuty)/256) && leftState == 1) {
            //Set the state as low
            leftState = 0;
            //Set the pin as low
            LATAbits.LATA4 = 0;
            
            LATCbits.LATC0 = 0;
        }
        
        //Check if the left state should be changed
        if (TMR0 < rightDuty && rightState == 0) {
            rightState = 1;
            //Set the pin as high
            LATAbits.LATA5 = 1;
            
            LATCbits.LATC1 = 1;
        } else if (TMR0 > rightDuty && rightState == 1) {
            rightState = 0;
            //Set the pin as low
            LATAbits.LATA5 = 0;
            
            LATCbits.LATC1 = 0;
        }
        
        if (TMR0 > 256*8) {
            TMR0 = 0x00;
        }
    } else if (rightState || leftState) {
        //If it isn't enabled check the left and right state, turn off the PWM
        //on both if either is on.
        LATAbits.LATA4 = 0;
        LATAbits.LATA5 = 0;
        rightState = 0;
        leftState = 0;
    }
}

/*
 * This sets up Timer0 to be used by the pwm modules.
 */
void InitPWM(void) {
    //Use a 1:2 prescaler value
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
}

void main(void) {
    OSCCONbits.IRCF = 0b111;
    
    TRISBbits.RB7 = 0;
    TRISCbits.RC0 = 0;
    TRISCbits.RC1 = 0;
    TRISCbits.RC2 = 0;
    TRISCbits.RC3 = 0;
    LATC = 0x00;
    InitI2C();
    InitPWM();
    while(1) {
        CheckPWMOutput();
    }
    return;
}
