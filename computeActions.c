#include "TickTime.h"
#include "computeEvents.h"
#include "actionQueue.h"
#include "computeActions.h"
#include "cbus.h"

// forward declarations
void doWait(BYTE q, unsigned int duration);
void sendCBUS(BYTE nVoffset);
void processActionsQueue(BYTE q);

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
        processActionsQueue(q);
    }
}

void processActionsQueue(BYTE q) {
    BYTE op;
    BYTE arg;
    while (1) {
        COMPUTE_ACTION_T action = getAction(q);

        op = getNv(action.dataIndex++);
        
        switch (op) {
            case SEND:
                // send the event
                arg = getNv(action.dataIndex++);
                if (EVENT_STATE(arg) == EVENT_ON) {
                    sendProducedEvent(EVENT_NO(arg), TRUE);
                } else {
                    sendProducedEvent(EVENT_NO(arg), FALSE);
                }
                break;
            case DELAY:
                arg = getNv(action.dataIndex++);
                if (startWait[q].Val == 0) {
                    startWait[q].Val = tickGet();
                }
                // still doing delay?
                if (tickTimeSince(startWait[q]) < ((long)arg * (long)HUNDRED_MILI_SECOND)) {
                    return;
                }
                // delay over
                startWait[q].Val = 0;
                break;
            case CBUS:
                // send the cbus message
                sendCBUS(action.dataIndex);
                arg = getNv(action.dataIndex);   // the CBUS OPC
                arg = (arg >> 5);       // number of arguments
                action.dataIndex += (arg+1);           // skip forward
                break;
            default:
                // finished this action list
                doneAction(q);
                return;
        }
        // move to next data
        changeAction(q, action);
	}
}


/**
 * Send a CBUS message. 
 * @parmam nVoffset is the offset into NVs which contains the CBUS OPC and subsequent
 * data bytes.
 */
void sendCBUS(BYTE nVoffset) {
    BYTE msg[d0+8];
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