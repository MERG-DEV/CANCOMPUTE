/* Generated By:JJTree: Do not edit this line. ASTAdditiveExpression.java Version 4.3 */
/* JavaCCOptions:MULTI=true,NODE_USES_PARSER=false,VISITOR=true,TRACK_TOKENS=false,NODE_PREFIX=AST,NODE_EXTENDS=,NODE_FACTORY=,SUPPORT_CLASS_VISIBILITY_PUBLIC=true */
package computeparser;

public
class ASTAdditiveExpression extends SimpleNode {
  private String opCode;

public ASTAdditiveExpression(int id) {
    super(id);
  }

  public ASTAdditiveExpression(ComputeGrammar p, int id) {
    super(p, id);
  }


  /** Accept the visitor. **/
  public Object jjtAccept(ComputeGrammarVisitor visitor, Object data) {
    return visitor.visit(this, data);
  }

public void setOpCode(String image) {
	opCode = image;
	
}

public String getOpCode() {
	return opCode;
}
}
/* JavaCC - OriginalChecksum=071ba07a52602b101aacfc435d48adbc (do not edit this line) */