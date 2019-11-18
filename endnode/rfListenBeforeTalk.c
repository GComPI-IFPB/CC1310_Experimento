///*
// * Copyright (c) 2019, Texas Instruments Incorporated
// * All rights reserved.
// *
// * Redistribution and use in source and binary forms, with or without
// * modification, are permitted provided that the following conditions
// * are met:
// *
// * *  Redistributions of source code must retain the above copyright
// *    notice, this list of conditions and the following disclaimer.
// *
// * *  Redistributions in binary form must reproduce the above copyright
// *    notice, this list of conditions and the following disclaimer in the
// *    documentation and/or other materials provided with the distribution.
// *
// * *  Neither the name of Texas Instruments Incorporated nor the names of
// *    its contributors may be used to endorse or promote products derived
// *    from this software without specific prior written permission.
// *
// * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// */
//
///***** Includes *****/
///* Util headers */
//#include "packet.h"
//#include "queue.h"
//
///* Standard C Libraries */
//#include <stdlib.h>
//#include <stdint.h>
////#include <time.h>
//
///* RTOS Header files */
//#include <ti/sysbios/BIOS.h>
//#include <ti/sysbios/knl/Task.h>
//#include <ti/sysbios/knl/Semaphore.h>
//
//
///* Board Header files */
//#include "Board.h"
////
/////* Application Header files */
////#include "smartrf_settings/smartrf_settings.h"
//
/////* Pin driver handle */
////static PIN_Handle ledPinHandle;
////static PIN_State ledPinState;
////
/////* Application LED pin configuration table: */
////PIN_Config ledPinTable[] =
////{
////    Board_PIN_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
////    PIN_TERMINATE
////};
//
///* semaphore object definitions */
//static Semaphore_Struct semStruct;
//static Semaphore_Handle semHandle;
//
///***** Defines *****/
//#define TASK_STACK_SIZE 1024
//#define TX_TASK_PRIORITY   2
//#define RX_TASK_PRIORITY   2
//
//
///***** Prototypes *****/
//void txTask_init();
//void rxTask_init();
//extern void txTaskFunction(UArg arg0, UArg arg1);
////extern void rxTaskFunction(UArg arg0, UArg arg1);
////static void txcallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
//
///***** Variable declarations *****/
//static Task_Params txTaskParams, rxTaskParams;
//Task_Struct txTask, rxTask;    /* not static so you can see in ROV */
//static uint8_t txTaskStack[TASK_STACK_SIZE], rxTaskStack[TASK_STACK_SIZE];
//
//
///*
// *  ======== main ========
// */
//
//int main(void) {
//
//    Semaphore_Params semParams;
//    /* Construct a Semaphore object to be use as a resource lock, inital count 1 */
//    Semaphore_Params_init(&semParams);
//    Semaphore_construct(&semStruct, 0, &semParams);
//    /* Obtain instance handle */
//    semHandle = Semaphore_handle(&semStruct);
//
//    Board_initGeneral();
//    txTask_init();
//    rxTask_init();
//    BIOS_start();
//
//    return (0);
//}
//
//void txTask_init() {
//
//    Task_Params_init(&txTaskParams);
//    txTaskParams.stackSize = TASK_STACK_SIZE;
//    txTaskParams.priority = TX_TASK_PRIORITY;
//    txTaskParams.stack = &txTaskStack;
//    txTaskParams.arg0 = (UInt)1000000;
//
//    Task_construct(&txTask, txTaskFunction, &txTaskParams, NULL);
//}
//
//void rxTask_init() {
//
//    Task_Params_init(&rxTaskParams);
//    txTaskParams.stackSize = TASK_STACK_SIZE;
//    txTaskParams.priority = RX_TASK_PRIORITY;
//    txTaskParams.stack = &rxTaskStack;
//    txTaskParams.arg0 = (UInt)1000000;
//
//    Task_construct(&rxTask, rxTaskFunction, &rxTaskParams, NULL);
//}
//
//
//
