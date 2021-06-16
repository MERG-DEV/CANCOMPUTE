
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
 * File:   main.c
 * Author: Ian Hogg
 * 
 * This is the main for the CANCOMPTE module.
 * 
 * Timer usage:
 * TMR0 used in ticktime for symbol times. 
 *
 * Created on 10 April 2017, 10:26
 */
/** TODOs
 * * Work out if we need the event hash
 * * writing NVs may not be reliable
 * * Very first event sent out seems to disappear. It is put into ECAN tx buffer but never comes out.
 * * Support when <event> within ... i.e. allow event to be used as a boolean
 * * Handle the within <time> timeout clause
 * 
 * DONES:
 * * Move rules, expressions, operands, actions to Flash
 * * Run rules expression regularly, 
 * * Store result of rule expression so can detect changes
 * * Execute actions on result of expression
 * * Handle ARDAT
 * * receive to check times
 * * Multiple actionQueues
 * 
 */
/*
 * Flash memory is laid out thus:
 * Usage            Qty     Size    Total Size  Type      Address
 * NVs              256     1       0x0100      Flash     0x7F00 - 0x7FFF
 * Events           100     7       0x02BC      Flash     0x7C44 - 0x7EFF
 * Rules            50      4       0x00C8      Flash     0x7B7C - 0x7C43
 * Expressions      200     3       0x0258      Flash     0x7604 - 0x7B7B
 * RuleIndex        1       1       0x0001      Flash     0x7603 - 0x7603
 * ExpressionIndex  1       1       0x0001      Flash     0x7602 - 0x7602
 * ruleState        1       1       0x0001      Flash     0x7601 - 0x7601
 * NvPtr            1       1       0x0001      Flash     0x7600 - 0x7600
 * 
 * RAM is also use for the buffers
 * Rxbuf            200     4       800         RAM
 */

/**
 *	The Main CANCOMPUTE program supporting configurable I/O.
 */

#include "devincs.h"
#include <stddef.h>
#include "module.h"
#include "cancompute.h"
#include "computeFLiM.h"
#include "config.h"
#include "StatusLeds.h"
#include "computeEEPROM.h"
#include "events.h"
#include "computeNv.h"
#include "FliM.h"
#include "romops.h"
#include "can18.h"
#include "cbus.h"
#include "actionQueue.h"
#include "computeActions.h"
#include "rules.h"

#ifdef NV_CACHE
#include "nvCache.h"
#endif

#ifdef __18CXX
void ISRLow(void);
void ISRHigh(void);
#endif

// forward declarations
void __init(void);
BOOL checkCBUS( void);
void ISRHigh(void);
void initialise(void);
void factoryReset(void);
void factoryResetGlobalNv(void);
BOOL sendProducedEvent(unsigned char action, BOOL on);
void factoryResetEE(void);
void factoryResetFlash(void);

extern void load(void);

#ifdef __18CXX
void high_irq_errata_fix(void);

/*
 * Interrupt vectors (moved higher when bootloader present)
 */

// High priority interrupt vector

#ifdef BOOTLOADER_PRESENT
    #pragma code high_vector=0x808
#else
    #pragma code high_vector=0x08
#endif


//void interrupt_at_high_vector(void)

void HIGH_INT_VECT(void)
{
    _asm
        CALL high_irq_errata_fix, 1
    _endasm
}

/*
 * See 18F2480 errata
 */
void high_irq_errata_fix(void) {
    _asm
        POP
        GOTO ISRHigh
    _endasm
}

// low priority interrupt vector

#ifdef BOOTLOADER_PRESENT
    #pragma code low_vector=0x818
#else
    #pragma code low_vector=0x18
#endif

void LOW_INT_VECT(void)
{
    _asm GOTO ISRLow _endasm
}
#endif

static TickValue   startTime;
//static BOOL        started = FALSE;
static TickValue   lastRulesPollTime;
//static TickValue   lastRulePollTime;
/* This is used to stamp event times and to do the "within" comparisons */
WORD globalTimeStamp;

#ifdef BOOTLOADER_PRESENT
// ensure that the bootflag is zeroed
#pragma romdata BOOTFLAG
rom BYTE eeBootFlag = 0;
#endif


COMPUTE_ACTION_T NO_ACTION = {NOP,0};

// MAIN APPLICATION
#pragma code
/**
 * It is all run from here.
 * Initialise everything and then loop receiving and processing CAN messages.
 */
#ifdef __18CXX
void main(void) {
#else
int main(void) @0x800 {
#endif
    BYTE sodDelay;
    BOOL started = FALSE;

    initRomOps();
#ifdef NV_CACHE
    // If we are using the cache make sure we get the NVs early in initialisation
    NV = loadNvCache(); // replace pointer with the cache
#endif
    // Both LEDs off to start with during initialisation
    initStatusLeds();

    startTime.Val = tickGet();
    lastRulesPollTime.Val = startTime.Val;
//    lastRulePollTime.Val = startTime.Val;
    globalTimeStamp = 0;
    
    initialise(); 
    sodDelay = getNv(NV_SOD_DELAY);
    while (TRUE) {
        // Startup delay for CBUS about 2 seconds to let other modules get powered up - ISR will be running so incoming packets processed
        if (!started) {
            if ( (tickTimeSince(startTime) > (sodDelay * HUNDRED_MILI_SECOND) + TWO_SECOND)) {
                started = TRUE;
                sendProducedEvent(ACTION_PRODUCER_SOD, TRUE);
            }
        }
        checkCBUS();    // Consume any CBUS message and act upon it
        FLiMSWCheck();  // Check FLiM switch for any mode changes
        
        if (tickTimeSince(lastRulesPollTime) > 100*ONE_MILI_SECOND) {
            globalTimeStamp++;
            if (started) {
                runRules();
                lastRulesPollTime.Val = tickGet();
                processActions();
            }
        }
        // Check for any flashing status LEDs
        checkFlashing();
     } // main loop
} // main
 

/**
 * The order of initialisation is important.
 */
void initialise(void) {
    // enable the 4x PLL
    OSCTUNEbits.PLLEN = 1; 
    
    // check if EEPROM is valid
   if (ee_read((WORD)EE_VERSION) != EEPROM_VERSION) {
        // may need to upgrade of data in the future
        // set EEPROM to default values
        factoryResetEE();
        // If the FCU has requested EE rewrite then they also want to reset events and NVs
        factoryResetFlash();
        // set the reset flag to indicate it has been initialised
        ee_write((WORD)EE_VERSION, EEPROM_VERSION);
        writeFlashByte((BYTE*)(AT_NV + NV_VERSION), (BYTE)FLASH_VERSION);
#ifdef NV_CACHE
        loadNvCache();                
#endif
    }
    // check if FLASH is valid
    if (getNv(NV_VERSION) != FLASH_VERSION) {
        if (getNv(NV_VERSION) == 1) {
            /* upgrade from version 1 to version 2 */
            /* NUM_EXPRESSIONS changed from 100 to 200. This also pushed the
             * statistics down in memory. Fortunately the NVs and events are not affected.
             * To fix this we can just reload the rules and expressions from NVs.
             */
            load();  // just reload
            computeEventsInit();
        } else {
            // set Flash to default values
            factoryResetFlash();
        }
        // set the version number to indicate it has been initialised
        writeFlashByte((BYTE*)(AT_NV + NV_VERSION), (BYTE)FLASH_VERSION);
#ifdef NV_CACHE
        loadNvCache();                
#endif
    }
    initTicker(0);  // set low priority
    // Enable PORT B weak pullups
    INTCON2bits.RBPU = 0;
    // RB bits 0,1,4,5 need pullups
    WPUB = 0x33;
    ruleInit();
    computeEventsInit();
    // The init routine doesn't initialise the values
    NO_ACTION.op = ACTION_OPCODE_NOP;
    NO_ACTION.arg = 0;
    actionQueueInit();
    initActions();
    computeEventsInit();
    computeFlimInit(); // This will call FLiMinit, which, in turn, calls eventsInit, cbusInit

    // default to all digital IO
    ANCON0 = 0x00;
    ANCON1 = 0x00; 

    /*
     * Now configure the interrupts.
     * Low priority interrupt used for CAN and tick timer.
     */
    
    // Enable interrupt priority
    RCONbits.IPEN = 1;
    // enable interrupts, all init now done
    ei(); 
}

/**
 * set EEPROM to default values
 */
void factoryResetEE(void) {
    ee_write((WORD)EE_BOOT_FLAG, 0);
    ee_write((WORD)EE_CAN_ID, DEFAULT_CANID);
    ee_write_short((WORD)EE_NODE_ID, DEFAULT_NN); 
    ee_write((WORD)EE_FLIM_MODE, fsSLiM);
}

/**
 * Set up the EEPROM and Flash.
 * Should get called once on first power up or upon NNRST CBUS command. 
 * Initialise EEPROM and Flash.
 */
void factoryReset(void) {
    factoryResetEE();
    factoryResetFlash();
}

void factoryResetFlash(void) {
    factoryResetGlobalNv();
    clearAllEvents();
    
    writeFlashByte((BYTE*)(AT_RULEINDEX), (BYTE)0);
    writeFlashByte((BYTE*)(AT_EXPRESSIONINDEX), (BYTE)0);
    writeFlashByte((BYTE*)(AT_RULESTATE), (BYTE)0);
    writeFlashByte((BYTE*)(AT_NVPTR), (BYTE)1);

    flushFlashImage();
}

unsigned char isSuitableTimeToWriteFlash() {
    return TRUE;
}

/**
 * Check to see if a message has been received on the CBUS and process 
 * it if one has been received.
 * @return true if a message has been received.
 */
BOOL checkCBUS( void ) {
    BYTE    msg[20];

    if (cbusMsgReceived( 0, (BYTE *)msg )) {
        shortFlicker();         // short flicker LED when a CBUS message is seen on the bus
        if (parseCBUSMsg(msg)) {               // Process the incoming message
            longFlicker();      // extend the flicker if we processed the message
            return TRUE;
        }
        if (thisNN(msg)) {
            // handle the module specifics
            switch (msg[d0]) {
            case OPC_NNRSM: // reset to manufacturer defaults
                factoryReset();
                return TRUE;
            case OPC_NNRST: // restart
                // if we just call main then the stack won't be reset and we'd also want variables to be nullified
                // instead call the RESET vector (0x0000)
                Reset();
 /*           case OPC_ARDAT:
                doArdat();
                return TRUE;*/
            }
        }
    }
    return FALSE;
}


#ifdef __18CXX
// C intialisation - declare a copy here so the library version is not used as it may link down in bootloader area
extern const rom near BYTE * NvBytePtr;

extern rom near EventTable * eventTable;

void __init(void)
{
    // if using c018.c the routine to initialise data isn't called. Explicitly setting here is more efficient
    NvBytePtr = (const rom near BYTE*)AT_NV;
    eventTable = (rom near EventTable*)AT_EVENTS;
}

// Interrupt service routines
#if defined(__18CXX)
#pragma interruptlow ISRLow
void ISRLow(void) {
#elif defined(__dsPIC30F__) || defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24FK__) || defined(__PIC24H__)
    void _ISRFAST __attribute__((interrupt, auto_psv)) _INT1Interrupt(void)
#elif defined(__PIC32MX__)
    void __ISR(_EXTERNAL_1_VECTOR, ipl4) _INT1Interrupt(void)
#else
    void interrupt low_priority low_isr(void) {
#endif
    tickISR();
    canInterruptHandler();
}

// Interrupt service routines

#if defined(__18CXX)
#pragma interruptlow ISRHigh
void ISRHigh(void) {
#elif defined(__dsPIC30F__) || defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24FK__) || defined(__PIC24H__)
    void _ISRFAST __attribute__((interrupt, auto_psv)) _INT1Interrupt(void)
#elif defined(__PIC32MX__)
    void __ISR(_EXTERNAL_1_VECTOR, ipl4) _INT1Interrupt(void)
#else 
    void interrupt high_priority high_isr (void) {
#endif

}

