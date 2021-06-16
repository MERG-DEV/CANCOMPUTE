
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
 * File:   nodeVarables.h
 * Author: 	Ian Hogg
 * Comments:	Definitions for NVs
 * Revision history: 
 */

#ifndef XC_NODE_VARIABLES_H
#define	XC_NODE_VARIABLES_H

/**
 * This is where the module specific NVs are specified.
 * NVs can be accessed either by byte offset (for read/set of NVs in FLiM.c)
 * or by use of a structure with named elements. These two must be kept synchronised.
 * 
 * The following are required by FLiM.c: NV_NUM, AT_NV
 */

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    
#include "GenericTypeDefs.h"
#include "cancompute.h"

//#define FLASH_VERSION   0x01
#define FLASH_VERSION   0x02    // updated when number of Expressions increased from 100 to 200
    
// Global NVs
#define NV_VERSION                      0
#define NV_RULES                        1
#define NV_SOD_DELAY                    255


/*
 * This structure is required by FLiM.h - although its fields are not actually used.
 */
typedef struct {
        BYTE nv_version;                // version of NV structure
        BYTE nv_rules[254];              // The NVs where the rules are stores
        BYTE hbDelay;                    // Interval in 100mS for automatic heartbeat. Set to zero for no heartbeat.
} ModuleNvDefs;

#define NV_NUM  255     // Number of node variables
#ifdef __18F25K80
#define AT_NV   (0x8000 - ((NV_NUM)+1))        // Where the NVs are stored. (_ROMSIZE - 256)  Size=256 bytes
#endif
#ifdef __18F26K80
#define AT_NV   0xFF00                  // Where the NVs are stored. (_ROMSIZE - 256)  Size=256 bytes
#endif

extern void computeNvInit(void);
extern unsigned int getNodeVar(unsigned int index);
extern void setNodeVar(unsigned int index, unsigned int value);
extern BOOL validateNV(BYTE nvIndex, BYTE oldValue, BYTE value);
extern void actUponNVchange(unsigned char index, unsigned char oldValue, unsigned char value);
extern void defaultNVs(unsigned char i, unsigned char type);        
extern BYTE getNv(BYTE NVindex);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_NODE_VARAIABLES_H */

