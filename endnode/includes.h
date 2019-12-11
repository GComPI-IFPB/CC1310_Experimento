/*
 * includes.h
 *
 *  Created on: 31 de out de 2019
 *      Author: Left
 */

#ifndef INCLUDES_H_
#define INCLUDES_H_

/***** Includes *****/
/* Util headers */
#include "packet.h"

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/System.h>

/* Standard C Libraries */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

/* RTOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>

/* TI Drivers */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/PIN.h>
#include <ti/devices/cc13x0/driverlib/aon_batmon.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)

/* Application Header files */
#include "RFQueue.h"
#include "smartrf_settings/smartrf_settings.h"

/* Board Header files */
#include <ti/drivers/Board.h>
#include "Board.h"

#endif /* INCLUDES_H_ */
