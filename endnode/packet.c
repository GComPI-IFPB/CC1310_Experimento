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
    p->packetSrcID = 0;
    p->packetID[0] = 0;
    p->packetID[1] = 0;
    p->temp = 0;
    p->jump_count = 0;
}

