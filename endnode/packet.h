/*
 * packet.h
 *
 *  Created on: 7 de out de 2019
 *      Author: Allan Bispo
 */

#include <stdint.h>

#ifndef PACKET_H_
#define PACKET_H_

typedef struct {
    uint8_t srcID; // source of packet - sensor ID
    uint8_t rtrID;
    uint8_t dstID;
    uint8_t seqNumber[2]; // packetID
    uint8_t payload; // data to be transmitted
    uint8_t txPower;
    uint8_t rssi;
}tPacket;

void initPacket(tPacket *p);

#endif /* PACKET_H_ */
