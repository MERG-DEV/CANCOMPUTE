package computeparser.visitor;

import computeparser.ASTAction;
import computeparser.ASTActionList;
import computeparser.ASTAdditiveExpression;
import computeparser.ASTAndExpression;
import computeparser.ASTDefine;
import computeparser.ASTDefineList;
import computeparser.ASTEventLiteral;
import computeparser.ASTExpression;
import computeparser.ASTIdentifier;
import computeparser.ASTMessage;
import computeparser.ASTMessageState;
import computeparser.ASTOrExpression;
import computeparser.ASTPrimaryBooleanExpression;
import computeparser.ASTPrimaryBooleanExpression.OpCodes;
import computeparser.ASTPrimaryIntegerExpression;
import computeparser.ASTRelationalExpression;
import computeparser.ASTRule;
import computeparser.ASTRuleList;
import computeparser.ASTSimpleNode;
import computeparser.ASTTime;
import computeparser.ASTUnaryBooleanExpression;
import computeparser.ASTUnits;
import computeparser.ComputeGrammarVisitor;
import computeparser.MessageState;
import computeparser.Node;
import computeparser.SimpleNode;

public class NvVisitor implements ComputeGrammarVisitor {
	private int nvIndex = 1;

	@Override
	public Object visit(SimpleNode node, Object data) {
		node.childrenAccept(this, data);
		return null;
	}
	

	@Override
	public Object visit(ASTSimpleNode node, Object data) {
		node.childrenAccept(this, data);
		return null;
	}

	@Override
	public Object visit(ASTDefineList node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTRuleList node, Object data) {
		node.childrenAccept(this, data);
		doNv(nvIndex++, NvOpCode.END);
		return null;
	}

	@Override
	public Object visit(ASTDefine node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTRule node, Object data) {
		if (node.jjtGetNumChildren() > 0) {
			doNv(nvIndex++, NvOpCode.RULE);
			ASTExpression expression = (ASTExpression) node.jjtGetChild(0);
			ASTTime within = (ASTTime) node.jjtGetChild(1);
			ASTActionList actions = (ASTActionList) node.jjtGetChild(2);
			within.jjtAccept(this, data);
			expression.jjtAccept(this, data);
			actions.jjtAccept(this, data);
			if (node.jjtGetNumChildren() > 3) {	// have a THEN action list
				doNv(nvIndex++, NvOpCode.THEN);
				actions = (ASTActionList) node.jjtGetChild(3);
				actions.jjtAccept(this, data);
			}
		}
		return null;
	}

	@Override
	public Object visit(ASTTime node, Object data) {
		// Convert to 1/10 of seconds
		Node n = node.jjtGetChild(0);
		ASTUnits u = (ASTUnits)n;
		int val = node.getTime();
		if (u.getUnits() == 1) {
			val *= 10;
		}
		if (u.getUnits() == 1000) {
			val /= 100;
		}
		doNv(nvIndex++, val, "Time Value");
		return null;
	}

	@Override
	public Object visit(ASTActionList node, Object data) {
		node.childrenAccept(this, data);
		return null;
	}

	@Override
	public Object visit(ASTAction node, Object data) {
		if (node.jjtGetNumChildren() == 1) {	// just Time()
			doNv(nvIndex++, NvOpCode.DELAY);
			node.childrenAccept(this, data);
		}
		if (node.jjtGetNumChildren() == 2) {	// MessageState and Event
			//node.childrenAccept(this, data);
			Node n = node.jjtGetChild(0);
			ASTMessageState ms = (ASTMessageState)n;
			if (ms.getState() == MessageState.ON) {
				doNv(nvIndex++, NvOpCode.SEND_ON);
			} else {
				doNv(nvIndex++, NvOpCode.SEND_OFF);
			}
			n = node.jjtGetChild(1);
			n.jjtAccept(this, data);
			
		}
		//node.childrenAccept(this, data);
		return null;
	}

	@Override
	public Object visit(ASTExpression node, Object data) {
		node.childrenAccept(this, data);
		return null;
	}

	@Override
	public Object visit(ASTOrExpression node, Object data) {
		int depth = 0;
		while (true) {
			Node n;
			if (node.jjtGetNumChildren() >  depth+1) {
				doNv(nvIndex++, NvOpCode.OR);
				n = node.jjtGetChild(depth);
				n.jjtAccept(this, data);
				depth++;
			} else {
				n = node.jjtGetChild(depth);
				n.jjtAccept(this, data);
				depth++;
				break;
			}
		}
		return null;
	}

	@Override
	public Object visit(ASTAndExpression node, Object data) {
		int depth = 0;
		while (true) {
			Node n;
			if (node.jjtGetNumChildren() >  depth+1) {
				doNv(nvIndex++, NvOpCode.AND);
				n = node.jjtGetChild(depth);
				n.jjtAccept(this, data);
				depth++;
			} else {
				n = node.jjtGetChild(depth);
				n.jjtAccept(this, data);
				depth++;
				break;
			}
		}
		return null;
	}

	@Override
	public Object visit(ASTIdentifier node, Object data) {
		//System.out.println("identifier");
		node.childrenAccept(this, data);
		int ev = DefinesVisitor.getIndex(node.getName());
		if (ev == 0) {
			System.out.println("Error: undefined variable \""+node.getName()+"\"");
		} else {
			doNv(nvIndex++, ev, "Event Number");
		}
		return null;
	}

	@Override
	public Object visit(ASTEventLiteral node, Object data) {
		node.childrenAccept(this, data);
		return null;
	}

	@Override
	public Object visit(ASTMessageState node, Object data) {
		node.childrenAccept(this, data);
		return null;
	}

	@Override
	public Object visit(ASTRelationalExpression node, Object data) {
		if (node.jjtGetNumChildren() >  1) {
			String op = node.getOpCode();
			if ("is".equals(op)) {
				doNv(nvIndex++, NvOpCode.EQUALS);
			} else if ("equals".equals(op)) {
				doNv(nvIndex++, NvOpCode.EQUALS);
			} else if ("=".equals(op)) {
				doNv(nvIndex++, NvOpCode.EQUALS);
			} else if ("!=".equals(op)) {
				doNv(nvIndex++, NvOpCode.NOTEQUALS);
			} else if ("<".equals(op)) {
				doNv(nvIndex++, NvOpCode.LESS);
			} else if ("<=".equals(op)) {
				doNv(nvIndex++, NvOpCode.LESSEQUAL);
			} else if (">".equals(op)) {
				doNv(nvIndex++, NvOpCode.MORE);
			} else if (">=".equals(op)) {
				doNv(nvIndex++, NvOpCode.MOREEQUAL);
			} else {
				System.out.println("Error unknown RelationExpression operator"+op);
			}
		}
		node.childrenAccept(this, data);
		return null;
	}


	@Override
	public Object visit(ASTAdditiveExpression node, Object data) {
		if (node.jjtGetNumChildren() >  1) {
			String op = node.getOpCode();
			if ("+".equals(op)) {
				doNv(nvIndex++, NvOpCode.PLUS);
			} else if ("-".equals(op)){
				doNv(nvIndex++, NvOpCode.MINUS);
			} else {
				System.out.println("Error unknown AdditiveExpression operator"+op);
			}
		}
		node.childrenAccept(this, data);
		return null;
	}

	@Override
	public Object visit(ASTUnits node, Object data) {
		node.childrenAccept(this, data);
		return null;
	}

	@Override
	public Object visit(ASTUnaryBooleanExpression node, Object data) {
		if (node.getOp()) {
			doNv(nvIndex++, NvOpCode.NOT);
		}
		node.childrenAccept(this, data);
		return null;
	}


	@Override
	public Object visit(ASTPrimaryBooleanExpression node, Object data) {
		if (node.getOpCode() == OpCodes.STATE) {
			ASTMessageState ms = (ASTMessageState) node.jjtGetChild(1);
			if (ms.getState() == MessageState.ON) {
				doNv(nvIndex++, NvOpCode.STATE_ON);
			} else {
				doNv(nvIndex++, NvOpCode.STATE_OFF);
			}
			node.jjtGetChild(0).jjtAccept(this, null);
			return null;
		}
		if (node.getOpCode() == OpCodes.RECEIVED) {
			ASTMessageState ms = (ASTMessageState) node.jjtGetChild(0);
			if (ms.getState() == MessageState.ON) {
				doNv(nvIndex++, NvOpCode.RECEIVED_ON);
			} else {
				doNv(nvIndex++, NvOpCode.RECEIVED_OFF);
			}
		} 
		if (node.getOpCode() == OpCodes.BEFORE) {
			ASTMessage ml = (ASTMessage) node.jjtGetChild(0);
			ASTMessage mr = (ASTMessage) node.jjtGetChild(1);
			if(((ASTMessageState)(ml.jjtGetChild(0))).getState()==MessageState.ON) {
				if(((ASTMessageState)(mr.jjtGetChild(0))).getState()==MessageState.ON) {
					doNv(nvIndex++, NvOpCode.BEFORE_ON_ON);
				} else {
					doNv(nvIndex++, NvOpCode.BEFORE_ON_OFF);
				}
			} else {
				if(((ASTMessageState)(mr.jjtGetChild(0))).getState()==MessageState.ON) {
					doNv(nvIndex++, NvOpCode.BEFORE_OFF_ON);
				} else {
					doNv(nvIndex++, NvOpCode.BEFORE_OFF_OFF);
				}
			}
			ml.jjtAccept(this, null);
			mr.jjtAccept(this, null);
			return null;
		}
		if (node.getOpCode() == OpCodes.AFTER) {
			ASTMessage ml = (ASTMessage) node.jjtGetChild(0);
			ASTMessage mr = (ASTMessage) node.jjtGetChild(1);
			if(((ASTMessageState)(ml.jjtGetChild(0))).getState()==MessageState.ON) {
				if(((ASTMessageState)(mr.jjtGetChild(0))).getState()==MessageState.ON) {
					doNv(nvIndex++, NvOpCode.AFTER_ON_ON);
				} else {
					doNv(nvIndex++, NvOpCode.AFTER_ON_OFF);
				}
			} else {
				if(((ASTMessageState)(mr.jjtGetChild(0))).getState()==MessageState.ON) {
					doNv(nvIndex++, NvOpCode.AFTER_OFF_ON);
				} else {
					doNv(nvIndex++, NvOpCode.AFTER_OFF_OFF);
				}
			}	
			ml.jjtAccept(this, null);
			mr.jjtAccept(this, null);
			return null;
		}
		node.childrenAccept(this, data);
		return null;
	}


	@Override
	public Object visit(ASTMessage node, Object data) {
		//System.out.println("message");
		node.childrenAccept(this, data);
		return null;
	}


	@Override
	public Object visit(ASTPrimaryIntegerExpression node, Object data) {
		if (node.getOpCode() == ASTPrimaryIntegerExpression.OpCodes.COUNT) {
			ASTMessageState ms = (ASTMessageState) node.jjtGetChild(0);
			if (ms.getState() == MessageState.ON) {
				doNv(nvIndex++, NvOpCode.COUNT_ON);
			} else {
				doNv(nvIndex++, NvOpCode.COUNT_OFF);
			}
		} else if (node.getOpCode() == ASTPrimaryIntegerExpression.OpCodes.INTEGER) {
			doNv(nvIndex++, NvOpCode.INTEGER);
			doNv(nvIndex++, node.getInt(), "Integer value");
		}
		node.childrenAccept(this, data);
		return null;
	}

	private String hex(int i) {
		int i1 = (i>>4)&0xF;
		int i2 = i&0xF;
		return ""+(char)(i1>9?i1-10+'A':i1+'0')+(char)(i2>9?i2-10+'A':i2+'0');
	}
	private String dec(int i) {
		String ret=""+i;
		if (i < 10) ret += " ";
		if (i < 100) ret += " ";
		return ret;
	}
	
	private void doNv(int nvi, NvOpCode op) {
		doNv(nvi, op.code(), ""+op);
	}
	
	private void doNv(int nvi, int val, String desc) {
		System.out.println("NV#"+dec(nvi)+" (0x"+hex(nvi)+")="+dec(val)+" (0x"+hex(val)+")\t//"+desc);
	}

}
