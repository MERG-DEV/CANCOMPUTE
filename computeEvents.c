/*
 Routines for CBUS FLiM operations - part of CBUS libraries for PIC 18F
  This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material
    The licensor cannot revoke these freedoms as long as you follow the license terms.
    Attribution : You must give appropriate credit, provide a link to the license,
                   and indicate if changes were made. You may do so in any reasonable manner,
                   but not in any way that suggests the licensor endorses you or your use.
    NonCommercial : You may not use the material for commercial purposes. **(see note below)
    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                  your contributions under the same license as the original.
    No additional restrictions : You may not apply legal terms or technological measures that
                                  legally restrict others from doing anything the license permits.
   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms
**************************************************************************************************************
	The FLiM routines have no code or definitions that are specific to any
	module, so they can be used to provide FLiM facilities for any module 
	using these libraries.
	
*/ 

#include "events.h"
/*
 * File:   mioEvents.c
 * Author: Ian Hogg
 * 
 * Here we deal with the module specific event handling. This covers:
 * <UL>
 * <LI>Setting of default events.</LI>
 * <li>Processing of inbound, consumed events</LI>
 *</UL>
 * 
 * Created on 10 April 2017, 10:26
 */

#include <stddef.h>
#include "module.h"
#include "GenericTypeDefs.h"
#include "romops.h"
#include "computeEEPROM.h"
#include "computeNv.h"


#include "computeEvents.h"
#include "cbus.h"
#include "FliM.h"

#pragma udata large_rx_buffer
RxBuffer rxBuffers[NUM_BUFFERS];
#pragma udata
static BYTE bufferIndex;    // where to write the next event

// forward declarations


void computeEventsInit(void) {
    bufferIndex=0;
}


/**
 * Process the consumed events. 
 * 
 * 
 * @param tableIndex the required action to be performed.
 * @param msg the full CBUS message so that OPC  and DATA can be retrieved.
 */
void processEvent(BYTE tableIndex, BYTE * msg) {
    int ev;
    BYTE opc=msg[d0];
    getEVs(tableIndex);

     // store event
    ev = getEv(tableIndex, 0);   // the user's index is in ev#1
    if (ev < 0) {
        // this shouldn't happen
        return;
    }
    rxBuffers[bufferIndex].index = ev; // the user's reference index is in ev#1
    rxBuffers[bufferIndex].on = opc&EVENT_ON_MASK;
    /* disable the timer to prevent roll over of the lower 16 bits while before/after reading of the extension */
    TMR_IE = 0;
    rxBuffers[bufferIndex].time.bytes.b1 = timerExtension1;
    rxBuffers[bufferIndex].time.bytes.b2 = timerExtension2;
    /* enable the timer*/
    TMR_IE = 1;
        
    bufferIndex++;
    if (bufferIndex >= NUM_BUFFERS) bufferIndex = 0;
}

// No default events
BOOL getDefaultProducedEvent(PRODUCER_ACTION_T paction) {
    return FALSE;
}

/*
 * Count the number of times we have received the specified event within the specified time
 */
BYTE received(BYTE eventNo, BOOL on) {
    BYTE i;
    BYTE ret = 0;
    if (rxBuffers[i].index == eventNo) {
        if (rxBuffers[i].on == on) {
            ret++;
        }
    }
    return ret;
}
