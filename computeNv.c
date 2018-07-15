
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

void computeNvInit(void) {

}

/**
 * Validate value of NV based upon bounds and inter-dependencies.
 * @return TRUE is a valid change
 */
BOOL validateNV(unsigned char index, unsigned char oldValue, unsigned char value) {
    // TODO more validations
    return TRUE;
} 

void actUponNVchange(unsigned char index, unsigned char oldValue, unsigned char value) {
    // 
}


/**
 * Set NVs back to factory defaults.
 */
void factoryResetGlobalNv(void) {
    writeFlashByte((BYTE*)(AT_NV + NV_SOD_DELAY), (BYTE)0);
    writeFlashByte((BYTE*)(AT_NV + NV_HB_DELAY), (BYTE)0);
#ifdef NV_CACHE
    loadNvCache();
#endif
}

