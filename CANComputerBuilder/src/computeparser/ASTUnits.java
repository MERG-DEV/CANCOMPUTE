/* Generated By:JJTree: Do not edit this line. ASTUnits.java Version 4.3 */
/* JavaCCOptions:MULTI=true,NODE_USES_PARSER=false,VISITOR=true,TRACK_TOKENS=false,NODE_PREFIX=AST,NODE_EXTENDS=,NODE_FACTORY=,SUPPORT_CLASS_VISIBILITY_PUBLIC=true */
package computeparser;

public
class ASTUnits extends SimpleNode {
  private int units;

public ASTUnits(int id) {
    super(id);
  }

  public ASTUnits(ComputeGrammar p, int id) {
    super(p, id);
  }


  /** Accept the visitor. **/
  public Object jjtAccept(ComputeGrammarVisitor visitor, Object data) {
    return visitor.visit(this, data);
  }

public void setUnits(int i) {
	units = i;
}
public int getUnits() {
	return units;
}
}
/* JavaCC - OriginalChecksum=0bd07520a73d44d4a1b64a7630d80479 (do not edit this line) */
