/* Generated By:JJTree: Do not edit this line. ASTRule.java Version 4.3 */
/* JavaCCOptions:MULTI=true,NODE_USES_PARSER=false,VISITOR=true,TRACK_TOKENS=false,NODE_PREFIX=AST,NODE_EXTENDS=,NODE_FACTORY=,SUPPORT_CLASS_VISIBILITY_PUBLIC=true */
package computeparser;

public
class ASTRule extends SimpleNode {
  public ASTRule(int id) {
    super(id);
  }

  public ASTRule(ComputeGrammar p, int id) {
    super(p, id);
  }


  /** Accept the visitor. **/
  public Object jjtAccept(ComputeGrammarVisitor visitor, Object data) {
    return visitor.visit(this, data);
  }
}
/* JavaCC - OriginalChecksum=7c9607c591100956b4b3353786be2e41 (do not edit this line) */
