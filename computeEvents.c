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
#include "ComputeActions.h"

#pragma udata large_rx_buffer
RxBuffer rxBuffers[NUM_BUFFERS];
#pragma udata
static BYTE bufferIndex;    // where to write the next event
extern BYTE timeLimit;

// forward declarations
BYTE currentEventState[NUM_EVENTS]; // use a BYTE even though a BOOL would do


void computeEventsInit(void) {
    for (bufferIndex=0; bufferIndex < NUM_EVENTS; bufferIndex++) {
        currentEventState[bufferIndex] = 0;
    }
    bufferIndex=0;
    
}


/**
 * Process the consumed events. 
 * Store the events in the rxBuffer along with the time received.
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
    if (opc&EVENT_ON_MASK) {
        rxBuffers[bufferIndex].eventNoAndOnOff = ev | EVENT_ON; // the user's reference index is in ev#1
    } else {
        rxBuffers[bufferIndex].eventNoAndOnOff = EVENT_NO(ev); // the user's reference index is in ev#1
    }
    /* disable the timer to prevent roll over of the lower 16 bits while before/after reading of the extension */
    TMR_IE = 0;
    rxBuffers[bufferIndex].time=globalTimeStamp;  
    /* enable the timer*/
    TMR_IE = 1;
    // store current state
    currentEventState[ev] = !(opc&EVENT_ON_MASK);
        
    bufferIndex++;
    if (bufferIndex >= NUM_BUFFERS) bufferIndex = 0;
}

// No default events
BOOL getDefaultProducedEvent(PRODUCER_ACTION_T paction) {
    return FALSE;
}

/**
 * Return true if the event has been received within the time window.
 * @param eventNo the event number (not the EN) including on/off
 * @return true if the event has been received within the time window
 */
BOOL receivedEvent(BYTE eventNo) {
    BYTE bi;
    BYTE i;
    
    if (bufferIndex == 0) {
        bi = NUM_BUFFERS;
    } else {
        bi = bufferIndex - 1;
    }
    
    // go backwards through the buffer list until we exceed the time limit
    for (i=0; i<NUM_BUFFERS; i++) {
        if ((globalTimeStamp - rxBuffers[bi].time) > timeLimit) break;
        if (rxBuffers[bi].eventNoAndOnOff == eventNo) {
            return TRUE;
        }
        if (bi == 0) {
            bi = NUM_BUFFERS;
        } else {
            bi--;
        }
    }
    return FALSE;
}

/**
 * Count the number of times we have received the specified event within the specified timeLimit time.
 * @param eventNo the event number (not the EN) including on/off
 * @return the count of the number of times the event has been received within the time window
 */
BYTE countEvent(BYTE eventNo) {
    BYTE bi;
    BYTE ret = 0;
    BYTE i;
    
    if (bufferIndex == 0) {
        bi = NUM_BUFFERS;
    } else {
        bi = bufferIndex - 1;
    }
    
     // go backwards through the buffer list until we exceed the time limit
    for (i=0; i<NUM_BUFFERS; i++) {
        if ((globalTimeStamp - rxBuffers[bi].time) > timeLimit) break;
        if (rxBuffers[bi].eventNoAndOnOff == eventNo) {
            ret++;
            if (ret == 255) return ret;
        }
        if (bi == 0) {
            bi = NUM_BUFFERS;
        } else {
            bi--;
        }
    }
    return ret;
}

/**
 *  return TRUE if event1 and event2 are within time and event1 occurs before event2
 * @param event1 the event number (not the EN) including on/off
 * @param event2 the event number (not the EN) including on/off
 * @return true if the event has been received within the time window
 */
BYTE sequence2 (BYTE event1, BYTE event2) {
    BYTE bi;
    BYTE ret = 0;
    BYTE i;
    
    if (bufferIndex == 0) {
        bi = NUM_BUFFERS;
    } else {
        bi = bufferIndex - 1;
    }
 
    for (i=0; i<NUM_BUFFERS; i++) {
        if ((globalTimeStamp - rxBuffers[bi].time) > timeLimit) break;
        if (rxBuffers[bi].eventNoAndOnOff == event2) {
            for ( ; i<NUM_BUFFERS; i++) {
                if ((globalTimeStamp - rxBuffers[bi].time) > timeLimit) break;
                if (rxBuffers[bi].eventNoAndOnOff == event1) {
                    return TRUE;
                }
                if (bi == 0) {
                    bi = NUM_BUFFERS;
                } else {
                    bi--;
                }
            }
            return FALSE;
        }
        if (bi == 0) {
            bi = NUM_BUFFERS;
        } else {
            bi--;
        }
    }
    return FALSE;
}


/**
 *  return TRUE if all the event and event are within time and in the right order
 * @param event1 the number of events
 * @param event2 the NV index to the start of the event list
 * @return true if all the events have been received within the time window
 */
BYTE sequenceMulti(BYTE num, BYTE nvIndex) {
    BYTE bi;
    BYTE ret = 0;
    BYTE i;
    BYTE nvi = nvIndex;
    
    if (bufferIndex == 0) {
        bi = NUM_BUFFERS;
    } else {
        bi = bufferIndex - 1;
    }
    if (num == 0) return TRUE;
 
    for (i=0; i<NUM_BUFFERS; i++) {
        if ((globalTimeStamp - rxBuffers[bi].time) > timeLimit) break;  // didn't find all in time
        
        if (rxBuffers[bi].eventNoAndOnOff == getNv(nvi)) {  // found next event
            nvi++;
            if (nvIndex >= nvIndex+num) return TRUE;    // reached end
        }
        if (bi == 0) {
            bi = NUM_BUFFERS;
        } else {
            bi--;
        }
    }
    return FALSE;
}