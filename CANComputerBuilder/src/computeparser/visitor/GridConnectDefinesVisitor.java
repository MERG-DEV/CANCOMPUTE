package computeparser.visitor;

import computeparser.ASTAction;
import computeparser.ASTActionList;
import computeparser.ASTAdditiveExpression;
import computeparser.ASTAndExpression;
import computeparser.ASTDefine;
import computeparser.ASTDefineList;
import computeparser.ASTEventLiteral;
import computeparser.ASTEventState;
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
import computeparser.ASTSetNN;
import computeparser.ASTSimpleNode;
import computeparser.ASTTime;
import computeparser.ASTUnaryBooleanExpression;
import computeparser.ASTUnits;
import computeparser.ComputeGrammarVisitor;
import computeparser.SimpleNode;

public class GridConnectDefinesVisitor implements ComputeGrammarVisitor {

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
		System.out.println(":S0B20N53"+ASTSetNN.getNN()+";");	// LEARN
		node.childrenAccept(this, data);
		System.out.println(":S0B20N54"+ASTSetNN.getNN()+";");	// UNLEARN
		return null;
	}

	@Override
	public Object visit(ASTRuleList node, Object data) {
		node.childrenAccept(this, data);
		return null;
	}
 
	@Override
	public Object visit(ASTDefine node, Object data) {		
		ASTIdentifier id = (ASTIdentifier)node.jjtGetChild(0);
		ASTEventLiteral ev = (ASTEventLiteral)node.jjtGetChild(1);
		System.out.println(":S0B20ND2"+Util.hexQuad(ev.getEvent().getNN())+Util.hexQuad(ev.getEvent().getEN())+"01"+Util.hexPair(Variables.getIndex())+";");	// EVLRN
		Variables.setIndex(id.getName());
		node.childrenAccept(this, data);
		return null;
	}

	@Override
	public Object visit(ASTRule node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTTime node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTActionList node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTAction node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTExpression node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTOrExpression node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTAndExpression node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTIdentifier node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTEventLiteral node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTMessageState node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTRelationalExpression node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTAdditiveExpression node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTUnits node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTUnaryBooleanExpression node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTPrimaryBooleanExpression node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTMessage node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTPrimaryIntegerExpression node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTSetNN node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTEventState node, Object data) {
		return null;
	}
	
}
