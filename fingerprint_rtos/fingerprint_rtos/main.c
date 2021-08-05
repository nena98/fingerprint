/*
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

/* Board Header file */
#include "Board.h"

#define TASKSTACKSIZE   512

UART_Handle uart0;
UART_Handle uart5;

Task_Struct taskProcessingInputStruct;
Char taskProcessingInputStack[TASKSTACKSIZE];

Task_Struct taskDeviceRespondStruct;
Char taskDeviceRespondStack[TASKSTACKSIZE];

Task_Struct taskStartMenuStruct;
Char taskStartMenuStack[TASKSTACKSIZE];

Event_Struct evtStruct;
Event_Handle evtHandle;

void checkRegisteredNumber()
{
    char cmd[] = "<C>CheckRegisteredNo</C>";

    UART_write(uart5, &cmd, sizeof(cmd));
}

void startOptions()
{
    char op1[] = "\033[2J\033[H1. Check number of registered fingerprints\r\n";
    char op2[] = "2. Register fingerprint\r\n";
    char op3[] = "3. Compare fingerprint\r\n";
    char op4[] = "4. Query fingerprint information\r\n";
    char op5[] = "5. Scan and upload fingerprint image\r\n";
    char op6[] = "6. Clear registered fingerprint\r\n";
    char op7[] = "*After the previous option is done, press anything to continue!\r\n";

    UART_write(uart0, &op1, sizeof(op1));
    UART_write(uart0, &op2, sizeof(op2));
    UART_write(uart0, &op3, sizeof(op3));
    UART_write(uart0, &op4, sizeof(op4));
    UART_write(uart0, &op5, sizeof(op5));
    UART_write(uart0, &op6, sizeof(op6));
    UART_write(uart0, &op7, sizeof(op7));
}

Void startMenuTask(UArg arg0, UArg arg1)
{

    while(1)
    {
        startOptions();

        /* Wait for (Event_Id_00 & Event_Id_01)*/
        Event_pend(evtHandle,
                   Event_Id_00 + Event_Id_01,  /* andMask */
                   Event_Id_NONE,                /* orMask */
                   BIOS_WAIT_FOREVER);

    }
}

Void processingInputTask(UArg arg0, UArg arg1)
{
    char cmd;
    uint8_t posted = 0;

    while(1)
    {
        UART_read(uart0, &cmd, 1);

        if(posted == 1)
        {
            posted = 0;
            Event_post(evtHandle, Event_Id_00);
            continue;
        }

        switch(cmd)
        {
        case '1':
            checkRegisteredNumber();
            break;
        default:
            break;
        }

        /* Explicit posting of Event_Id_01 by calling Event_post() */
        Event_post(evtHandle, Event_Id_01);
        posted = 1;

    }
}

Void deviceRespondTask(UArg arg0, UArg arg1)
{
    char data;

    while(1)
    {
        UART_read(uart5, &data, 1);
        UART_write(uart0, &data, 1);
    }
}

/*
 *  ======== main ========
 */
int main(void)
{

    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initUART();

    UART_Params uart0Params;

    /* Create a UART0 with data processing off. */
    UART_Params_init(&uart0Params);
    uart0Params.writeDataMode = UART_DATA_BINARY;
    uart0Params.readDataMode = UART_DATA_BINARY;
    uart0Params.readReturnMode = UART_RETURN_FULL;
    uart0Params.readEcho = UART_ECHO_OFF;
    uart0Params.baudRate = 9600;
    uart0 = UART_open(Board_UART0, &uart0Params);

    if (uart0 == NULL) {
        System_abort("Error opening the UART0");
    }

    UART_Params uart5Params;

    /* Create a UART5 with data processing off. */
    UART_Params_init(&uart5Params);
    uart5Params.writeDataMode = UART_DATA_BINARY;
    uart5Params.readDataMode = UART_DATA_BINARY;
    uart5Params.readReturnMode = UART_RETURN_FULL;
    uart5Params.readEcho = UART_ECHO_OFF;
    uart5Params.baudRate = 9600;
    uart5 = UART_open(Board_UART5, &uart5Params);

    if (uart5 == NULL) {
        System_abort("Error opening the UART5");
    }

    /* Create event*/
    Event_construct(&evtStruct, NULL);

    /* Obtain event instance handle */
    evtHandle = Event_handle(&evtStruct);

    if (evtHandle == NULL) {
        System_abort("Event create failed");
    }

    /* Create tasks*/
    Task_Params taskProcessingInputParams;

    Task_Params_init(&taskProcessingInputParams);
    taskProcessingInputParams.stackSize = TASKSTACKSIZE;
    taskProcessingInputParams.stack = &taskProcessingInputStack;
    taskProcessingInputParams.priority = 2;
    taskProcessingInputParams.instance->name = "processing input";
    Task_construct(&taskProcessingInputStruct, (Task_FuncPtr)processingInputTask, &taskProcessingInputParams, NULL);

    Task_Params taskDeviceRespondParams;

    Task_Params_init(&taskDeviceRespondParams);
    taskDeviceRespondParams.stackSize = TASKSTACKSIZE;
    taskDeviceRespondParams.stack = &taskDeviceRespondStack;
    taskDeviceRespondParams.priority = 1;
    taskDeviceRespondParams.instance->name = "device respond";
    Task_construct(&taskDeviceRespondStruct, (Task_FuncPtr)deviceRespondTask, &taskDeviceRespondParams, NULL);

    Task_Params taskStartMenuParams;

    Task_Params_init(&taskStartMenuParams);
    taskStartMenuParams.stackSize = TASKSTACKSIZE;
    taskStartMenuParams.stack = &taskStartMenuStack;
    taskStartMenuParams.priority = 1;
    taskStartMenuParams.instance->name = "start menu";
    Task_construct(&taskStartMenuStruct, (Task_FuncPtr)startMenuTask, &taskStartMenuParams, NULL);

    /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* Start BIOS */
    BIOS_start();

    return (0);
}
