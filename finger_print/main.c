//*****************************************************************************
//
// uart_echo.c - Example demonstrating UART module in internal loopback mode.
//
// Copyright (c) 2015-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions
//   are met:
// 
//   Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the  
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This is part of revision 2.1.4.178 of the Tiva Firmware Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "inc/hw_types.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

//*****************************************************************************
//
//! \addtogroup uart_examples_list
//! <h1>UART Loopback (uart_loopback)</h1>
//!
//! This example demonstrates the use of a UART port in loopback mode.  On
//! being enabled in loopback mode, the transmit line of the UART is internally
//! connected to its own receive line.  Hence, the UART port receives back the
//! entire data it transmitted.
//!
//! This example echoes data sent to the UART's transmit FIFO back to the same
//! UART's receive FIFO.  To achieve this, the UART is configured in loopback
//! mode.  In the loopback mode, the Tx line of the UART is directly connected
//! to its Rx line internally and all the data placed in the transmit buffer is
//! internally transmitted to the Receive buffer.
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board.
//! - UART5 peripheral - For internal Loopback
//! - UART0 peripheral - As console to display debug messages.
//!     - UART0RX - PA0
//!     - UART0TX - PA1
//!
//! UART parameters for the UART0 and UART7 port:
//! - Baud rate - 115,200
//! - 8-N-1 operation
//
//*****************************************************************************

//*****************************************************************************
//
// Macros used in this application.
//
//*****************************************************************************

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

void
UART5IntHandler(void)
{
    uint32_t ui32Status;

    //
    // Get the interrupt status.
    //
    ui32Status = UARTIntStatus(UART5_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    ROM_UARTIntClear(UART5_BASE, ui32Status);

    if(((ui32Status & UART_INT_RX) == UART_INT_RX) || ((ui32Status & UART_INT_RT) == UART_INT_RT))
    {
        //
        // Loop while there are characters in the receive FIFO.
        //
        while(UARTCharsAvail(UART5_BASE))
        {
            //
            // Read the next character from the UART5 and write it back to the UART0
            //
            ROM_UARTCharPutNonBlocking(UART0_BASE,
                                       ROM_UARTCharGetNonBlocking(UART5_BASE));
        }

    }

}

//*****************************************************************************
//
// Send a string to the UART.  This function sends a string of characters to a
// particular UART module.
//
//*****************************************************************************
void
UARTSend(uint32_t ui32UARTBase, const uint8_t *pui8Buffer, uint32_t ui32Count)
{
    //
    // Loop while there are more characters to send.
    //
    while(ui32Count--)
    {
        //
        // Write the next character to the UART.
        //
        MAP_UARTCharPut(ui32UARTBase, *pui8Buffer++);
    }
}

//*****************************************************************************
//
// Configue UART in internal loopback mode and tranmsit and receive data
// internally.
//
//*****************************************************************************
int
main(void)
{

#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
    defined(TARGET_IS_TM4C129_RA1) ||                                         \
    defined(TARGET_IS_TM4C129_RA2)
    uint32_t ui32SysClock;
#endif


    //
    // Set the clocking to run directly from the crystal.
    //
#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
    defined(TARGET_IS_TM4C129_RA1) ||                                         \
    defined(TARGET_IS_TM4C129_RA2)
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_OSC), 25000000);
#else
    MAP_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);
#endif

    //
    // Enable the peripherals used by this example.
    // UART0 :  To dump information to the console about the example.
    // UART5 :  Enabled in loopback mode. Anything transmitted to Tx will be
    //          received at the Rx.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    //
    // Set GPIO A0 and A1 as UART pins.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Set GPIO E4 and E5 as UART pins.
    //
    GPIOPinConfigure(GPIO_PE4_U5RX);
    GPIOPinConfigure(GPIO_PE5_U5TX);
    MAP_GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    //
    // Internal loopback programming.  Configure the UART in loopback mode.
    //
    //UARTLoopbackEnable(UART5_BASE);

    //
    // Configure the UART for 115,200, 8-N-1 operation.
    //
#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
    defined(TARGET_IS_TM4C129_RA1) ||                                         \
    defined(TARGET_IS_TM4C129_RA2)
    MAP_UARTConfigSetExpClk(UART0_BASE, ui32SysClock, 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
    MAP_UARTConfigSetExpClk(UART5_BASE, ui32SysClock, 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
#else
    MAP_UARTConfigSetExpClk(UART0_BASE, MAP_SysCtlClockGet(), 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
    MAP_UARTConfigSetExpClk(UART5_BASE, MAP_SysCtlClockGet(), 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
#endif

    //
    // Enable the UART interrupt.
    //
    ROM_IntEnable(INT_UART5);
    ROM_UARTIntEnable(UART5_BASE, UART_INT_RX | UART_INT_RT);

    //write available options
    UARTSend(UART0_BASE, (uint8_t *)"\033[2J\033[H1. Check number of registered fingerprints\r\n",
                         strlen("\033[2J\033[H\1. Check number of registered fingerprints\r\n"));
    UARTSend(UART0_BASE, (uint8_t *)"2. Register fingerprint\r\n",
                             strlen("2. Register fingerprint\r\n"));
    UARTSend(UART0_BASE, (uint8_t *)"3. Compare fingerprint\r\n", strlen("3. Compare fingerprint\r\n"));
    UARTSend(UART0_BASE, (uint8_t *)"4. Query fingerprint information\r\n", strlen("4. Query fingerprint information\r\n"));
    UARTSend(UART0_BASE, (uint8_t *)"5. Scan and upload fingerprint image\r\n", strlen("5. Scan and upload fingerprint image\r\n"));
    UARTSend(UART0_BASE, (uint8_t *)"6. Clear registered fingerprint\r\n", strlen("6. Clear registered fingerprint\r\n"));

    //
    // Wait for the UART module to complete transmitting.
    //
    while(MAP_UARTBusy(UART5_BASE))
    {
    }

    while(1)
    {
        //wait for user to choose option
        uint8_t number = ROM_UARTCharGet(UART0_BASE);

        switch(number)
        {
        case '1':
            UARTSend(UART5_BASE, (uint8_t*)"<C>CheckRegisteredNo</C>", strlen("<C>CheckRegisteredNo</C>"));
            break;
        case '2':
            UARTSend(UART0_BASE, (uint8_t *)"Enter index to register fingerprint\r\n",
                                                 strlen("Enter index to register fingerprint\r\n"));

            //write menu
            UARTSend(UART0_BASE, (uint8_t *)"a. 0-index\r\n", strlen("a. 0-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"b. 1-index\r\n", strlen("b. 1-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"c. 2-index\r\n", strlen("c. 2-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"d. 3-index\r\n", strlen("d. 3-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"e. 4-index\r\n", strlen("e. 4-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"f. 5-index\r\n", strlen("f. 5-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"g. 6-index\r\n", strlen("g. 6-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"h. 7-index\r\n", strlen("h. 7-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"i. 8-index\r\n", strlen("i. 8-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"j. 9-index\r\n", strlen("j. 9-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"k. 10-index\r\n", strlen("k. 10-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"l. 11-index\r\n", strlen("l. 11-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"m. 12-index\r\n", strlen("m. 12-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"n. 13-index\r\n", strlen("n. 13-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"o. 14-index\r\n", strlen("o. 14-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"p. 15-index\r\n", strlen("p. 15-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"q. 16-index\r\n", strlen("q. 16-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"r. 17-index\r\n", strlen("r. 17-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"s. 18-index\r\n", strlen("s. 18-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"t. 19-index\r\n", strlen("t. 19-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"u. 20-index\r\n", strlen("u. 20-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"v. 21-index\r\n", strlen("v. 21-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"w. 22-index\r\n", strlen("w. 22-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"x. 23-index\r\n", strlen("x. 23-index\r\n"));

            //wait for user to enter index
            uint8_t index;

            index = ROM_UARTCharGet(UART0_BASE);


            switch(index)
            {
            case 'a':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=0</C>", strlen("<C>RegisterOneFp=0</C>"));
                break;
            case 'b':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=1</C>", strlen("<C>RegisterOneFp=1</C>"));
                break;
            case 'c':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=2</C>", strlen("<C>RegisterOneFp=2</C>"));
                break;
            case 'd':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=3</C>", strlen("<C>RegisterOneFp=3</C>"));
                break;
            case 'e':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=4</C>", strlen("<C>RegisterOneFp=4</C>"));
                break;
            case 'f':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=5</C>", strlen("<C>RegisterOneFp=5</C>"));
                break;
            case 'g':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=6</C>", strlen("<C>RegisterOneFp=6</C>"));
                break;
            case 'h':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=7</C>", strlen("<C>RegisterOneFp=7</C>"));
                break;
            case 'i':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=8</C>", strlen("<C>RegisterOneFp=8</C>"));
                break;
            case 'j':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=9</C>", strlen("<C>RegisterOneFp=9</C>"));
                break;
            case 'k':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=10</C>", strlen("<C>RegisterOneFp=10</C>"));
                break;
            case 'l':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=11</C>", strlen("<C>RegisterOneFp=11</C>"));
                break;
            case 'm':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=12</C>", strlen("<C>RegisterOneFp=12</C>"));
                break;
            case 'n':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=13</C>", strlen("<C>RegisterOneFp=13</C>"));
                break;
            case 'o':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=14</C>", strlen("<C>RegisterOneFp=14</C>"));
                break;
            case 'p':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=15</C>", strlen("<C>RegisterOneFp=15</C>"));
                break;
            case 'q':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=16</C>", strlen("<C>RegisterOneFp=16</C>"));
                break;
            case 'r':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=17</C>", strlen("<C>RegisterOneFp=17</C>"));
                break;
            case 's':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=18</C>", strlen("<C>RegisterOneFp=18</C>"));
                break;
            case 't':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=19</C>", strlen("<C>RegisterOneFp=19</C>"));
                break;
            case 'u':
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=20</C>", strlen("<C>RegisterOneFp=20</C>"));
                break;
            case ('v'):
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=21</C>", strlen("<C>RegisterOneFp=21</C>"));
                break;
            case ('w'):
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=22</C>", strlen("<C>RegisterOneFp=22</C>"));
                break;
            case ('x'):
                UARTSend(UART5_BASE, (uint8_t*)"<C>RegisterOneFp=23</C>", strlen("<C>RegisterOneFp=23</C>"));
                break;
            default:
                break;
            }
            break;
        case '3':
            UARTSend(UART5_BASE, (uint8_t*)"<C>CompareFingerprint</C>", strlen("<C>CompareFingerprint</C>"));
            break;
        case '4':
            UARTSend(UART5_BASE, (uint8_t*)"<C>FpImageInformation</C>", strlen("<C>FpImageInformation</C>"));
            break;
        case '5':
            UARTSend(UART5_BASE, (uint8_t*)"<C>ScanFpImage</C>", strlen("<C>ScanFpImage</C>"));
            break;
        case '6':
            UARTSend(UART0_BASE, (uint8_t *)"Enter index to clear fingerprint\r\n",
                                                         strlen("Enter index to delete fingerprint\r\n"));

            //write menu
            UARTSend(UART0_BASE, (uint8_t *)"a. 0-index\r\n", strlen("a. 0-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"b. 1-index\r\n", strlen("b. 1-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"c. 2-index\r\n", strlen("c. 2-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"d. 3-index\r\n", strlen("d. 3-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"e. 4-index\r\n", strlen("e. 4-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"f. 5-index\r\n", strlen("f. 5-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"g. 6-index\r\n", strlen("g. 6-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"h. 7-index\r\n", strlen("h. 7-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"i. 8-index\r\n", strlen("i. 8-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"j. 9-index\r\n", strlen("j. 9-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"k. 10-index\r\n", strlen("k. 10-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"l. 11-index\r\n", strlen("l. 11-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"m. 12-index\r\n", strlen("m. 12-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"n. 13-index\r\n", strlen("n. 13-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"o. 14-index\r\n", strlen("o. 14-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"p. 15-index\r\n", strlen("p. 15-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"q. 16-index\r\n", strlen("q. 16-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"r. 17-index\r\n", strlen("r. 17-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"s. 18-index\r\n", strlen("s. 18-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"t. 19-index\r\n", strlen("t. 19-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"u. 20-index\r\n", strlen("u. 20-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"v. 21-index\r\n", strlen("v. 21-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"w. 22-index\r\n", strlen("w. 22-index\r\n"));
            UARTSend(UART0_BASE, (uint8_t *)"x. 23-index\r\n", strlen("x. 23-index\r\n"));

            //wait for user to enter index
            uint8_t del_index;

            del_index = ROM_UARTCharGet(UART0_BASE);


            switch(del_index)
            {
            case 'a':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=0</C>", strlen("<C>ClearOneFp=0</C>"));
                break;
            case 'b':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=1</C>", strlen("<C>ClearOneFp=1</C>"));
                break;
            case 'c':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=2</C>", strlen("<C>ClearOneFp=2</C>"));
                break;
            case 'd':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=3</C>", strlen("<C>ClearOneFp=3</C>"));
                break;
            case 'e':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=4</C>", strlen("<C>ClearOneFp=4</C>"));
                break;
            case 'f':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=5</C>", strlen("<C>ClearOneFp=5</C>"));
                break;
            case 'g':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=6</C>", strlen("<C>ClearOneFp=6</C>"));
                break;
            case 'h':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=7</C>", strlen("<C>ClearOneFp=7</C>"));
                break;
            case 'i':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=8</C>", strlen("<C>ClearOneFp=8</C>"));
                break;
            case 'j':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=9</C>", strlen("<C>ClearOneFp=9</C>"));
                break;
            case 'k':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=10</C>", strlen("<C>ClearOneFp=10</C>"));
                break;
            case 'l':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=11</C>", strlen("<C>ClearOneFp=11</C>"));
                break;
            case 'm':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=12</C>", strlen("<C>ClearOneFp=12</C>"));
                break;
            case 'n':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=13</C>", strlen("<C>ClearOneFp=13</C>"));
                break;
            case 'o':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=14</C>", strlen("<C>ClearOneFp=14</C>"));
                break;
            case 'p':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=15</C>", strlen("<C>ClearOneFp=15</C>"));
                break;
            case 'q':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=16</C>", strlen("<C>ClearOneFp=16</C>"));
                break;
            case 'r':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=17</C>", strlen("<C>ClearOneFp=17</C>"));
                break;
            case 's':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=18</C>", strlen("<C>ClearOneFp=18</C>"));
                break;
            case 't':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=19</C>", strlen("<C>ClearOneFp=19</C>"));
                break;
            case 'u':
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=20</C>", strlen("<C>ClearOneFp=20</C>"));
                break;
            case ('v'):
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=21</C>", strlen("<C>ClearOneFp=21</C>"));
                break;
            case ('w'):
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=22</C>", strlen("<C>ClearOneFp=22</C>"));
                break;
            case ('x'):
                UARTSend(UART5_BASE, (uint8_t*)"<C>ClearOneFp=23</C>", strlen("<C>ClearOneFp=23</C>"));
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    //
    // Return no errors
    //
    return(0);
}
