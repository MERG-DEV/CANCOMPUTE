#include "TickTime.h"
#include "computeEvents.h"
#include "actionQueue.h"
#include "computeActions.h"
#include "cbus.h"

// forward declarations
void doWait(BYTE q, unsigned int duration);
void sendCBUS(BYTE nVoffset);

static TickValue startWait[NUM_ACTION_QUEUES];

void initActions(void) {
    unsigned char q;
    for (q=0; q<NUM_ACTION_QUEUES; q++) {
        startWait[q].Val = 0;
    }
}

/**
 * This needs to be called on a regular basis to see if any
 * actions have finished and the next needs to be started.
 */
void processActions(void) {
    BYTE q;
    for (q=0; q<NUM_ACTION_QUEUES; q++) {
        COMPUTE_ACTION_T action = getAction(q);

        if (action.op == ACTION_OPCODE_NOP) {
            doneAction(q);
            continue;
        }
        // Check for SOD

        if (action.op == ACTION_OPCODE_DELAY) {
            doWait(q, action.arg);
            continue;
        }
        if (action.op == ACTION_OPCODE_SEND_ON) {
            sendProducedEvent(action.arg, TRUE);
            doneAction(q);
            continue;
        }
        if (action.op == ACTION_OPCODE_SEND_OFF) {
            sendProducedEvent(action.arg, FALSE);
            doneAction(q);
            continue;
        }
        if (action.op == ACTION_OPCODE_SEND_CBUS) {
            sendCBUS(action.arg);   // arg is the NV offset pointing to OPC
            doneAction(q);
        }
    }
}

/**
 * Stop processing actions for a while.
 * 
 * @param duration in 0.1second units
 */
void doWait(BYTE q, unsigned int duration) {
    // start the timer
    if (startWait[q].Val == 0) {
        startWait[q].Val = tickGet();
        return;
    } else {
        // check if timer expired
        if ((tickTimeSince(startWait[q]) > ((long)duration * (long)HUNDRED_MILI_SECOND))) {
            doneAction(q);
            startWait[q].Val = 0;
            return;
        } 
    }
}


/**
 * Send a CBUS message. 
 * @parmam nVoffset is the offset into NVs which contains the CBUS OPC and subsequent
 * data bytes.
 */
void sendCBUS(BYTE nVoffset) {
    BYTE msg[d0+4];
    BYTE i;
    BYTE len;
    msg[d0] = getNv(nVoffset);    // OPC
    // work out how many subsequent bytes
    len = (msg[d0] >> 5U);
    for (i = 1; i<=len; i++) {
        msg[d0+i] = getNv(nVoffset + i);
    }
    
    cbusSendMsg(CBUS_OVER_CAN, msg);
}