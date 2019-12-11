/*
 * packet.c
 *
 *  Created on: 31 de out de 2019
 *      Author: Left
 */


#include "packet.h"
#include <stdlib.h>

void initPacket(tPacket *p) {
    if (p == NULL) p = malloc(sizeof(tPacket));
    p->srcID = 0; // source of packet - sensor ID
    p->seqNumber[0] = 0;
    p->seqNumber[1] = 0;
    p->txPower = 0;
    p->dstID = 0;
    p->rssi = 0;
    p->payload = 0; // temperature
}

