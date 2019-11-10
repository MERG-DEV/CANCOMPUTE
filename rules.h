#ifndef RULES_H
#define RULES_H

#include "GenericTypeDefs.h"

#define NUM_EXPRESSIONS 100
#define NUM_RULES       50

typedef enum {
    NOP         =200,   // C8 No action Operation
    SEQUENCE    =(222), // DE followed by one NV giving number of other (event) NVs)
    
	RECEIVED	=(224),	// E0 followed by one NV giving the event index
    STATE_ON	=(225),	// E1 followed by one NV giving the event index
	STATE_OFF	=(226),	// E2 followed by one NV giving the event index
	
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
	
	COUNT       =250,	// FA followed by one NV giving the event index
	DELAY		=251,	// FB followed by one NV giving the time period in 0.1sec units
	
	SEND    	=253,	// FD followed by one NV giving the event index
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
    BYTE within;        // number of seconds
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
    INVALID_EVENT           =7
} RuleState;

extern BYTE ruleIndex;
extern BYTE expressionIndex;
extern RuleState ruleState;
extern BYTE nvPtr;

extern void runRules(void);
extern void ruleInit(void);

// Sizes: 
// Rules        4*NUM_RULES = 200 = 0x96
// Expressions  3*NUM_EXPRESSIONS = 300 = 0x12c
// Operands     3*NUM_OPERANDS = 600 = 0x258
#define AT_RULES            ((BYTE*)(AT_EVENTS - 200))
#define AT_EXPRESSIONS      ((BYTE*)(AT_RULES - 300))

#define AT_RULEINDEX        ((BYTE*)(AT_EXPRESSIONS - 1))
#define AT_EXPRESSIONINDEX  ((BYTE*)(AT_RULEINDEX -1))
#define AT_RULESTATE        ((BYTE*)(AT_EXPRESSIONINDEX -1))
#define AT_NVPTR            ((BYTE*)(AT_RULESTATE - 1))

#endif