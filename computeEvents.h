
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
 * File:   mioEvents.h
 * Author: Ian
 *
 * Created on 17 April 2017, 13:14
 */

#ifndef COMPUTEEVENTS_H
#define	COMPUTEEVENTS_H

#ifdef	__cplusplus
extern "C" {
#endif
    /*
     * This is where all the module specific actions are defined.
     * The following definitions are required by the FLiM code:
     * NUM_PRODUCER_ACTIONS, NUM_CONSUMER_ACTIONS, HASH_LENGTH, EVT_NUM, 
     * EVperEVT, NUM_CONSUMED_EVENTS
     * 
     * For MIO an action is a BYTE (unsigned char). The upperupper bit is used
     * to indicate a sequential action. It would be nice to define:
     * 
     * typedef union {
     *      unsigned char action_byte;
     *      struct {
     *          unsigned char action :7;
     *          unsigned char sequential :1;
     *      }
     * } ACtION_T;
     * but C spec doesn't define what size this would be. Therefore I just use
     * BYTE (unsigned char).
     * 
     * The actions are defined below - but remember consumed actions may also have MSB
     * set indicating sequential.
     */
 
#include "cancompute.h"

extern void computeEventsInit(void);


// These are chosen so we don't use too much memory 32*20 = 640 bytes.
// Used to size the hash table used to lookup events in the events2actions table.
#define HASH_LENGTH     32
#define CHAIN_LENGTH    20

#define NUM_EVENTS              100       // must be less than 256 otherwise loops fail. These will have EV#1 with 0..99
#define EVENT_TABLE_WIDTH       1         // Width of eventTable
#define EVperEVT                1         // Max number of EVs per event

#define AT_EVENTS               (AT_NV - 0x2BC)      //(AT_NV - sizeof(EventTable)*NUM_EVENTS) Size=NUM_EVENTS * 7 = 700(0x2BC) bytes


// We'll also be using configurable produced events
#define PRODUCED_EVENTS
#define ConsumedActionType  BYTE;

#define ACTION_PRODUCER_BASE    0
#define NUM_PRODUCER_ACTIONS    256                 // Since we are using 255 for SOD this must be beyond the expected NUM_EVENTS
#define ACTION_PRODUCER_SOD     255                 // EV#1 with 255 indicates the Produced SOD

extern void processEvent(BYTE eventIndex, BYTE* message);
extern void processActions(void);
extern BYTE findEventByEV1(BYTE ev);
extern BYTE received(BYTE eventNo, BOOL on);
extern void doArdat(void);
extern BYTE sequence (BYTE event1, BOOL oo1, BYTE event2, BOOL oo2);
extern BYTE currentEventState[NUM_EVENTS];

typedef struct {
    BOOL on;
    BYTE index;
    union {
        WORD word;
        struct {
            BYTE b1;
            BYTE b2;
        } bytes;
    } time;
} RxBuffer;
#define NUM_BUFFERS 128

#include "events.h"

#ifdef	__cplusplus
}
#endif

#endif	/* COMPUTEEVENTS_H */

