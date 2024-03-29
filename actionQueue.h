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
 * File:   actionQueue.h
 * Author: Ian
 *
 * Created on 1 June 2017, 13:14
 *
 */
#ifndef __ACTIONQUEUE_H_
#define __ACTIONQUEUE_H_



extern void actionQueueInit(void);
extern BOOL pushAction(COMPUTE_ACTION_T a);
extern COMPUTE_ACTION_T getAction(BYTE q);
extern void changeAction(BYTE q, COMPUTE_ACTION_T newVal);
extern void doneAction(BYTE q);
extern COMPUTE_ACTION_T pullAction(BYTE q);
extern COMPUTE_ACTION_T peekActionQueue(BYTE q, unsigned char index);
extern void deleteActionQueue(BYTE q, unsigned char index);
extern void nextQueue(void);
#endif