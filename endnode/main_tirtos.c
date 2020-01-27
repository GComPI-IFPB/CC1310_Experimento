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

#define TX_TASK_PRIORITY 2

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
#define MAX_LENGTH             124 /* Max length byte the radio will accept */
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
void txTask_init();
void rxTask_init();
void createPacketTask_init();
void UARTinit();
void writeTask_init();


void printArray(uint8_t *arr);
void printPacket(tPacket *p);
void copyByteArray(uint8_t *to, uint8_t *from);
void insertPacket(uint8_t *arr, tPacket *p);
void getPacket(uint8_t *arr, tPacket *p);
void copyPacketToArray(uint8_t *arr, tPacket *p);
void newestInsert(uint8_t *arr, tPacket *p);


static void rxcallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
static void txcallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);


static void createPacketFunction(UArg arg0, UArg arg1);
static void txTaskFunction(UArg arg0, UArg arg1);
static void rxTaskFunction(UArg arg0, UArg arg1);
static void writeTaskFunction(UArg arg0, UArg arg1);


/* RF object definitions */
static RF_Object rfObject;
static RF_Handle rfHandle;
static RF_Params rfParams;

/* UART object definitions */
static UART_Handle uart;
static UART_Params uartParams;

/* semaphore object definitions */

// Mutex
static Semaphore_Struct semHandleStruct;
static Semaphore_Handle semHandle;
// Own Queue access
static Semaphore_Struct semOwnEmptyStruct, semOwnFullStruct;
static Semaphore_Handle semOwnEmptyHandle, semOwnFullHandle;
// RX Queue access
static Semaphore_Struct semTxRxEmptyStruct, semTxRxFullStruct;
//static Semaphore_Handle semTxRxEmptyHandle, semTxRxFullHandle;
// Valid Packet
static Semaphore_Struct semValidFalseStruct, semValidTrueStruct;
//static Semaphore_Handle semValidFalseHandle, semValidTrueHandle;
// Write
static Semaphore_Struct writeHandleStruct;
static Semaphore_Handle writeHandle;

/* Pin driver handle */
static PIN_Handle ledPinHandle;
static PIN_State ledPinState;

/* Receive dataQueue for RF Core to fill in data */
static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t packetLength;
static uint8_t* packetDataPointer;

/* Task objects and variables */
static Task_Params txTaskParams, rxTaskParams, createPacketParams, writeParams;
Task_Struct txTask, rxTask, createPacketTask, writeTask;    /* not static so you can see in ROV */
static uint8_t txTaskStack[TASK_STACK_SIZE], rxTaskStack[TASK_STACK_SIZE], createPacketStack[TASK_STACK_SIZE], writeStack[TASK_STACK_SIZE];

/* Packets and buffer */
static tPacket packetTX, packetRX;
static uint8_t packetBuffer[MAX_LENGTH];
static uint8_t tmpPacketBuffer[MAX_LENGTH];

static uint16_t seqNumber;
static uint8_t sensorID;

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

uint8_t newestIndex;
uint8_t initTask;
uint8_t writeFlag;

/*
 *  ======== main ========
 */
int main(void) {

//    TIME = 60;

//    txTaskFlag = 0;
    memset(tmpPacketBuffer, 0, sizeof(tmpPacketBuffer));
    memset(packetBuffer, 0, sizeof(packetBuffer));

    newestIndex = 0;

    writeFlag = 0;

    initTask = 1;

    sensorID = 2;

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
    /**************************************************************/

    /* WRITE */
    Semaphore_Params writeHandleParams;
    Semaphore_Params_init(&writeHandleParams);
    Semaphore_construct(&writeHandleStruct, 1, &writeHandleParams);
    writeHandle = Semaphore_handle(&writeHandleStruct);
    /**************************************************************/
    /* End of Semaphores constructions */

    RF_init();
    UARTinit();

    createPacketTask_init();
    writeTask_init();
    txTask_init();
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
    RF_cmdPropTx.pktLen = MAX_LENGTH;
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

void UARTinit() {

    UART_init();

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.dataLength = UART_LEN_8;
    uartParams.baudRate = 115200;

    uart = UART_open(Board_UART0, &uartParams);

    if (uart == NULL) {
         /* UART_open() failed */
         while (1);
    }
}

void createPacketTask_init() {

    Task_Params_init(&createPacketParams);
    createPacketParams.stackSize = TASK_STACK_SIZE;
    createPacketParams.priority = CREATE_TASK_PRIORITY;
    createPacketParams.stack = &createPacketStack;
    createPacketParams.arg0 = (UInt)1000000;

    Task_construct(&createPacketTask, createPacketFunction, &createPacketParams, NULL);
}

void txTask_init() {

    Task_Params_init(&txTaskParams);
    txTaskParams.stackSize = TASK_STACK_SIZE;
    txTaskParams.priority = TX_TASK_PRIORITY;
    txTaskParams.stack = &txTaskStack;
    txTaskParams.arg0 = (UInt)1000000;

    Task_construct(&txTask, txTaskFunction, &txTaskParams, NULL);

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

    AONBatMonEnable();
    while (1) {
        /* Run forever */
        Semaphore_pend(semOwnEmptyHandle, BIOS_WAIT_FOREVER);

        Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);


        /* Create packet with incrementing sequence number & random payload */
        packetTX.srcID = sensorID;
        packetTX.seqNumber[0] = (uint8_t)(seqNumber >> 8);
        packetTX.seqNumber[1] = (uint8_t)(seqNumber++);
        packetTX.payload = (uint8_t) AONBatMonTemperatureGetDegC();
        AONBatMonDisable();

        Semaphore_post(semHandle);

        Semaphore_post(semOwnFullHandle);

        AONBatMonEnable();
    }
}

void writeTaskFunction(UArg arg0, UArg arg1) {
    while(1) {
        /* Waiting for packet */
        Semaphore_pend(writeHandle, BIOS_WAIT_FOREVER); // DORME THREAD
        /* Writing packet to UART */
        if (writeFlag) {
            UART_write(uart, &tmpPacketBuffer, sizeof(tmpPacketBuffer));
            writeFlag = 0;
        }
//        memset(packetBuffer, 0, sizeof(packetBuffer));
    }
}

void txTaskFunction(UArg arg0, UArg arg1) {
    sleep(5);
    while (1) {
        Semaphore_post(writeHandle);
        Semaphore_pend(semOwnFullHandle, BIOS_WAIT_FOREVER);
        Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);

        uint8_t i = 0;
        uint16_t slp, momentum = 0;
        for (i = 0; i < 4; i++) {
            RF_yield(rfHandle);
            switch (i) {
            case 0:
                packetTX.txPower = (uint8_t) 14;
                insertPacket(tmpPacketBuffer, &packetTX);
                copyByteArray(packetBuffer, tmpPacketBuffer);
                tmpPacketBuffer[0]--;

                RF_cmdPropRadioDivSetup.txPower = 0xA73F;

                RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal,
                            &txcallback, 0);
                slp = rand()%500000;
                momentum += slp;
                usleep(slp);
                break;
            case 1:
                packetTX.txPower = (uint8_t) 11;
                insertPacket(tmpPacketBuffer, &packetTX);
                copyByteArray(packetBuffer, tmpPacketBuffer);
                tmpPacketBuffer[0]--;

                RF_cmdPropRadioDivSetup.txPower = 0x50DA;

                RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal,
                            &txcallback, 0);
                slp = rand()%500000;
                momentum += slp;
                usleep(slp);
                break;
            case 2:
                packetTX.txPower = (uint8_t) 8;
                insertPacket(tmpPacketBuffer, &packetTX);
                copyByteArray(packetBuffer, tmpPacketBuffer);
                tmpPacketBuffer[0]--;

                RF_cmdPropRadioDivSetup.txPower = 0x24CB;

                RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal,
                            &txcallback, 0);
                slp = rand()%500000;
                momentum += slp;
                usleep(slp);
                break;
            case 3:
                packetTX.txPower = (uint8_t) 5;
                insertPacket(tmpPacketBuffer, &packetTX);
                copyByteArray(packetBuffer, tmpPacketBuffer);
                tmpPacketBuffer[0]--;

                RF_cmdPropRadioDivSetup.txPower = 0x18C6;

                RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal,
                            &txcallback, 0);
                slp = rand()%500000;
                momentum += slp;
                usleep(slp);
                break;
            }
        }

        memset(tmpPacketBuffer, 0, sizeof(tmpPacketBuffer));
        memset(packetBuffer, 0, sizeof(packetBuffer));
        newestIndex = 0;

        Semaphore_post(semHandle);

        Semaphore_post(semOwnEmptyHandle);

        sleep(30 - (momentum/1e6));
//        sleep(5);
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
    }

}

static void rxTaskFunction(UArg arg0, UArg arg1)
{
    /* Enter RX mode and stay forever in RX */

    while (1) {

        Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);

        RF_EventMask terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropRx,
                                                  RF_PriorityNormal, &rxcallback,
                                                  RF_EventRxEntryDone);

        Semaphore_post(semHandle);
    }
}

/*
 *  ======== RX callback ========
 */

static void rxcallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventRxEntryDone)
    {
//        printf("Recebeu!\n");
        uint8_t buffer[124];
        /* Toggle pin to indicate RX */

        /* Get current unhandled data entry */
        currentDataEntry = RFQueue_getDataEntry();

        /* Handle the packet data, located at &currentDataEntry->data:
         * - Length is the first byte with the current configuration
         * - Data starts from the second byte */
//        printf("RX - ");
//        printArray(&currentDataEntry->data + 1);
//        printf("TMP - ");
//        printArray(tmpPacketBuffer);
        packetLength      = *(uint8_t*)(&currentDataEntry->data);
        packetDataPointer =  (uint8_t*)(&currentDataEntry->data + 1);

        /* Copy the payload + the status byte to the packet variable */
        memcpy(buffer, packetDataPointer, (packetLength + 1));
//        printf("B - ");
//        printArray(buffer);

        getPacket(buffer, &packetRX);
//        printf("%d\n", packetRX.srcID);

        if (packetRX.srcID > sensorID) {

            if (packetRX.srcID == (sensorID + 1) && initTask) {
                initTask = 0;
                txTask_init();
            }

            packetRX.rtrID = (uint8_t) sensorID;
            packetRX.dstID = (uint8_t) sensorID;
            packetRX.rssi = (uint8_t) rxStatistics.lastRssi;
//            printf("Valido\n");
            int i, j, k;

            if (buffer[0] > 0 && tmpPacketBuffer[0] > 0) {
                if ((buffer[0] + tmpPacketBuffer[0]) >= 15) {
                    newestInsert(tmpPacketBuffer, &packetRX);
                }
                else {
                    i = buffer[0] * sizeof(tPacket);
                    j = tmpPacketBuffer[0] * sizeof(tPacket);
                    for (k = 1; k <= i; k++){
                        tmpPacketBuffer[j+k] = buffer[k];
                    }
                    tmpPacketBuffer[0] += buffer[0];
                    copyPacketToArray(tmpPacketBuffer, &packetRX);
                }
            }
            else if (tmpPacketBuffer[0] > 0) {
                i = buffer[0] * sizeof(tPacket);
                j = tmpPacketBuffer[0] * sizeof(tPacket);
                for (k = 1; k < i; k++){
                    tmpPacketBuffer[j+k] = buffer[k];
                }
                copyPacketToArray(tmpPacketBuffer, &packetRX);
            }
            else {
                i = buffer[0] * sizeof(tPacket);
                j = tmpPacketBuffer[0] * sizeof(tPacket);
                for (k = 0; k < i; k++){
                    tmpPacketBuffer[j+k] = buffer[k];
                }
                copyPacketToArray(tmpPacketBuffer, &packetRX);
            }
            PIN_setOutputValue(ledPinHandle, Board_PIN_LED1, !PIN_getOutputValue(Board_PIN_LED1));
        }
        RFQueue_nextEntry();
    }
}

void insertPacket(uint8_t *arr, tPacket *p) {
    int index = arr[0] * sizeof(tPacket);
    if (index < 0) index = 0;
    arr[index+1] = p->srcID; // source of packet - sensor ID
    arr[index+2] = p->rtrID;
    arr[index+3] = p->dstID;
    arr[index+4] = p->seqNumber[0];
    arr[index+5] = p->seqNumber[1];
    arr[index+6] = p->payload;
    arr[index+7] = p->txPower;
    arr[index+8] = p->rssi;
    arr[0]++;
}

void copyPacketToArray(uint8_t *arr, tPacket *p) {
    int index = (arr[0] - 1) * sizeof(tPacket);
    if (index < 0) index = 0;
    arr[index+1] = p->srcID; // source of packet - sensor ID
    arr[index+2] = p->rtrID;
    arr[index+3] = p->dstID;
    arr[index+4] = p->seqNumber[0];
    arr[index+5] = p->seqNumber[1];
    arr[index+6] = p->payload;
    arr[index+7] = p->txPower;
    arr[index+8] = p->rssi;
}

void getPacket(uint8_t *arr, tPacket *p) {
    int index = (arr[0] - 1) * sizeof(tPacket);
    if (index < 0) index = 0;
    p->srcID = arr[index+1]; // source of packet - sensor ID
    p->rtrID = arr[index+2];
    p->dstID = arr[index+3];
    p->seqNumber[0] = arr[index+4];
    p->seqNumber[1] = arr[index+5];
    p->payload = arr[index+6];
    p->txPower = arr[index+7];
    p->rssi = arr[index+8];
}

void copyByteArray(uint8_t *to, uint8_t *from) {
    int i;
    for (i = 0; i < 124; i++) {
        to[i] = from[i];
    }
}

void printArray(uint8_t *arr) {
    int i;
    for (i = 0; i < 15; i++) {
        printf("%d", arr[i]);
    }
    printf("\n");
}

void newestInsert(uint8_t *arr, tPacket *p) {
    arr[newestIndex+1] = p->srcID; // source of packet - sensor ID
    arr[newestIndex+2] = p->rtrID;
    arr[newestIndex+3] = p->dstID;
    arr[newestIndex+4] = p->seqNumber[0];
    arr[newestIndex+5] = p->seqNumber[1];
    arr[newestIndex+6] = p->payload;
    arr[newestIndex+7] = p->txPower;
    arr[newestIndex+8] = p->rssi;
    newestIndex += 8;
    arr[0]--;
}
