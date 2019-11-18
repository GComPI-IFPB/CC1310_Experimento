/*
 * Copyright (c) 2019, Texas Instruments Incorporated
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

/***** Includes *****/
#include "includes.h"

/***** Defines *****/

#define TASK_STACK_SIZE 1024

#define CREATE_TASK_PRIORITY 2

#define OWN_TX_TASK_PRIORITY 2

#define RECEIVED_TX_TASK_PRIORITY 2

#define RX_TASK_PRIORITY 2

#define PACKET_INTERVAL     (uint32_t)(4000000*1.0f)

/* Set Receive timeout to 500ms */
#define RX_TIMEOUT          (uint32_t)(4000000*0.5f)

#define PAYLOAD_LENGTH          sizeof(tPacket)

//#define PACKET_INTERVAL_US      1000000

/* Number of times the CS command should run when the channel is BUSY */
#define CS_RETRIES_WHEN_BUSY    10

/* The channel is reported BUSY is the RSSI is above this threshold */
#define RSSI_THRESHOLD_DBM      -80

#define IDLE_TIME_US            5000

/* Proprietary Radio Operation Status Codes Number: Operation ended normally */
#define PROP_DONE_OK            0x3400

/* Packet RX Configuration */
#define DATA_ENTRY_HEADER_SIZE 8  /* Constant header size of a Generic Data Entry */
#define MAX_LENGTH             sizeof(tPacket) /* Max length byte the radio will accept */
#define NUM_DATA_ENTRIES       2  /* NOTE: Only two data entries supported at the moment */
#define NUM_APPENDED_BYTES     2  /* The Data Entries data field will contain:
                                   * 1 Header byte (RF_cmdPropRx.rxConf.bIncludeHdr = 0x1)
                                   * Max 30 payload bytes
                                   * 1 status byte (RF_cmdPropRx.rxConf.bAppendStatus = 0x1) */



/* Buffer which contains all Data Entries for receiving data.
 * Pragmas are needed to make sure this buffer is 4 byte aligned (requirement from the RF Core) */
#if defined(__TI_COMPILER_VERSION__)
    #pragma DATA_ALIGN (rxDataEntryBuffer, 4);
        static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)];
#elif defined(__IAR_SYSTEMS_ICC__)
    #pragma data_alignment = 4
        static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)];
#elif defined(__GNUC__)
        static uint8_t rxDataEntryBuffer [RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
            MAX_LENGTH, NUM_APPENDED_BYTES)] __attribute__ ((aligned (4)));
#else
    #error This compiler is not supported.
#endif

/* ALL VARIABLES, OBJECTS AND FUNCTIONS */

/***** Prototypes *****/
void RF_init();
void txOwnTask_init();
void txRxTask_init();
void rxTask_init();
void createPacketTask_init();


static void rxcallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
static void txcallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);


static void createPacketFunction(UArg arg0, UArg arg1);
static void txOwnTaskFunction(UArg arg0, UArg arg1);
static void txRxTaskFunction(UArg arg0, UArg arg1);
static void rxTaskFunction(UArg arg0, UArg arg1);


/* RF object definitions */
static RF_Object rfObject;
static RF_Handle rfHandle;
static RF_Params rfParams;

/* semaphore object definitions */

//// Init mutex
//static Semaphore_Struct semInitStruct;
//static Semaphore_Handle semInitHandle;
// Mutex
static Semaphore_Struct semHandleStruct;
static Semaphore_Handle semHandle;
// Own Queue access
static Semaphore_Struct semOwnEmptyStruct, semOwnFullStruct;
static Semaphore_Handle semOwnEmptyHandle, semOwnFullHandle;
// RX Queue access
static Semaphore_Struct semTxRxEmptyStruct, semTxRxFullStruct;
static Semaphore_Handle semTxRxEmptyHandle, semTxRxFullHandle;
// Valid Packet
static Semaphore_Struct semValidFalseStruct, semValidTrueStruct;
static Semaphore_Handle semValidFalseHandle, semValidTrueHandle;

/* Pin driver handle */
static PIN_Handle ledPinHandle;
static PIN_State ledPinState;

/* Receive dataQueue for RF Core to fill in data */
static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t packetLength;
static uint8_t* packetDataPointer;

/* Task objects and variables */
static Task_Params txOwnTaskParams, rxTaskParams;
static Task_Params txRxTaskParams, createPacketParams;
Task_Struct txOwnTask, rxTask;
Task_Struct txRxTask, createPacketTask;    /* not static so you can see in ROV */
static uint8_t txOwnTaskStack[TASK_STACK_SIZE], rxTaskStack[TASK_STACK_SIZE];
static uint8_t txRxTaskStack[TASK_STACK_SIZE], createPacketStack[TASK_STACK_SIZE];

/* Packets and buffer */
static tPacket packetTX, packetRX;
static tPacket sharedOwnPacket, sharedRxPacket;
static uint8_t packetBuffer[MAX_LENGTH];

static uint16_t seqNumber;
static uint16_t IDGen;
static uint8_t sensorID;
//static uint32_t _time;
//static tQueue receivedPacketQueue, ownPacketQueue;

/* RSSI util */
static rfc_propRxOutput_t rxStatistics;


/* Application LED pin configuration table: */
PIN_Config ledPinTable[] =
{
    Board_PIN_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_PIN_LED2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
#if defined __CC1352R1_LAUNCHXL_BOARD_H__
    Board_DIO30_RFSW | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
#endif
    PIN_TERMINATE
};

//short int ownFlag;
//short int rxFlag;

/*
 *  ======== main ========
 */
int main(void) {

//    ownFlag = 0;
//    rxFlag = 0;

    sensorID = 3;

    Board_init();

    initPacket(&packetTX);
    initPacket(&packetRX);

    /* Construct Semaphores objects to be use as a resource lock*/
    /* MUTEX */
    Semaphore_Params semHandleParams;
    Semaphore_Params_init(&semHandleParams);
    Semaphore_construct(&semHandleStruct, 1, &semHandleParams);
    semHandle = Semaphore_handle(&semHandleStruct);
    /*************************************************************/

    /* ownQueue && ownTx */
    Semaphore_Params semOwnEmptyParams;
    Semaphore_Params_init(&semOwnEmptyParams);
    Semaphore_construct(&semOwnEmptyStruct, 1, &semOwnEmptyParams);
    semOwnEmptyHandle = Semaphore_handle(&semOwnEmptyStruct);

    Semaphore_Params semOwnFullParams;
    Semaphore_Params_init(&semOwnFullParams);
    Semaphore_construct(&semOwnFullStruct, 0, &semOwnFullParams);
    semOwnFullHandle = Semaphore_handle(&semOwnFullStruct);

    /*************************************************************/

    /* Received Queue && receivedTX */
    Semaphore_Params semTxRxEmptyParams;
    Semaphore_Params_init(&semTxRxEmptyParams);
    Semaphore_construct(&semTxRxEmptyStruct, 1, &semTxRxEmptyParams);
    semTxRxEmptyHandle = Semaphore_handle(&semTxRxEmptyStruct);

    Semaphore_Params semTxRxFullParams;
    Semaphore_Params_init(&semTxRxFullParams);
    Semaphore_construct(&semTxRxFullStruct, 0, &semTxRxFullParams);
    semTxRxFullHandle = Semaphore_handle(&semTxRxFullStruct);
    /*************************************************************/

    /* Packet on buffer */
    Semaphore_Params semValidFalseParams;
    Semaphore_Params_init(&semValidFalseParams);
    Semaphore_construct(&semValidFalseStruct, 1, &semValidFalseParams);
    semValidFalseHandle = Semaphore_handle(&semValidFalseStruct);

    Semaphore_Params semValidTrueParams;
    Semaphore_Params_init(&semValidTrueParams);
    Semaphore_construct(&semValidTrueStruct, 0, &semValidTrueParams);
    semValidTrueHandle = Semaphore_handle(&semValidTrueStruct);
    /*************************************************************/

    RF_init();

    createPacketTask_init();
    txOwnTask_init();
//    txRxTask_init();
//    rxTask_init();

    BIOS_start();


    return 0;
}

void RF_init() {

    RF_Params_init(&rfParams);

    ledPinHandle = PIN_open(&ledPinState, ledPinTable);
    if (ledPinHandle == NULL)
    {
        while(1);
    }

    if(RFQueue_defineQueue(&dataQueue,
                            rxDataEntryBuffer,
                            sizeof(rxDataEntryBuffer),
                            NUM_DATA_ENTRIES,
                            MAX_LENGTH + NUM_APPENDED_BYTES))
    {
        /* Failed to allocate space for all data entries */
        while(1);
    }

    /* Customize the CMD_PROP_TX command for this application */
    RF_cmdPropTx.pktLen = PAYLOAD_LENGTH;
    RF_cmdPropTx.pPkt = packetBuffer;
//    RF_cmdPropTx.startTime = 0;
    RF_cmdPropTx.startTrigger.triggerType = TRIG_NOW;
//    RF_cmdPropTx.startTrigger.pastTrig = 1;

//    RF_cmdNop.startTrigger.triggerType = TRIG_ABSTIME;
//    RF_cmdNop.startTrigger.pastTrig = 1;

//    /* Listen Before Talk */
//    /* Set up the next pointers for the command chain */
//    RF_cmdNop.pNextOp = (rfc_radioOp_t*)&RF_cmdPropCs;
//    RF_cmdPropCs.pNextOp = (rfc_radioOp_t*)&RF_cmdCountBranch;
//    RF_cmdCountBranch.pNextOp = (rfc_radioOp_t*)&RF_cmdPropTx;
//    RF_cmdCountBranch.pNextOpIfOk = (rfc_radioOp_t*)&RF_cmdPropCs;

    /* Modify CMD_PROP_RX command for application needs */
    RF_cmdPropRx.pQueue = &dataQueue;           /* Set the Data Entity queue for received data */
    RF_cmdPropRx.rxConf.bAutoFlushIgnored = 1;  /* Discard ignored packets from Rx queue */
    RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 1;   /* Discard packets with CRC error from Rx queue */
    RF_cmdPropRx.maxPktLen = MAX_LENGTH;        /* Implement packet length filtering to avoid PROP_ERROR_RXBUF */
    RF_cmdPropRx.pktConf.bRepeatOk = 1;
    RF_cmdPropRx.pktConf.bRepeatNok = 1;
    RF_cmdPropRx.endTime = RX_TIMEOUT; //RF_convertMsToRatTicks(500); /* Set RX timeout to 500 ms*/
    RF_cmdPropRx.endTrigger.triggerType = TRIG_REL_START; /* End is relative to start */
    RF_cmdPropRx.endTrigger.pastTrig = 1; /* End past trigger is allowed */
    RF_cmdPropRx.pOutput = (uint8_t*)&rxStatistics;


//    /* Customize the API commands with application specific defines */
//    RF_cmdPropCs.rssiThr = RSSI_THRESHOLD_DBM;
//    RF_cmdPropCs.csEndTime = (IDLE_TIME_US + 150) * 4; /* Add some margin */
//    RF_cmdCountBranch.counter = CS_RETRIES_WHEN_BUSY;

//    RF_cmdPropRx.pOutput = (uint8_t*)&rxStatistics;



    /* Request access to the radio */
#if defined(DeviceFamily_CC26X0R2)
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioSetup, &rfParams);
#else
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
#endif// DeviceFamily_CC26X0R2

    /* Set the frequency */
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);

//    _time = RF_getCurrentTime();
}

void createPacketTask_init() {

    Task_Params_init(&createPacketParams);
    createPacketParams.stackSize = TASK_STACK_SIZE;
    createPacketParams.priority = CREATE_TASK_PRIORITY;
    createPacketParams.stack = &createPacketStack;
    createPacketParams.arg0 = (UInt)1000000;

    Task_construct(&createPacketTask, createPacketFunction, &createPacketParams, NULL);
}

void txOwnTask_init() {

    Task_Params_init(&txOwnTaskParams);
    txOwnTaskParams.stackSize = TASK_STACK_SIZE;
    txOwnTaskParams.priority = OWN_TX_TASK_PRIORITY;
    txOwnTaskParams.stack = &txOwnTaskStack;
    txOwnTaskParams.arg0 = (UInt)1000000;

    Task_construct(&txOwnTask, txOwnTaskFunction, &txOwnTaskParams, NULL);
}

void txRxTask_init() {

    Task_Params_init(&txRxTaskParams);
    txRxTaskParams.stackSize = TASK_STACK_SIZE;
    txRxTaskParams.priority = RECEIVED_TX_TASK_PRIORITY;
    txRxTaskParams.stack = &txRxTaskStack;
    txRxTaskParams.arg0 = (UInt)1000000;

    Task_construct(&txRxTask, txRxTaskFunction, &txRxTaskParams, NULL);
}

void rxTask_init() {

    Task_Params_init(&rxTaskParams);
    rxTaskParams.stackSize = TASK_STACK_SIZE;
    rxTaskParams.priority = RX_TASK_PRIORITY;
    rxTaskParams.stack = &rxTaskStack;
    rxTaskParams.arg0 = (UInt)1000000;

    Task_construct(&rxTask, rxTaskFunction, &rxTaskParams, NULL);
}


void createPacketFunction(UArg arg0, UArg arg1) {
    IDGen = 0;
//    printf("TASK: Create Packet\n");

    AONBatMonEnable();
    while (1) {
//        printf("TASK: Create Packet LOOP\n");
        /* Run forever */

        /* Create packet with incrementing sequence number & random payload */
        packetTX.packetSrcID = sensorID;
        packetTX.packetID[0] = (uint8_t) (IDGen >> 8);
        packetTX.packetID[1] = (uint8_t) (IDGen++);
        packetTX.temp = (uint8_t) AONBatMonTemperatureGetDegC();
        AONBatMonDisable();
//        packetTX.temp[0] = (uint8_t) (tmprt >> 32);
//        packetTX.temp[1] = (uint8_t) (tmprt >> 24);
//        packetTX.temp[2] = (uint8_t) (tmprt >> 16);
//        packetTX.temp[3] = (uint8_t) (tmprt >> 8);
//        packetTX.temp[4] = (uint8_t) (tmprt);
        packetTX.data[0] = (uint8_t)(seqNumber >> 8);
        packetTX.data[1] = (uint8_t)(seqNumber++);
        packetTX.jumps[packetTX.jump_count].src = sensorID;


        Semaphore_pend(semOwnEmptyHandle, BIOS_WAIT_FOREVER);
//        printf("TASK: Create Packet PEND semOwnEmptyHandle\n");
        Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);
//        printf("TASK: Create Packet PEND semHandle\n");

        sharedOwnPacket = packetTX;

        Semaphore_post(semHandle);
//        printf("TASK: Create Packet POST semHandle\n");
        Semaphore_post(semOwnFullHandle);
//        printf("TASK: Create Packet POST semOwnFullHandle\n");


        AONBatMonEnable();
        sleep(1);

    }
}

void txOwnTaskFunction(UArg arg0, UArg arg1) {
    tPacket txOwnPacket;
//    printf("TASK: TX OWN\n");
    while (1) {
//            printf("TASK: TX OWN LOOP\n");

        Semaphore_pend(semOwnFullHandle, BIOS_WAIT_FOREVER);
//            printf("TASK: TX OWN PEND semOwnFullHandle\n");
        Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);
//            printf("TASK: TX OWN PEND semHandle\n");

        txOwnPacket = sharedOwnPacket;

        Semaphore_post(semHandle);
//            printf("TASK: TX OWN POST semHandle\n");
        Semaphore_post(semOwnEmptyHandle);
//            printf("TASK: TX OWN POST semOwnEmptyHandle\n");

        Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);

        memcpy(packetBuffer, &txOwnPacket, sizeof(tPacket));

        RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal,
        &txcallback, 0);

        Semaphore_post(semHandle);
//        Task_yield();
//        Task_sleep(1000000);
//        sleep(1);
    }
}


/*
 *  ======== txTaskFunction ========
 */

void txRxTaskFunction(UArg arg0, UArg arg1) {
    tPacket packetTxRx;
//    printf("TASK: TX RX\n");
    while (1) {
//        printf("TASK: TX RX LOOP\n");

        Semaphore_pend(semValidTrueHandle, BIOS_WAIT_FOREVER);
//        printf("TASK: TX RX PEND semValidTrueHandle\n");
        Semaphore_pend(semTxRxFullHandle, BIOS_WAIT_FOREVER);
//        printf("TASK: TX RX PEND semTxRxFullHandle\n");
//        Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);
//        printf("TASK: TX RX PEND semHandle\n");

        packetTxRx = sharedRxPacket;

//        Semaphore_post(semHandle);
//        printf("TASK: TX RX POST semHandle\n");
        Semaphore_post(semTxRxEmptyHandle);
//        printf("TASK: TX RX POST semTxRxEmptyHandle\n");
        Semaphore_post(semValidFalseHandle);
//        printf("TASK: TX RX POST semValidFalseHandle\n");

        Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);

        packetTxRx.jumps[packetTxRx.jump_count].src = sensorID;

        memcpy(packetBuffer, &packetTxRx, sizeof(tPacket));

        RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal,
        &txcallback, 0);

        Semaphore_post(semHandle);



//        Task_yield();
//        Task_sleep(1000);
//        sleep(1);
    }
}

/*
 *  ======== TX callback ========
 */

static void txcallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e) {

    if ((e & RF_EventLastCmdDone) &&
        (RF_cmdPropTx.status == PROP_DONE_OK)) {
        PIN_setOutputValue(ledPinHandle, Board_PIN_LED2,
                                       !PIN_getOutputValue(Board_PIN_LED2));
//        printf("TASK: TX RX SEND\n");
    }

}

static void rxTaskFunction(UArg arg0, UArg arg1)
{
    /* Enter RX mode and stay forever in RX */
//    printf("TASK: RX\n");
    while (1) {
//        printf("TASK: RX LOOP\n");


        Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);
//        printf("TASK: RX PEND semHandle\n");


        RF_EventMask terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropRx,
                                                  RF_PriorityNormal, &rxcallback,
                                                  RF_EventRxEntryDone);

        Semaphore_post(semHandle);
//        printf("TASK: RX POST semHandle\n");

//        printf("TASK: RX POST semTxRxFullHandle\n");

//        Task_yield();
        sleep(1);
    }
}

/*
 *  ======== RX callback ========
 */

static void rxcallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
//    printf("RX CALLBACK\n");
    if (e & RF_EventRxEntryDone)
    {
//        printf("RX CALLBACK TRUE\n");
        /* Toggle pin to indicate RX */

        /* Get current unhandled data entry */
        currentDataEntry = RFQueue_getDataEntry();

        /* Handle the packet data, located at &currentDataEntry->data:
         * - Length is the first byte with the current configuration
         * - Data starts from the second byte */
        packetLength      = *(uint8_t*)(&currentDataEntry->data);
        packetDataPointer =  (uint8_t*)(&currentDataEntry->data + 1);

        /* Copy the payload + the status byte to the packet variable */
        memcpy(&packetRX, packetDataPointer, (packetLength + 1));
//        printf("RX CALLBACK: Copy packet\n");

        if (packetRX.packetSrcID > sensorID) {

//            printf("RX CALLBACK: Valid packet\n");
            packetRX.jumps[packetRX.jump_count].to = sensorID;
            packetRX.jumps[packetRX.jump_count].rssi = rxStatistics.lastRssi;
            packetRX.jump_count++;

            Semaphore_pend(semTxRxEmptyHandle, BIOS_WAIT_FOREVER);
//            printf("RX CALLBACK PEND semTxRxEmptyHandle\n");
            Semaphore_pend(semValidFalseHandle, BIOS_WAIT_FOREVER);
//            printf("RX CALLBACK PEND semValidFalseHandle\n");

            sharedRxPacket = packetRX;

            Semaphore_post(semValidTrueHandle);
//            printf("RX CALLBACK POST semValidTrueHandle\n");
            Semaphore_post(semTxRxFullHandle);
//            printf("RX CALLBACK POST semTxRxFullHandle\n");

            PIN_setOutputValue(ledPinHandle, Board_PIN_LED1, !PIN_getOutputValue(Board_PIN_LED1));

        }

        RFQueue_nextEntry();
    }
}

