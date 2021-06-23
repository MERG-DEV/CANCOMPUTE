#include "rules.h"
#include "computeEvents.h"
#include "events.h"
#include "romops.h"
#include "actionQueue.h"
#include "computeActions.h"
#include "hwsettings.h"

// forward declarations
BYTE newRule(void);
BYTE newExpression(void);
BYTE getNv(BYTE NVindex);
void loadExpression(BYTE expression);
void skipActions(void);

rom near Rule * rules;
rom near Expression * expressions;

BYTE ruleIndex;         // tracks the number of Rules structures allocated
BYTE expressionIndex;   // tracks the number of Expression structures allocated
RuleState ruleState;    // keeps track if there are any errors whilst parsing rules
BYTE nvPtr;             // index into NVs whilst parsing the rules
BYTE timeLimit;
static BOOL results[NUM_RULES]; // remeber the previous result of the expression
                                // so we can see then it changes
#define TOO_MANY        0xFF

// Forward declarations
BYTE execute(BYTE e);
void doActions(BYTE nvi);

/**
 * Reset back to no rules, free up any allocated structures and clear rule results.
 */
void ruleInit(void) {
    BYTE i;
    rules = (rom near Rule*)AT_RULES;
    expressions = (rom near Expression*)AT_EXPRESSIONS;
    for (i=0; i<NUM_RULES; i++) {
        results[i] = FALSE;
    }
    ruleIndex = readFlashBlock(AT_RULEINDEX);
    expressionIndex = readFlashBlock(AT_EXPRESSIONINDEX);
    ruleState = readFlashBlock(AT_RULESTATE);
    nvPtr = readFlashBlock(AT_NVPTR);
}

/**
 * Run (evaluate) the rules and perform the Actions if necessary.
 * 
 * Will run the Action set if the rule evaluation changes from false to true 
 * and runs the THEN set of Actions if the evaluation result changes from true
 * to false.
 * 
 * Handles the THEN clause.
 * It isn't obvious exactly what this is for so a bit of explanation...
 * 
 * The actions after the <time> parameter are run when the expression transitions from false to true.
 * The actions after the <then> parameter are run when the expressions transitions from true to false.
 * 
 */
void runRules(void) {
    BYTE rule;
    BYTE result;
    BYTE b;
    LED_BLUE = LED_ON;
    for (rule=0; rule<ruleIndex; rule++) {
        b = readFlashBlock(&(rules[rule].expression));
        timeLimit = readFlashBlock(&(rules[rule].within));
        result = execute(b);
        if (results[rule] != result) {
            results[rule] = result;        
            if (result) {
                b = readFlashBlock(&(rules[rule].actions));
                doActions(b);
            } else {
                b = readFlashBlock(&(rules[rule].thens));
                doActions(b);
            }
        }
    }
    LED_BLUE = LED_OFF;
}

/**
 * Execute the set of Actions pointed to by nvi.
 * @param nvi the index into the NVs which is the start of the Actions
 */
void doActions(BYTE nvi) {
    COMPUTE_ACTION_T action;
    BYTE op;
    BYTE evt;
    while (1) {
        op = getNv(nvi++);

        evt = getNv(nvi++);
        
        switch(op) {
            case DELAY:
                action.op = ACTION_OPCODE_DELAY;
                action.arg = evt;
                break;
            case SEND:
                if (EVENT_STATE(evt)) {
                    action.op = ACTION_OPCODE_SEND_ON;
                } else {
                    action.op = ACTION_OPCODE_SEND_OFF;
                }
                action.arg = EVENT_NO(evt);
                break;
            default: 
                nextQueue();
                return;
        }
        pushAction(action);
    }
}

/**
 * Loads the rules.
 * Goes through the NVs parsing them and checking they are valid rules. The
 * rules are transferred to Flash data structures of Rule type.
 */
void load(void) {
	ruleState = VALID;
	ruleIndex = 0;
	expressionIndex = 0;
	nvPtr = 1;
		
	while (ruleState == VALID) {
        BYTE i;
		BYTE nv = getNv(nvPtr++);
        BYTE r;
        BYTE e;
		
		if ((OpCodes)nv >= END) {
			break;
		}
		if ((OpCodes)nv != RULE) {
			ruleState = UNKNOWN_INSTRUCTION;
			break;
		}
		r = newRule();
		if (r < 0) {ruleState = TOO_MANY_RULES; break;}
		writeFlashByte((BYTE*)(&(rules[r].within)), getNv(nvPtr++));    // the time limit
    	writeFlashByte((BYTE*)(&(rules[r].expression)), newExpression());   // allocate an Expression structure
        e = newExpression();
        if (e == TOO_MANY) {
            ruleIndex--;
            ruleState=TOO_MANY_EXPRESSIONS;
            break;
        }
        writeFlashByte((BYTE*)(&(rules[r].expression)), e);   // allocate an Expression structure
        loadExpression(e);                                    // Load the expression from NVs
        if (ruleState != VALID) break;
        writeFlashByte((BYTE*)(&(rules[r].actions)), nvPtr);   // point to the actions in the NVs
		skipActions();
        if (ruleState != VALID) break;
        nv = getNv(nvPtr);
        if (nv == THEN) {
            nvPtr++;
            writeFlashByte((BYTE*)(&(rules[r].thens)), nvPtr);   // point to the actions in the NVs
            skipActions();
        } else {
            writeFlashByte((BYTE*)(&(rules[r].thens)), 0);   // no then actions
        }
	}
    // save the indexes etc.
    writeFlashByte(AT_RULEINDEX, ruleIndex);
    writeFlashByte(AT_EXPRESSIONINDEX, expressionIndex);
    writeFlashByte(AT_RULESTATE, ruleState);
    writeFlashByte(AT_NVPTR, nvPtr);
}

/**
 * Loads an expression from the NV list into an Expression structure held in Flash.
 * 
 * THIS IS A RECURSIVE FUNCTION.
 * 
 * @param expression the index into the Expression table into which the expression 
 * indexed by nvPtr is loaded
 */
void loadExpression(BYTE expression) {
	BYTE nv = getNv(nvPtr++);
    BYTE val;
    BYTE num;
    BYTE op_index, expr_index;
        
    if (expression >= NUM_EXPRESSIONS) return;
		
	if (nv < MIN_RULE_OPCODE) {
		ruleState = UNKNOWN_INSTRUCTION;
		return;
	}
		
    // save the opcode in the expression
	writeFlashByte((BYTE*)(&(expressions[expression].opCode)), nv);
		
    // Now save the operands - 1 or 2 depending upon the opcode
    // The operands may be integers or events or expressions
    switch(nv) {
        case BEFORE:
        case AFTER:
            // Two events with on/off state
            val = getNv(nvPtr++);
            writeFlashByte((BYTE*)(&(expressions[expression].op1.eventNo)), val);
            if (EVENT_NO(val) > (NUM_EVENTS -1)) {ruleState = INVALID_EVENT; return;}
                
            val = getNv(nvPtr++);
            writeFlashByte((BYTE*)(&(expressions[expression].op2.eventNo)), val);
            if (EVENT_NO(val) > (NUM_EVENTS -1)) {ruleState = INVALID_EVENT; return;}
            break;
        case SEQUENCE:
            // length and offset of events with on/off state
            num = getNv(nvPtr++);   // number loaded from NV
            writeFlashByte((BYTE*)(&(expressions[expression].op1.integer)), num);
               
            val = nvPtr;    // Offset from the current pointer where the events are
            writeFlashByte((BYTE*)(&(expressions[expression].op2.integer)), val);       // save nvPtr in op2
                
            nvPtr += num;   // skip over the events
            break;
        case RECEIVED:
        case COUNT:
            // One event with on/off state
            val = getNv(nvPtr++);
            writeFlashByte((BYTE*)(&(expressions[expression].op1.eventNo)), val);
            if (val == 0) {ruleState = INVALID_EVENT; return;}
            if (EVENT_NO(val) > (NUM_EVENTS)) {ruleState = INVALID_EVENT; return;}
            break;
        case STATE_ON:
        case STATE_OFF:
            // One event WITHOUT on/off state
            val = getNv(nvPtr++);
            writeFlashByte((BYTE*)(&(expressions[expression].op1.eventNo)), val);
            if (val == 0) {ruleState = INVALID_EVENT; return;}
            if (val > (NUM_EVENTS)) {ruleState = INVALID_EVENT; return;}
            break;
        case INTEGER:
            // One integer
            val = getNv(nvPtr++);
            writeFlashByte((BYTE*)(&(expressions[expression].op1.integer)), val);
            break;
        case NOT:
            // One expression
            expr_index = newExpression();
            if (expr_index == TOO_MANY) {ruleState = TOO_MANY_EXPRESSIONS; return;}
            writeFlashByte((BYTE*)(&(expressions[expression].op1.expression)), expr_index);
            loadExpression(expr_index);
            if (ruleState != VALID) return;
            break;
        case AND: 
        case OR:
        case MORE:
		case MOREEQUAL:
		case LESS:
		case LESSEQUAL:
		case EQUALS:
		case NOTEQUALS:
		case PLUS:
		case MINUS:
            // Two expressions
            expr_index = newExpression();
            if (expr_index == TOO_MANY) {ruleState = TOO_MANY_EXPRESSIONS; return;}
			writeFlashByte((BYTE*)(&(expressions[expression].op1.expression)), expr_index);
            loadExpression(expr_index);
            if (ruleState != VALID) return;
               
            expr_index = newExpression();
            if (expr_index == TOO_MANY) {ruleState = TOO_MANY_EXPRESSIONS; return;}
            writeFlashByte((BYTE*)(&(expressions[expression].op2.expression)), expr_index);
            loadExpression(expr_index);
            if (ruleState != VALID) return;
            break;     
        default:
            ruleState = UNKNOWN_INSTRUCTION;
            break;
	}
}

/**
 * Skip forward looking for the next non-Action.
 * Actions are not transferred but left in the NV space. The down-side of this 
 * is that NVs may be updated which could corrupt the Actions. Normally the rules
 * would be reparsed so I'm not going to worry about this. 
 */
void skipActions(void) {
    BYTE aop = 0;
    BYTE evt;
    
    while (1) {
        // get the OPCODE
        aop = getNv(nvPtr++);
        
        switch (aop) {
            case SEND:
                // get the argument
                evt = getNv(nvPtr++);
                if (evt & 0x40) { // evt needs to have bit 6 clear so it will fit into ActionQueue
                    ruleState = ARGUMENT_TOO_LARGE; 
                    return;
                }
                break;
            case DELAY:
                // get the argument
                evt = getNv(nvPtr++);
                if (evt > 0x3F) { // evt needs to be only 6 bits so it will fit into ActionQueue
                    ruleState = ARGUMENT_TOO_LARGE; 
                    return;
                }
                break;
            default:
                nvPtr--;    // move back so that we leave pointer at next instruction
                return;
        }
	}
}

/**
 * Every 100ms the rules are evaluated.
 * This evaluates a rule's expression. The expression has an opCode which determines how to interpret the two operands op1 and op2.
 * 
 * THIS IS A RECURSIVE FUNCTION.
 * 
 * @param e is an index into the expressions array
 * @return the result of the evaluation
 */
BYTE execute(BYTE e) {
    BYTE opCode;
    BYTE op1, op2;
    opCode = readFlashBlock(&(expressions[e].opCode));
    op1 = readFlashBlock(&(expressions[e].op1));
    op2 = readFlashBlock(&(expressions[e].op2));
    
    switch(opCode) {
        case BEFORE:// return true if event op1 is before op2 and both are present
            if (EVENT_NO(op1) >= NUM_EVENTS) return 0;
            if (EVENT_NO(op2) >= NUM_EVENTS) return 0;
            return sequence2(op1, op2);
        case AFTER:// return true if event op1 is after op2 and both are present
            if (EVENT_NO(op1) >= NUM_EVENTS) return 0;
            if (EVENT_NO(op2) >= NUM_EVENTS) return 0;
            return sequence2(op2, op1);
        case SEQUENCE:
            return sequenceMulti(op1, op2);
        case STATE_ON:
            if (op1 >= NUM_EVENTS) return 0;
            return (currentEventState[op1] != 0);
        case STATE_OFF:
            if (op1 >= NUM_EVENTS) return 0;
            return (currentEventState[op1] == 0);
        case COUNT:
            if (EVENT_NO(op1) >= NUM_EVENTS) return 0;
            return countEvent(op1);
        case RECEIVED:
            if (EVENT_NO(op1) >= NUM_EVENTS) return 0;
            return receivedEvent(op1);
        case INTEGER:
            return op1;
        case NOT:
            return !execute(op1);
    }
    
    op2 = readFlashBlock(&(expressions[e].op2));    // this seems to be a duplicate
    
    switch (opCode) {                               // Why is the switch split?
        case AND: 
            return execute(op1) && execute(op2);
		case OR:
            return execute(op1) || execute(op2);
        case MORE:
            return execute(op1) > execute(op2);
		case MOREEQUAL:
            return execute(op1) >= execute(op2);
		case LESS:
            return execute(op1) < execute(op2);
		case LESSEQUAL:
            return execute(op1) <= execute(op2);
		case EQUALS:
            return execute(op1) == execute(op2);
		case NOTEQUALS:
            return execute(op1) != execute(op2);
		case PLUS:
            return execute(op1) + execute(op2);
		case MINUS:
            return execute(op1) - execute(op2);
        default:
            ruleState = UNKNOWN_INSTRUCTION;
            return 0;
    }
}

/**
 * Allocate a new Rule from the Rule array.
 * 
 * @return index into the Rule array 
 */
BYTE newRule(void) {
    if (ruleIndex >= NUM_RULES) return TOO_MANY;
	return ruleIndex++;
}

/** Allocate a new Expression from the Expression array.
 *
 * @return index into the Expression array
 */
BYTE newExpression(void) {
	if (expressionIndex >= NUM_EXPRESSIONS) return TOO_MANY;
	return expressionIndex++;
}


