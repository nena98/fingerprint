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
#include <ti/sysbios/knl/Mailbox.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

/* Board Header file */
#include "Board.h"

#define TASKSTACKSIZE   512

UART_Handle uart0;
UART_Handle uart5;

#define NUMMSGS         1

typedef struct MsgObj {
    Char    cmd;
} MsgObj;

typedef struct MailboxMsgObj {
    Mailbox_MbxElem  elem;      /* Mailbox header        */
    MsgObj           data;       /* Application's mailbox */
} MailboxMsgObj;

MailboxMsgObj mailboxBuffer[NUMMSGS];

Mailbox_Struct mbxStruct;
Mailbox_Handle mbxHandle;

Task_Struct taskUserInputStruct;
Char taskUserInputStack[TASKSTACKSIZE];

Task_Struct taskProcessingInputStruct;
Char taskProcessingInputStack[TASKSTACKSIZE];

Void userInputTask(UArg arg0, UArg arg1)
{
    MsgObj msg;

    while(1)
    {
        UART_read(uart0, &msg.cmd, 1);

        Mailbox_post(mbxHandle, &msg,  BIOS_WAIT_FOREVER);
    }
}

Void processingInputTask(UArg arg0, UArg arg1)
{

    while(1)
    {

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

    /* Create mailbox*/
    Mailbox_Params mbxParams;

    Mailbox_Params_init(&mbxParams);
    mbxParams.buf = (Ptr)mailboxBuffer;
    mbxParams.bufSize = sizeof(mailboxBuffer);
    Mailbox_construct(&mbxStruct, sizeof(MsgObj), NUMMSGS, &mbxParams, NULL);
    mbxHandle = Mailbox_handle(&mbxStruct);

    /* Create tasks*/
    Task_Params taskUserInputParams;

    Task_Params_init(&taskUserInputParams);
    taskUserInputParams.stackSize = TASKSTACKSIZE;
    taskUserInputParams.stack = &taskUserInputStack;
    taskUserInputParams.instance->name = "user input";
    Task_construct(&taskUserInputStruct, (Task_FuncPtr)userInputTask, &taskUserInputParams, NULL);

    Task_Params taskProcessingInputParams;

    Task_Params_init(&taskProcessingInputParams);
    taskProcessingInputParams.stackSize = TASKSTACKSIZE;
    taskProcessingInputParams.stack = &taskProcessingInputStack;
    taskProcessingInputParams.instance->name = "processing input";
    Task_construct(&taskProcessingInputStruct, (Task_FuncPtr)processingInputTask, &taskProcessingInputParams, NULL);

    /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* Start BIOS */
    BIOS_start();

    return (0);
}
