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
		System.out.println("NV#"+nvIndex++ +"="+NvOpCode.END.code()+"\t\t//END");
		return null;
	}

	@Override
	public Object visit(ASTDefine node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTRule node, Object data) {
		if (node.jjtGetNumChildren() > 0) {
			System.out.println("NV#"+nvIndex++ +"="+NvOpCode.RULE.code()+"\t\t//RULE");
			ASTExpression expression = (ASTExpression) node.jjtGetChild(0);
			ASTTime within = (ASTTime) node.jjtGetChild(1);
			ASTActionList actions = (ASTActionList) node.jjtGetChild(2);
			within.jjtAccept(this, data);
			expression.jjtAccept(this, data);
			actions.jjtAccept(this, data);
			if (node.jjtGetNumChildren() > 3) {	// have a THEN action list
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.THEN.code()+"\t\t//THEN");
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
		System.out.println("NV#"+nvIndex++ +"="+val+"\t\t//Time value");
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
			System.out.println("NV#"+nvIndex++ +"="+NvOpCode.DELAY.code()+"\t\t//DELAY");
			node.childrenAccept(this, data);
		}
		if (node.jjtGetNumChildren() == 2) {	// MessageState and Event
			//node.childrenAccept(this, data);
			Node n = node.jjtGetChild(0);
			ASTMessageState ms = (ASTMessageState)n;
			if (ms.getState() == MessageState.ON) {
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.SEND_ON.code()+"\t\t//SEND_ON");
			} else {
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.SEND_OFF.code()+"\t\t//SEND_OFF");
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
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.OR.code()+"\t\t//OR");
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
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.AND.code()+"\t\t//AND");
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
		node.childrenAccept(this, data);
		int ev = DefinesVisitor.getIndex(node.getName());
		if (ev == 0) {
			System.out.println("Error: undefined variable \""+node.getName()+"\"");
		} else {
			System.out.println("NV#"+nvIndex++ +"="+(ev)+"\t\t//Event number");
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
			if ("<".equals(op)) {
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.LESS.code()+"\t\t//LESS THAN");
			} else if ("<=".equals(op)) {
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.LESSEQUAL.code()+"\t\t//LESS OR EQUALS");
			} else if (">".equals(op)) {
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.MORE.code()+"\t\t//GREATER THAN");
			} else if (">=".equals(op)) {
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.MOREEQUAL.code()+"\t\t//GREATER OR EQUALS");
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
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.PLUS.code()+"\t\t//PLUS");
			} else if ("-".equals(op)){
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.MINUS.code()+"\t\t//MINUS");
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
			System.out.println("NV#"+nvIndex++ +"="+NvOpCode.NOT.code()+"\t\t//NOT");
		}
		node.childrenAccept(this, data);
		return null;
	}


	@Override
	public Object visit(ASTPrimaryBooleanExpression node, Object data) {
		node.childrenAccept(this, data);
		return null;
	}


	@Override
	public Object visit(ASTMessage node, Object data) {
		node.childrenAccept(this, data);
		return null;
	}


	@Override
	public Object visit(ASTPrimaryIntegerExpression node, Object data) {
		if (node.getOpCode() == ASTPrimaryIntegerExpression.OpCodes.COUNT) {
			ASTMessageState ms = (ASTMessageState) node.jjtGetChild(0);
			if (ms.getState() == MessageState.ON) {
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.COUNT_ON.code()+"\t\t//COUNT_OFF");
			} else {
				System.out.println("NV#"+nvIndex++ +"="+NvOpCode.COUNT_OFF.code()+"\t\t//COUNT_OFF");
			}
		} else if (node.getOpCode() == ASTPrimaryIntegerExpression.OpCodes.INTEGER) {
			System.out.println("NV#"+nvIndex++ +"="+NvOpCode.INTEGER.code()+"\t\t//INTEGER");
			System.out.println("NV#"+nvIndex++ +"="+node.getInt()+"\t\t//INT VALUE");
		}
		node.childrenAccept(this, data);
		return null;
	}


}
