#include "rules.h"
#include "computeEvents.h"
#include "events.h"
#include "romops.h"
#include "actionQueue.h"

// forward declarations
BYTE newRule(void);
BYTE newExpression(void);
BYTE getNv(BYTE NVindex);
void loadExpression(BYTE expression);
void skipActions(void);

rom near Rule * rules = (rom near Rule*)AT_RULES;
rom near Expression * expressions = (rom near Expression*)AT_EXPRESSIONS;

BYTE ruleIndex;
BYTE expressionIndex;
RuleState ruleState;
BYTE nvPtr;
BYTE timeLimit;
static BOOL results[NUM_RULES];

rom BYTE romRuleIndex;
rom BYTE romExpressionIndex;
rom RuleState romRuleState;
rom BYTE romNvPtr;

#define TOO_MANY        0xFF

// Forward declarations
BYTE execute(BYTE e);
void doActions(BYTE nvi);

void ruleInit(void) {
    BYTE i;
    for (i=0; i<NUM_RULES; i++) {
        results[i] = FALSE;
    }
    ruleIndex = readFlashBlock(AT_RULEINDEX);
    expressionIndex = readFlashBlock(AT_EXPRESSIONINDEX);
    ruleState = readFlashBlock(AT_RULESTATE);
    nvPtr = readFlashBlock(AT_NVPTR);
}

void runRules(void) {
    BYTE rule;
    BYTE result;
    BYTE b;
    for (rule=0; rule<NUM_RULES; rule++) {
        if (rule > ruleIndex) break;
        b = readFlashBlock(&(rules[rule].expression));
        result = execute(b);
        if (results[rule] != result) {
            results[rule] = result;
            timeLimit = readFlashBlock(&(rules[rule].within));
            if (result) {
                b = readFlashBlock(&(rules[rule].actions));
                doActions(b);
            } else {
                b = readFlashBlock(&(rules[rule].thens));
                doActions(b);
            }
        }
    }
}

void doActions(BYTE nvi) {
    ACTION_T action;
    while (1) {
        action.op = getNv(nvi++);
        action.arg = getNv(nvi++);
        
        switch(action.op) {
            case DELAY:
            case SEND_ON:
            case SEND_OFF:
                pushAction(action);
                break;
            default: return;
        }
    }
    nextQueue();
}

void load(void) {
	ruleState = VALID;
	ruleIndex = 0;
	expressionIndex = 0;
	nvPtr = 1;
		
	while (1) {
        BYTE i;
		BYTE nv = getNv(nvPtr++);
        BYTE r;
		
		if ((OpCodes)nv == END) {
			break;
		}
		if ((OpCodes)nv != RULE) {
			ruleState = UNKNOWN_INSTRUCTION;
			break;
		}
		ruleIndex = newRule();
		if (r < 0) {ruleState = TOO_MANY_RULES; return;}
        ruleIndex = r;
		writeFlashByte((BYTE*)(&(rules[r].within)), getNv(nvPtr++));
    	writeFlashByte((BYTE*)(&(rules[r].expression)), newExpression());
        loadExpression(readFlashBlock(&rules[r].expression));
        writeFlashByte((BYTE*)(&(rules[r].actions)), nvPtr);   // point to the actions in the NVs
		skipActions();
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

void loadExpression(BYTE expression) {
		BYTE nv = getNv(nvPtr++);
        BYTE val;
        BYTE op_index, expr_index;
        
        if (expression >= NUM_EXPRESSIONS) return;
		
		if (nv < MIN_RULE_OPCODE) {
			ruleState = UNKNOWN_INSTRUCTION;
			return;
		}
		
		writeFlashByte((BYTE*)(&(expressions[expression].opCode)), nv);
		
        switch(nv) {
            case COUNT_ON:
            case COUNT_OFF:
                val = getNv(nvPtr++);
                writeFlashByte((BYTE*)(&(expressions[expression].op1.eventNo)), val);
                if (val > (NUM_EVENTS -1)) {ruleState = INVALID_EVENT; return;}
                break;
            case INTEGER:
                val = getNv(nvPtr++);
                writeFlashByte((BYTE*)(&(expressions[expression].op1.integer)), val);
                break;
            case NOT:
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

void skipActions(void) {
    BYTE ai = 0;
        
	ai = getNv(nvPtr++);
    switch (ai) {
        case SEND_ON:
        case SEND_OFF:
        case DELAY:
            getNv(nvPtr++);
            break;
        default:
            break;
	}
}

/*
 * @param e is an index into the expressions array.
 * The expression has an opCode which determines how to interpret the two operands op1 and op2
 */
BYTE execute(BYTE e) {
    BYTE opCode;
    BYTE op1, op2;
    opCode = readFlashBlock(&(expressions[e].opCode));
    op1 = readFlashBlock(&(expressions[e].op1));
    if (op1 >= NUM_EVENTS) return 0;
    
    switch(opCode) {
        case COUNT_ON:
            if (op1 >= NUM_EVENTS) return 0;
            return received(op1, TRUE);
        case COUNT_OFF:
            if (op1 >= NUM_EVENTS) return 0;
            return received(op1, FALSE);
        case INTEGER:
            return op1;
        case NOT:
            return !execute(op1);
    }
    
    op2 = readFlashBlock(&(expressions[e].op2));
    
    switch (opCode) {
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

BYTE newRule(void) {
    if (ruleIndex+1 >= NUM_RULES) return TOO_MANY;
	return ruleIndex++;
}
BYTE newExpression(void) {
	if (expressionIndex+1 >= NUM_EXPRESSIONS) return TOO_MANY;
	return expressionIndex++;
}


