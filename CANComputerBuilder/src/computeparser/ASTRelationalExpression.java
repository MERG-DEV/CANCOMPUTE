/* Generated By:JJTree: Do not edit this line. ASTRelationalExpression.java Version 4.3 */
/* JavaCCOptions:MULTI=true,NODE_USES_PARSER=false,VISITOR=true,TRACK_TOKENS=false,NODE_PREFIX=AST,NODE_EXTENDS=,NODE_FACTORY=,SUPPORT_CLASS_VISIBILITY_PUBLIC=true */
package computeparser;

public
class ASTRelationalExpression extends SimpleNode {
  private String opCode;

public ASTRelationalExpression(int id) {
    super(id);
  }

  public ASTRelationalExpression(ComputeGrammar p, int id) {
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
/* JavaCC - OriginalChecksum=4a976a0e0c82d4d862dde3193e8ef5d0 (do not edit this line) */
