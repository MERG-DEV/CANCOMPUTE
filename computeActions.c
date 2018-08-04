#include "TickTime.h"
#include "computeEvents.h"
#include "actionQueue.h"

// forward declarations
void doWait(unsigned int duration);

static TickValue startWait;

/**
 * This needs to be called on a regular basis to see if any
 * actions have finished and the next needs to be started.
 */
void processActions(void) {
    ACTION_T action = getAction();

    if (action.op == NOP) {
        doneAction();
        return;
    }
    // Check for SOD

    if (action.op == DELAY) {
        doWait(action.arg);
        return;
    }
    if (action.op == SEND_ON) {
        sendProducedEvent(action.arg, TRUE);
        return;
    }
    if (action.op == SEND_OFF) {
        sendProducedEvent(action.arg, FALSE);
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