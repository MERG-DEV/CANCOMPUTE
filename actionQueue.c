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
 * File:   actionQueue.c
 * Author: Ian
 *
 * Created on 1 June 2017, 13:14
 *
 * A queue of actions waiting to be processed by the main loop.
 * Implemented as two separated queues - a normal and an expedited priority queue.
 */

#include "GenericTypeDefs.h"
#include "module.h"
#include "computeEvents.h"
#include "actionQueue.h"
#include "queue.h"

//BYTE normalReadIdx[NUM_ACTION_QUEUES];                   // index of the next to read
//BYTE normalWriteIdx[NUM_ACTION_QUEUES];                  // index of the next to write
COMPUTE_ACTION_T normalQueueBuf[NUM_ACTION_QUEUES][ACTION_QUEUE_SIZE];   // the actual cyclic buffer space
Queue normalQueue[NUM_ACTION_QUEUES];
static BYTE currentPushQueue;

/**
 * Initialise the action queue.
 */
void actionQueueInit(void) {
    BYTE i;
    for (i=0; i<NUM_ACTION_QUEUES;i++) {
        normalQueue[i].size = ACTION_QUEUE_SIZE;
        normalQueue[i].readIdx = 0;
        normalQueue[i].writeIdx = 0;
        normalQueue[i].queue = normalQueueBuf[i];
    }
    currentPushQueue = 0;
}

/**
 * Put the action onto the list of actions to be processed.
 *
 * @param a the action to be processed
 */
BOOL pushAction(COMPUTE_ACTION_T a) {
    return push(&(normalQueue[currentPushQueue]), a);
}



/**
 * Get the action we need to be progressing.
 *
 * @return the action
 */
COMPUTE_ACTION_T getAction(BYTE q) {
	return peekActionQueue(q, 0);
}

/**
 * Pull the next action from the buffer.
 *
 * @return the next action
 */
COMPUTE_ACTION_T popAction(BYTE q) {
    return pop(&(normalQueue[q]));
}

/**
 * Change the value on a queue.
 * @param q
 * @param newVal
 */
void changeAction(BYTE q, COMPUTE_ACTION_T newVal) {
    change(&(normalQueue[q]), newVal);
}

/**
 * Mark as having completed the current action.
 */
void doneAction(BYTE q) {
	popAction(q);
}


/**
 * Peek an item in the queue. Does not remove the item from the queue.
 * @param index the item index within the queue
 * @return the Action or NO_ACTION 
 */
COMPUTE_ACTION_T peekActionQueue(BYTE q, unsigned char index) {
    if (index < quantity(&(normalQueue[q]))) {
        return peek(&(normalQueue[q]), index);
    }
    return NO_ACTION;
}


/**
 * Delete an item in the queue. Replace the item with NO_ACTION.
 * @param index the item index within the queue
 */
void deleteActionQueue(BYTE q, unsigned char index) {
     delete(&(normalQueue[q]), index);
}

void nextQueue(void) {
    currentPushQueue++;
    if (currentPushQueue >= NUM_ACTION_QUEUES) {
        currentPushQueue = 0;
    }
}