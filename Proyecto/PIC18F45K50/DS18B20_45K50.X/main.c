/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.6
        Device            :  PIC18F45K50
        Driver Version    :  2.00
 */

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip software and any
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party
    license terms applicable to your use of third party software (including open source software) that
    may accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
    FOR A PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
    SOFTWARE.
 */

#include "mcc_generated_files/mcc.h"

/*
                         Main application
 */

#include "DS18x20.h"
#include "lcd.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void main( void )
{
    // Initialize the device
    SYSTEM_Initialize();

    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts
    // Use the following macros to:

    // Enable the Global Interrupts
    // INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts
    // INTERRUPT_GlobalInterruptDisable();

    // Enable the Peripheral Interrupts
    // INTERRUPT_PeripheralInterruptEnable();

    // Disable the Peripheral Interrupts
    // INTERRUPT_PeripheralInterruptDisable();
    __delay_ms( 200 );
    Lcd_Init();
    __delay_ms( 100 );
    while ( 1 ) {

        uint8_t i;
        uint8_t present = 0;
        uint8_t type_s;
        uint8_t data[12];
        uint8_t addr[8];
        float celsius;

        if ( !search( addr ) ) {
            reset_search();
            __delay_ms( 250 );
            return;
        }

        reset();
        select( addr );
        write( 0x44 );      // start conversion, with parasite power on at the end

        __delay_ms( 1000 ); // maybe 750ms is enough, maybe not
        // we might do a ds.depower() here, but the reset will take care of it.

        present = reset();
        select( addr );
        write( 0xBE );              // Read Scratchpad

        char HEX_[10];
        for ( i = 0; i < 9; i++ ) { // we need 9 bytes
            data[i] = read();
            sprintf( HEX_, "0x%x", data[i] );
        }

        // Convert the data to actual temperature
        // because the result is a 16 bit signed integer, it should
        // be stored to an "int16_t" type, which is always 16 bits
        // even when compiled on a 32 bit processor.
        int16_t raw = ( data[1] << 8 ) | data[0];
        if ( type_s ) {
            raw = raw << 3; // 9 bit resolution default
            if ( data[7] == 0x10 ) {
                // "count remain" gives full 12 bit resolution
                raw = ( raw & 0xFFF0 ) + 12 - data[6];
            }
        } else {
            uint8_t cfg = ( data[4] & 0x60 );
            // at lower res, the low bits are undefined, so let's zero them
            if ( cfg == 0x00 )
                raw = raw & ~7; // 9 bit resolution, 93.75 ms
            else if ( cfg == 0x20 )
                raw = raw & ~3; // 10 bit res, 187.5 ms
            else if ( cfg == 0x40 )
                raw = raw & ~1; // 11 bit res, 375 ms
            //// default is 12 bit resolution, 750 ms conversion time
        }
        celsius = (float)raw / 16.0;
        char Cel_[20];
        sprintf( Cel_, "Temp: %0.2fC", celsius );

        Lcd_Clear();
        Lcd_Set_Cursor( 1, 1 );
        Lcd_Write_String( Cel_ );
        __delay_ms( 3000 );
    }
}
/**
 End of File
 */