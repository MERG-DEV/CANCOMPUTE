#ifndef RULES_H
#define RULES_H

#include "GenericTypeDefs.h"

#define NUM_EXPRESSIONS 100
#define NUM_RULES       50

typedef enum {
    NOP         =224,   // No action Operation
    STATE_ON			=(225),	// followed by one NV giving the event index
	STATE_OFF			=(226),	// followed by one NV giving the event index
	BEFORE_OFF_OFF		=(227),	// followed by two NV giving the event indexes
	BEFORE_OFF_ON		=(228),	// followed by two NV giving the event indexes
	BEFORE_ON_OFF		=(229),	// followed by two NV giving the event indexes
	BEFORE_ON_ON		=(230),	// followed by two NV giving the event indexes
	AFTER_OFF_OFF		=(231),	// followed by two NV giving the event indexes
	AFTER_OFF_ON		=(232),	// followed by two NV giving the event indexes
	AFTER_ON_OFF		=(233),	// followed by two NV giving the event indexes
	AFTER_ON_ON			=(234),	// followed by two NV giving the event indexes
	THEN		=235,	// Followed by sequence of actions
	INTEGER		=236,	// followed by one NV of the integer value
	PLUS		=237,	// followed by two integer expressions
	MINUS		=238,	// followed by two integer expressions
	EQUALS		=239,	// followed by two integer expressions
	NOTEQUALS	=240,	// followed by two integer expressions
	LESS		=241,	// followed by two integer expressions
	LESSEQUAL	=242,	// followed by two integer expressions
	MORE		=243,	// followed by two integer expressions
	MOREEQUAL	=244,	// followed by two integer expressions
	RULE		=245,	// followed by one NV giving the time period, one boolean expression and a sequence of actions (DELAY, SEND_ON or SEND_OFF)
	AND			=246,	// followed by two boolean expressions
	NOT         =247,	// followed by two boolean expressions
	OR			=248,	// followed by two boolean expressions
	COUNT_ON	=249,	// followed by one NV giving the event index
	COUNT_OFF	=250,	// followed by one NV giving the event index
	DELAY		=251,	// followed by one NV giving the time period in 0.1sec units
	SEND_ON		=252,	// followed by one NV giving the event index
	SEND_OFF	=253,	// followed by one NV giving the event index
	END			=254
} OpCodes;

#define MIN_RULE_OPCODE STATE_ON

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
    TOO_MANY_OPERANDS       =3,
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