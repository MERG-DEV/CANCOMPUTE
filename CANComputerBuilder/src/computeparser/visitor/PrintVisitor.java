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
import computeparser.SimpleNode;

public class PrintVisitor implements ComputeGrammarVisitor {
	private int indent = 0;
	
	private void indented(String text) {
		for (int i=0; i<indent; i++) System.out.print(" ");
		System.out.println(text);
	}

	@Override
	public Object visit(SimpleNode node, Object data) {
		indented("SimpleNode");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}
	
	@Override
	public Object visit(ASTSimpleNode node, Object data) {
		indented("ASTSimpleNode");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}


	@Override
	public Object visit(ASTDefineList node, Object data) {
		indented("ASTDefineList");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTRuleList node, Object data) {
		indented("ASTRuleList");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTDefine node, Object data) {
		indented("ASTDefine");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTRule node, Object data) {
		indented("ASTRule");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTTime node, Object data) {
		indented("ASTTime");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTActionList node, Object data) {
		indented("ASTActionList");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTAction node, Object data) {
		indented("ASTAction");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTExpression node, Object data) {
		indented("ASTExpression");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTOrExpression node, Object data) {
		if (node.jjtGetNumChildren() > 1) {
			indented("ASTOrExpression");
			indent++;node.childrenAccept(this, data);indent--;
		} else {
			node.childrenAccept(this, data);
		}
		return null;
	}

	@Override
	public Object visit(ASTAndExpression node, Object data) {
		if (node.jjtGetNumChildren() > 1) {
			indented("ASTAndExpression ");
			indent++;node.childrenAccept(this, data);indent--;
		} else {
			node.childrenAccept(this, data);
		}
		return null;
	}

	@Override
	public Object visit(ASTIdentifier node, Object data) {
		indented("ASTIdentifier");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTEventLiteral node, Object data) {
		indented("ASTEventLiteral");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTMessageState node, Object data) {
		indented("ASTMessageState");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTRelationalExpression node, Object data) {
		indented("ASTRelationalExpression "+node.getOpCode());
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTAdditiveExpression node, Object data) {
		indented("ASTAdditiveExpression "+node.getOpCode());
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTUnits node, Object data) {
		indented("ASTUnits "+node.getUnits());
		return null;
	}

	@Override
	public Object visit(ASTUnaryBooleanExpression node, Object data) {
		indented("ASTUnaryBooleanExpression "+node.getOp());
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTPrimaryBooleanExpression node, Object data) {
		indented("ASTPrimaryBooleanExpression "+node.getOpCode());
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTMessage node, Object data) {
		indented("ASTMessage ");
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}

	@Override
	public Object visit(ASTPrimaryIntegerExpression node, Object data) {
		indented("ASTPrimaryIntegerExpression "+node.getOpCode());
		indent++;node.childrenAccept(this, data);indent--;
		return null;
	}
}
