
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
 * File:   nodeVariables.c
 * Author: Ian Hogg
 *
 * Created on 05 March 2016, 18:14
 */

/**
 * Node variables contain global parameters that need to be persisted to Flash.
 */

#include "devincs.h"
#include "module.h"
#include "computeNv.h"
#include "computeEEPROM.h"
#include "events.h"
#include "romops.h"
#include "FliM.h"
#ifdef NV_CACHE
#include "nvCache.h"
#endif
#include "config.h"
#include "cbus.h"

#include "rules.h"

#ifdef __XC8
const ModuleNvDefs moduleNvDefs @AT_NV; // = {    //  Allow 128 bytes for NVs. Declared const so it gets put into Flash
#else
//#pragma romdata myNV=AT_NV
#endif
/*
 Module specific NV routines
 */
#ifdef __XC8
const NodeVarTable nodeVarTable @AT_NV;
ModuleNvDefs * NV = (ModuleNvDefs*)&(moduleNvDefs);    // pointer to the NV structure
#else
#ifdef NV_CACHE
ModuleNvDefs * NV; // = &(nodeVarTable.moduleNVs);
#else
volatile rom near ModuleNvDefs * NV = (volatile rom near ModuleNvDefs*)&(nodeVarTable.moduleNVs);    // pointer to the NV structure
#endif
#endif

extern void load(void);

void computeNvInit(void) {

}

/**
 * Validate value of NV based upon bounds and inter-dependencies.
 * @return TRUE is a valid change
 */
BOOL validateNV(unsigned char index, unsigned char oldValue, unsigned char value) {
    // all NV values are allowed
    return TRUE;
} 

void actUponNVchange(unsigned char index, unsigned char oldValue, unsigned char value) {
    // setting any NV to END will cause an attempt to convert all to the execution structures
    if ((OpCodes)value == END) {
        load();
        // send message to indicate the result
        cbusMsg[d0] = OPC_ACDAT;
        // d1 and d2 are the NN
        cbusMsg[d3] = ruleState;
        cbusMsg[d4] = nvPtr;
        if (cbusMsg[d3] == VALID) {
            cbusMsg[d5] = ruleIndex;
            cbusMsg[d6] = expressionIndex;
        } else {
            cbusMsg[d5] = 0;
            cbusMsg[d6] = 0;
        }
        cbusMsg[d7] = 0;
        cbusSendMsgMyNN( 0, cbusMsg );
    }
}


/**
 * Set NVs back to factory defaults.
 */
void factoryResetGlobalNv(void) {
    writeFlashByte((BYTE*)(AT_NV + NV_SOD_DELAY), (BYTE)0);
    writeFlashByte((BYTE*)(AT_NV + NV_RULES), END);

#ifdef NV_CACHE
    loadNvCache();
#endif
}

BYTE getNv(BYTE NVindex) {
    WORD flashIndex;
    flashIndex = AT_NV;
    flashIndex += NVindex;
    
    return readFlashBlock(flashIndex);
}