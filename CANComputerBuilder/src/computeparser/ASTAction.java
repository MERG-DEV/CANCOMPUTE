/* Generated By:JJTree: Do not edit this line. ASTAction.java Version 4.3 */
/* JavaCCOptions:MULTI=true,NODE_USES_PARSER=false,VISITOR=true,TRACK_TOKENS=false,NODE_PREFIX=AST,NODE_EXTENDS=,NODE_FACTORY=,SUPPORT_CLASS_VISIBILITY_PUBLIC=true */
package computeparser;

public
class ASTAction extends SimpleNode {
  public enum OpCodes {
		SEND,
		DELAY,
		CBUS
  };
  
  private OpCodes opCode;
  private String cbusMessage;
  
  public String getCbusMessage() {
	return cbusMessage;
  }

  public void setCbusMessage(String cbusMessage) {
	this.cbusMessage = cbusMessage;
  }

  public ASTAction(int id) {
    super(id);
  }

  public ASTAction(ComputeGrammar p, int id) {
    super(p, id);
  }
  
  public void setAction(OpCodes op) {
	  opCode = op;
  }
  public OpCodes getAction() {
	  return opCode;
  }


  /** Accept the visitor. **/
  public Object jjtAccept(ComputeGrammarVisitor visitor, Object data) {
    return visitor.visit(this, data);
  }
}
/* JavaCC - OriginalChecksum=61ce30544d303d66b436ea45160db132 (do not edit this line) */
