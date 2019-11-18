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
    uint8_t src; // 7/10/13
    uint8_t to;  // 8/11/14
    uint8_t rssi; // 9/12/15
}pair;

typedef struct {
    uint8_t packetSrcID; // source of packet - sensor ID
    uint8_t packetID[2]; // packet identification
    uint8_t temp;
//    uint32_t temp;
    uint8_t data[2]; // data to be transmitted
    uint8_t jump_count;
    pair jumps[3]; // packet path through the nodes
}tPacket;

void initPacket(tPacket *p);

#endif /* PACKET_H_ */
