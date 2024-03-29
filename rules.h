#ifndef RULES_H
#define RULES_H

#include "GenericTypeDefs.h"

#define NUM_EXPRESSIONS 200
#define NUM_RULES       50

typedef enum {
    NOP         =200,   // C8 No action Operation
    SEQUENCE    =(222), // DE followed by one NV giving number of other (event) NVs)
    
	RECEIVED	=(224),	// E0 followed by one NV giving the event index
    STATE_ON	=(225),	// E1 followed by one NV giving the event index
	STATE_OFF	=(226),	// E2 followed by one NV giving the event index
    EXPLICIT_STATE_ON = 227,    // E3 followed by one NV giving the event index
    EXPLICIT_STATE_OFF = 228,   // E4 followed by one NV giving the event index
    EXPLICIT_STATE_UNKNOWN = 229,   // E5 followed by one NV giving the event index
	
	BEFORE		=(230),	// E6 followed by two NV giving the event indexes
	
	AFTER		=(234),	// EA followed by two NV giving the event indexes
	THEN		=235,	// EB Followed by sequence of actions
	INTEGER		=236,	// EC followed by one NV of the integer value
	PLUS		=237,	// ED followed by two integer expressions
	MINUS		=238,	// EE followed by two integer expressions
	EQUALS		=239,	// EF followed by two integer expressions
	NOTEQUALS	=240,	// F0 followed by two integer expressions
	LESS		=241,	// F1 followed by two integer expressions
	LESSEQUAL	=242,	// F2 followed by two integer expressions
	MORE		=243,	// F3 followed by two integer expressions
	MOREEQUAL	=244,	// F4 followed by two integer expressions
	RULE		=245,	// F5 followed by one NV giving the time period, one boolean expression and a sequence of actions (DELAY, SEND_ON or SEND_OFF)
	AND			=246,	// F6 followed by two boolean expressions
	NOT         =247,	// F7 followed by two boolean expressions
	OR			=248,	// F8 followed by two boolean expressions
    INPUT       =249,   // F9 followed by index
	COUNT       =250,	// FA followed by one NV giving the event index
    /* Actions */
	DELAY		=251,	// FB followed by one NV giving the time period in 0.1sec units
    CBUS        =252,   // FC followed by a CBUS message
	SEND    	=253,	// FD followed by one NV giving the event index
    /* End triggers the load */
	END			=254    // FE 
} OpCodes;

#define MIN_RULE_OPCODE NOP

#include "module.h"
#include "cancompute.h"
#include "MoreTypes.h"
#include "computeEvents.h"

typedef union {
    BYTE eventNo;
    BYTE expression;
    BYTE integer;
    BYTE index;
} Operand;

typedef struct {
    OpCodes opCode;
    Operand op1;
    Operand op2;
} Expression;

typedef struct {
    BYTE opCode;
    union {
        BYTE event;
        BYTE delay;
    } fields;
} Action;

typedef struct {
    BYTE within;        // number of 100 milli seconds
    BYTE expression;    // The index into the expression list
    BYTE actions;       // index into NVs where the actions start. Each action is 2 NVs
    BYTE thens;         // index into NVs where the then actions start. Each action is 2 NVs
} Rule;

typedef enum  {
    VALID                   =0,
    TOO_MANY_RULES          =1,
    TOO_MANY_EXPRESSIONS    =2,
    //TOO_MANY_OPERANDS       =3,
    TOO_MANY_ACTIONS        =4,
    UNKNOWN_INSTRUCTION     =5,
    UNKNOWN_ACTION          =6,
    INVALID_EVENT           =7,
    ARGUMENT_TOO_LARGE      =8,
    INVALID_INDEX           =9
} RuleState;

extern BYTE ruleIndex;
extern BYTE expressionIndex;
extern RuleState ruleState;
extern BYTE nvPtr;

extern void runRules(void);
extern void ruleInit(void);

/*
 * Flash Memory Map:
 * 
 * 0x7F00 - 0x7FFF NVs             (size=256 0x100)    255 NVs
 * 0x7C44 - 0x7EFF Events          (size=700 0x2BC)    100 events of 1EV
 * 0x7B7C - 0x7C43 Rules           (size=200 0xC8)     50 rules
 * 0x7924 - 0x7B7B Expressions     (size=600 0x258)    200 expressions
 * 0x7923 - 0x7923 ruleIndex       (size=1)
 * 0x7922 - 0x7922 expressionIndex (size=1)
 * 0x7921 - 0x7921 ruleState       (size=1)
 * 0x7920 - 0x7920 nvPtr           (size=1)
 */

#define AT_RULES            (AT_EVENTS - NUM_RULES*sizeof(Rule))
#define AT_EXPRESSIONS      (AT_RULES - NUM_EXPRESSIONS*sizeof(Expression))

#define AT_RULEINDEX        (AT_EXPRESSIONS - 1)
#define AT_EXPRESSIONINDEX  (AT_RULEINDEX -1)
#define AT_RULESTATE        (AT_EXPRESSIONINDEX -1)
#define AT_NVPTR            (AT_RULESTATE - 1)

#endif