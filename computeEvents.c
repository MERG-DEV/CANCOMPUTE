
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
#include "actionQueue.h"
#include "FliM.h"

// forward declarations
void clearEvents(unsigned char i);
void doSOD(void);
void doWait(unsigned int duration);

extern BOOL sendProducedEvent(unsigned char action, BOOL on);
BOOL sendInvertedProducedEvent(PRODUCER_ACTION_T action, BOOL state, BOOL invert);
extern BOOL needsStarting(unsigned char io, unsigned char action, unsigned char type);
extern BOOL completed(unsigned char io, unsigned char action, unsigned char type);

static TickValue startWait;

void computeEventsInit(void) {
    startWait.Val = 0;
}

/**
 * Set Global Events back to factory defaults.
 */
void factoryResetGlobalEvents(void) {
    // we don't create a default SOD event
}

/**
 * Reset events for the IO back to default. Called when the Type of the IO
 * is changed.
 * @param io the IO number
 */
void defaultEvents(unsigned char io, unsigned char type) {
    WORD en = io+1;
    clearEvents(io); 
#ifdef TEST_DEFAULT_EVENTS
    // add the module's default events for this io
    switch(type) {
        case TYPE_OUTPUT:
        case TYPE_BOUNCE:
            /*
             * We create both a Produced and a Consumed event here.
             */
            // Consume ACON/ASON and ACOF/ASOF events with en as port number
            addEvent(nodeID, en, 1, ACTION_IO_CONSUMER_OUTPUT_EV(io), TRUE);
            addEvent(nodeID, 200+en, 1, ACTION_IO_CONSUMER_OUTPUT_FLASH(io), TRUE);
//            addEvent(nodeID, 100+en, 0, ACTION_IO_PRODUCER_OUTPUT(io), TRUE);
            break;
        case TYPE_INPUT:
            // Produce ACON/ASON and ACOF/ASOF events with en as port number
 //           addEvent(nodeID, en, 0, ACTION_IO_PRODUCER_INPUT(io), TRUE);
            break;
        case TYPE_SERVO:
            // Produce ACON/ASON and ACOF/ASOF events with en as port number
//            addEvent(nodeID, 100+en, 0, ACTION_IO_PRODUCER_SERVO_START(io), TRUE);
//            addEvent(nodeID, 300+en, 0, ACTION_IO_PRODUCER_SERVO_MID(io), TRUE);
//            addEvent(nodeID, 200+en, 0, ACTION_IO_PRODUCER_SERVO_END(io), TRUE);
            // Consume ACON/ASON and ACOF/ASOF events with en as port number
            addEvent(nodeID, en, 1, ACTION_IO_CONSUMER_SERVO_EV(io), TRUE);
            break;
        case TYPE_MULTI:
            // no defaults for multi
            break;
        case TYPE_ANALOGUE_IN:
            // Produce ACON/ASON and ACOF/ASOF events with en as port number
 //           addEvent(nodeID, en, 0, ACTION_IO_PRODUCER_ANALOGUE(io), TRUE);
            break;
        case TYPE_MAGNET:
            // Produce ACON/ASON and ACOF/ASOF events with en as port number
 //           addEvent(nodeID, en, 0, ACTION_IO_PRODUCER_MAGNETH(io), TRUE);
 //           addEvent(nodeID, 100+en, 0, ACTION_IO_PRODUCER_MAGNETL(io), TRUE);
            break;
    }
#endif
}

/**
 * If we don't define default produced actions in the eventTable above then
 * we can generate the event using code. This was requested by PeteB so that
 * eventTable space isn't used unnecessarily. The downside is that NERD won't
 * list these events.
 * If a default event is defined here then it should be written to the global
 * producedEvent variable.
 * 
 * @param action
 * @return true if there is an event
 */
BOOL getDefaultProducedEvent(PRODUCER_ACTION_T paction) {
    
    return FALSE;
}

/**
 * Clear the events for the IO. Called prior to setting the default events.
 * @param i the IO number
 */
void clearEvents(unsigned char io) {
    
}

/**
 * Process the consumed events. Extract the actions from the EVs. 
 * The actions are pushed onto the actionQueue for subsequent processing in
 * sequence.
 * 
 * If an event is defined to have actions A1, A2, A3, A4 and A2 has the SIMULANEOUS 
 * flag set then the sequence will be executed for ON event: A1, A2&A3, A4 and
 * we therefore put:
 *  A1 (sequential), A2 (simultaneous), A3 (sequential, A4 (sequential) 
 * into the action queue.
 * 
 * For an OFF event we want the sequence: A4, A3&A2, A1 and therefore we put:
 *  A4 (sequential), A3 (simultaneous), A2 (sequential), A1 (sequential)
 * into the action queue. Therefore when doing an OFF Event we need to fiddle
 * with the SIMULTANEOUS bit.
 * 
 * @param tableIndex the required action to be performed.
 * @param msg the full CBUS message so that OPC  and DATA can be retrieved.
 */
void processEvent(BYTE tableIndex, BYTE * msg) {
    unsigned char e;
    unsigned char io;
    unsigned char ca;
    int action;

    BYTE opc = getEVs(tableIndex);
#ifdef SAFETY
    if (opc != 0) {
        return; // error getting EVs. Can't report the error so just return
    }
#endif
    opc=msg[d0];

}

// record the last action started so we can tell if we are starting a new action 
static unsigned char lastAction = NO_ACTION;
/**
 * This needs to be called on a regular basis to see if any
 * actions have finished and the next needs to be started.
 */
void processActions(void) {
    unsigned char io;
    unsigned char type;
    CONSUMER_ACTION_T action = getAction();
    CONSUMER_ACTION_T ioAction;
    unsigned char simultaneous;
    unsigned char peekItem;
    
    
    if (action == NO_ACTION) {
        doneAction();
        return;
    }
    // Check for SOD
    if (action == ACTION_CONSUMER_SOD) {
        // Do the SOD
        doSOD();
        doneAction();
        return;
    }
    if (action == ACTION_CONSUMER_WAIT05) {
        doWait(5);
        return;
    }
    if (action == ACTION_CONSUMER_WAIT1) {
        doWait(10);
        return;
    }
    if (action == ACTION_CONSUMER_WAIT2) {
        doWait(20);
        return;
    }
    if (action == ACTION_CONSUMER_WAIT5) {
        doWait(50);
        return;
    }
    
}

/**
 * Stop processing actions for a while.
 * 
 * @param duration in 0.1second units
 */
void doWait(unsigned int duration) {
    // start the timer
    if (startWait.Val == 0) {
        startWait.Val = tickGet();
        return;
    } else {
        // check if timer expired
        if ((tickTimeSince(startWait) > ((long)duration * (long)HUNDRED_MILI_SECOND))) {
            doneAction();
            startWait.Val = 0;
            return;
        } 
    }
}
/**
 * Do the consumed SOD action. This sends events to indicate current state of the system.
 */
void doSOD(void) {
    
}

BOOL sendInvertedProducedEvent(PRODUCER_ACTION_T action, BOOL state, BOOL invert) {
    return sendProducedEvent(action, invert?!state:state);
}
