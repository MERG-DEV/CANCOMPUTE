package computeparser.visitor;

import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import computebuilder.Event;
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
import computeparser.EventState;
import computeparser.MessageState;
import computeparser.Node;
import computeparser.SimpleNode;
import computeparser.ASTPrimaryBooleanExpression.OpCodes;

public class HexVisitor implements ComputeGrammarVisitor {
	private final static int FLASH_BLOCK_SIZE = 0x40;
	
	private enum HexRecords {
		HEADER(4),
		DATA(0),
		TRAILER(1);
		private final int code;
		private HexRecords(int i) {
			code = i;
		}
		public int getCode() { return code;}
	}

	
	// address information
	class AddressInfo {
		public int version;
		
		public int ruleAddress;
		public int numRules;
		
		public int expressionAddress;
		public int numExpressions;
		
		public int eventAddress;
		public int numEvents;
		
		public int actionAddress;
		public int numActionBytes;
		
		public int ruleIndex;
		public int expressionIndex;
		public int ruleState;
		public int nvPtr;
		
		public AddressInfo(int ver, int ra, int nr, int ea, int ne, int va, int nv, int aa, int na, int ri, int ei, int rs, int np) {
			version = ver;
			
			ruleAddress = ra;
			numRules = nr;
			
			expressionAddress = ea;
			numExpressions = ne;
			
			eventAddress = va;
			numEvents = nv;
			
			actionAddress = aa;
			numActionBytes = na;
			
			ruleIndex = ri;
			expressionIndex = ei;
			ruleState = rs;
			nvPtr = np;
		}
	}
	private enum Action {
		ACTION_OPCODE_NOP(0),


		ACTION_OPCODE_DELAY(251),
		ACTION_OPCODE_SEND_CBUS(252), 
		ACTION_OPCODE_SEND(253),
		ACTION_OPCODE_END(254);
		private final int code;
		private Action(int i) {
			code = i;
		}
		public int getCode() { return code;}
	}
	private enum RuleState {
	    VALID(0),
	    TOO_MANY_RULES(1),
	    TOO_MANY_EXPRESSIONS(2),
	    TOO_MANY_EVENTS(3),
	    TOO_MANY_ACTIONS(4),
	    UNKNOWN_INSTRUCTION(5),
	    UNKNOWN_ACTION(6),
	    INVALID_EVENT(7),
	    ARGUMENT_TOO_LARGE(8),
	    INVALID_INDEX(9);
	    private final int code;
		private RuleState(int i) {
			code = i;
		}
		public int getCode() { return code;}
	} ;
	
	private class Rule {
		int within;
		Expression expression;
		int actions;
		int thens;
	}
	
	private class Operand {
		Expression expression=null;
		int number;
		@Override
		public String toString() {
			return expression!=null?("Expression:"+expression):("Number:"+number);
		}
	}
	
	
	private class Expression {
		public Expression() {
			opCode = null;
			op1 = null;
			op2 = null;
		}
		NvOpCode opCode;
		Operand op1;
		Operand op2;
	}
	
	private Map<Integer, AddressInfo> addressInfos;
	private RuleState ruleState;
	private List<Rule> rules;
	private List<Expression> expressions;
	private List<Integer> datas;
	private ArrayList<Event> events;
	private AddressInfo addressInfo;
	private FileWriter fw;


	public HexVisitor(Integer dataVersion, String outputFilename) throws IOException {
		
		addressInfos = new HashMap<Integer, AddressInfo>();
		/* Address info for data version 1 */
		addressInfos.put(1, new AddressInfo(
				2,				// flash version
				0x7B7C,  50,	// rules
				0x7924, 200,	// expressions
				0x7C44, 100,	// events
				0x7F00, 254,	// action bytes in NV space
				0x7923,			// ruleIndex
				0x7922,			// expressionIndex
				0x7921,			// ruleState
				0x7920			// nvPtr
		));
		ruleState = RuleState.VALID;
		rules = new ArrayList<Rule>();
		expressions = new ArrayList<Expression>();
		datas = new ArrayList<Integer>();
		events = new ArrayList<Event>();
		
		addressInfo = addressInfos.get(dataVersion);
		if (addressInfo == null) {
			System.out.println("Can't determine addressinfo for datavsersion "+dataVersion);
			throw new IllegalArgumentException("Can't determine addressinfo for datavsersion "+dataVersion);
		}
		fw = new FileWriter(outputFilename);
		System.out.println("opened \""+outputFilename+"\" for writing");
		System.out.println("Running HexVisitor");
		datas.add(addressInfo.version);		// NV#0 is the version
	}

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
	public Object visit(ASTSetNN node, Object data) {
		node.childrenAccept(this, data);
		return null;
	}

	@Override
	public Object visit(ASTDefineList node, Object data) {
		node.childrenAccept(this, data);
		return null;
	}

	@Override
	public Object visit(ASTRuleList node, Object data) {
		int flashBlockStart=0;
		int min_addr=0;
		
		node.childrenAccept(this, data);
		/* Done all the processing, now output the actual records */
		/* First output the param structure but change the load address to be that
		 * of the first data structure in memory.
		 * The version numbers in this param structure don't matter since this
		 * data isn't actually written to the module but just used by FCU.
		 */

		try {
			fw.write(hexRecord(HexRecords.HEADER, 0, "0000"));
		} catch (IOException e) {
			e.printStackTrace();
		}
		try {
			min_addr = Math.min(addressInfo.actionAddress, 
					Math.min(addressInfo.eventAddress,
					Math.min(addressInfo.expressionAddress, 
					Math.min(addressInfo.ruleAddress, 
					Math.min(addressInfo.actionAddress,
					Math.min(addressInfo.ruleIndex,
					Math.min(addressInfo.expressionIndex,
					Math.min(addressInfo.ruleState, addressInfo.nvPtr))))))));
			/* 
			 * This is a nasty work-around for a limitation in Bootloader.asm
			 * The address must point to the start of a flash block.
			 * So let's round the address down and output some 0xFF as padding at the start.
			 */
			flashBlockStart = min_addr & ~(FLASH_BLOCK_SIZE-1);
			System.out.println("min_addr="+Integer.toHexString(min_addr)+" flashBlockStart="+Integer.toHexString(flashBlockStart));
			// address is little endian
			String param1 = "A5623C6401FE020B0D01"+
					Util.hexPair(flashBlockStart%256)+
					Util.hexPair(flashBlockStart/256)+
					"00000000";
			String param2 = "0000010100000000140040080000";
			fw.write(hexRecord(HexRecords.DATA, 0x820, param1));
			// calculate the parameter checksum
			// MANU_ID+				= A5
			// MINOR_VER+			= 62
			// MODULE_ID+			= 3C
			// NUM_EVENTS+			= 64
			// EVperEVT+			= 01
			// (NV_NUM-1)+			= FE
			// MAJOR_VER+			= 02
			// MODULE_FLAGS+		= 0B
			// CPU+					= 0D
			// PB_CAN +				= 01	Total of this = 02C1
			// (LOAD_ADDRESS>>8)+	= flashBlockStart/256
			// (LOAD_ADDRESS&0xFF)+	= flashBlockStart%256
			// CPUM_MICROCHIP+		= 01
			// BETA+				= 01
			// sizeof(ParamVals)+	= 14
			// (MNAME_ADDRESS>>8)+	= 40
			// (MNAME_ADDRESS&0xFF) = 08	Total of this part = 0064
			int chksum = 0x02C1 + flashBlockStart/256 + flashBlockStart%256 + 0x005e;
					
			fw.write(hexRecord(HexRecords.DATA, 0x830, param2+
					Util.hexPair(chksum%256)+
					Util.hexPair(chksum/256)
			));
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		try {
			fw.write(hexRecord(HexRecords.HEADER, 0, "0000"));
		} catch (IOException e) {
			e.printStackTrace();
		}
		// Now output any necessary padding
		for (;flashBlockStart<min_addr;flashBlockStart++) {
			try {
				fw.write(hexRecord(HexRecords.DATA, flashBlockStart, "FF"));
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
		}

		
		/* now the index and state data */
		try {
			fw.write(hexRecord(HexRecords.DATA, addressInfo.nvPtr, 
					Util.hexPair(datas.size())));
		} catch (IOException e) {
			e.printStackTrace();
		}
		try {
			fw.write(hexRecord(HexRecords.DATA, addressInfo.ruleState, 
					Util.hexPair(ruleState.getCode())));
		} catch (IOException e) {
			e.printStackTrace();
		}
		try {
			fw.write(hexRecord(HexRecords.DATA, addressInfo.expressionIndex, 
					Util.hexPair(expressions.size())));
		} catch (IOException e) {
			e.printStackTrace();
		}
		try {
			fw.write(hexRecord(HexRecords.DATA, addressInfo.ruleIndex, 
					Util.hexPair(rules.size())));
		} catch (IOException e) {
			e.printStackTrace();
		}	
		
		/* Next the Expressions */
		try {
			fw.write(hexRecord(HexRecords.HEADER, 0, "0000"));
		} catch (IOException e) {
			e.printStackTrace();
		}
		if (expressions.size() > addressInfo.numExpressions) {
			System.out.println("Too many expressions "+expressions.size()+" max for module="+addressInfo.numExpressions);
			try {
				fw.write(hexRecord(HexRecords.DATA, addressInfo.ruleState,
						Util.hexPair(RuleState.TOO_MANY_EXPRESSIONS.getCode())));
			} catch (IOException e1) {
				e1.printStackTrace();
			}
			return null;
		}
		for (Expression e : expressions) {
			try {
				//System.out.println("e="+e+" op="+e.opCode+" op1="+e.op1+" op2="+e.op2);
				fw.write(hexRecord(HexRecords.DATA, addressInfo.expressionAddress+expressions.indexOf(e)*3,
						Util.hexPair(e.opCode.code())+
						Util.hexPair(e.op1!=null?e.op1.expression!=null?expressions.indexOf(e.op1.expression):e.op1.number:0)+		// op1
						Util.hexPair(e.op2!=null?e.op2.expression!=null?expressions.indexOf(e.op2.expression):e.op2.number:0)		// op2
						));	// expressions are 3 bytes
			} catch (IOException e1) {
				e1.printStackTrace();
			}
		}
		
		/* next the rules */
		try {
			fw.write(hexRecord(HexRecords.HEADER, 0, "0000"));
		} catch (IOException e) {
			e.printStackTrace();
		}
		if (rules.size() > addressInfo.numRules) {
			System.out.println("Too many rules "+rules.size()+" max for module="+addressInfo.numRules);
			try {
				fw.write(hexRecord(HexRecords.DATA, addressInfo.ruleState,
						Util.hexPair(RuleState.TOO_MANY_RULES.getCode())));
			} catch (IOException e1) {
				e1.printStackTrace();
			}
			return null;
		}
		for (Rule r : rules) {
			try {
				fw.write(hexRecord(HexRecords.DATA, addressInfo.ruleAddress+rules.indexOf(r)*4,	// rules are 4 bytes
						Util.hexPair(r.within)+
						Util.hexPair(expressions.indexOf(r.expression))+
						Util.hexPair(r.actions)+	//actions
						Util.hexPair(r.thens)	// thens
						));	
			} catch (IOException e1) {
				e1.printStackTrace();
			}	
		}
		/* next the events */
		try {
			fw.write(hexRecord(HexRecords.HEADER, 0, "0000"));
		} catch (IOException e) {
			e.printStackTrace();
		}
		if (events.size() >= addressInfo.numEvents) {
			System.out.println("Too many events "+events.size()+" max for module="+addressInfo.numEvents);
			try {
				fw.write(hexRecord(HexRecords.DATA, addressInfo.ruleState,
						Util.hexPair(RuleState.TOO_MANY_EVENTS.getCode()))+"");
			} catch (IOException e1) {
				e1.printStackTrace();
			}
			return null;
		}
		for (Event e:events) {
			try {
				fw.write(hexRecord(HexRecords.DATA, addressInfo.eventAddress+events.indexOf(e)*7,	//Address - events are 7 bytes
						"11FF"+					// 1 EV, not a continuation, not free
						Util.hexSwapQuad(e.getNN())+	// NN
						Util.hexSwapQuad(e.getEN())+	// EN
						Util.hexPair(events.indexOf(e)+1)));	// EV1
			} catch (IOException ee) {
				ee.printStackTrace();
			}
		}
		
		/* next the Actions stored in the NV space */
		try {
			fw.write(hexRecord(HexRecords.HEADER, 0, "0000"));
		} catch (IOException e) {
			e.printStackTrace();
		}
		if (datas.size() > addressInfo.numActionBytes) {
			System.out.println("Too many actions "+datas.size()+" max for module="+addressInfo.numActionBytes);
			try {
				fw.write(hexRecord(HexRecords.DATA, addressInfo.ruleState,
						Util.hexPair(RuleState.TOO_MANY_ACTIONS.getCode())));
			} catch (IOException e1) {
				e1.printStackTrace();
			}
			return null;
		}
		for (int i=0; i<datas.size(); i++) {
			try {
				fw.write(hexRecord(HexRecords.DATA, addressInfo.actionAddress+i,	// data is 1 byte
						Util.hexPair(datas.get(i))
						));
			} catch (IOException e1) {
				e1.printStackTrace();
			}	
		}
		
		
		/* the trailer */
		try {
			fw.write(hexRecord(HexRecords.TRAILER, 0, ""));
		} catch (IOException e2) {
			e2.printStackTrace();
		}
		try {
			System.out.println("Closing file");
			fw.close();
		} catch (IOException e1) {
			e1.printStackTrace();
		}
		return null;
	}

	private String hexRecord(HexRecords type, int address, String record) {
		
		return ":"+
				Util.hexPair(record.length()/2)+
				Util.hexQuad(address)+
				Util.hexPair(type.getCode())+
				record+
				Util.hexPair(hex_checksum(
						Util.hexPair(record.length()/2)+
						Util.hexQuad(address)+
						Util.hexPair(type.getCode())+
						record))+
				"\n";
	}

	private int hex_checksum(String string) {
		int ck = 0;
		for (int i=0; i<string.length(); i+=2) {
			String ss = string.substring(i, i+2);
			ck -= Util.fromHex(ss.charAt(0), ss.charAt(1));
		}
		return ck;
	}
	@Override
	public Object visit(ASTDefine node, Object data) {
		ASTIdentifier id = (ASTIdentifier)node.jjtGetChild(0);
		ASTEventLiteral ev = (ASTEventLiteral)node.jjtGetChild(1);

		//System.out.println("Got a define for identifier("+id.getName()+")="+ev.getEvent());
		//System.out.println("Add Event "+ev.getEvent()+" EV#1="+Variables.getIndex());
		events.add(ev.getEvent());
		Variables.setIndex(id.getName());
		
		node.childrenAccept(this, data);
		return null;
	}

	@Override
	public Object visit(ASTRule node, Object data) {
		if (node.jjtGetNumChildren() > 0) {
			Rule r = new Rule();
			r.expression = (Expression) node.jjtGetChild(0).jjtAccept(this, data);
			r.within = ((Integer)(node.jjtGetChild(1).jjtAccept(this, data)));

			/* Actions */
			r.actions = datas.size();				// save the current location
			ASTActionList actions = (ASTActionList) node.jjtGetChild(2);
			actions.jjtAccept(this, data);
			
			/* thens */
			if (node.jjtGetNumChildren() > 3) {	// have a THEN action list
				r.thens = datas.size();				// save the current location
				actions = (ASTActionList) node.jjtGetChild(3);
				actions.jjtAccept(this, data);
			} else {
				// no thens
				r.thens = datas.size()-1;				// save the location of the last END
			}
			
			rules.add(r);
		}
		return null;
	}

	@Override
	public Object visit(ASTTime node, Object data) {
		// Convert to 1/10 of seconds
		Node n = node.jjtGetChild(0);
		ASTUnits u = (ASTUnits)n;
		return (node.getTime()*u.getUnits())/100;
	}

	@Override
	public Object visit(ASTUnits node, Object data) {
		return null;
	}

	@Override
	public Object visit(ASTActionList node, Object data) {
		node.childrenAccept(this, data);
		datas.add(Action.ACTION_OPCODE_END.getCode());	// terminate the list of Actions
		return null;
	}

	@Override
	public Object visit(ASTAction node, Object data) {
		if (node.getAction() == ASTAction.OpCodes.DELAY) {	// just Time()
			datas.add(Action.ACTION_OPCODE_DELAY.getCode());
			datas.add((Integer) node.jjtGetChild(0).jjtAccept(this, data));
		}
		if (node.getAction() == ASTAction.OpCodes.SEND) {	// Event
			datas.add(Action.ACTION_OPCODE_SEND.getCode());
			int eventIndex = (Integer) node.jjtGetChild(0).jjtAccept(this, data);
			
			datas.add(eventIndex);
		}
		if (node.getAction() == ASTAction.OpCodes.CBUS) {	// Hex string
			datas.add(Action.ACTION_OPCODE_SEND_CBUS.getCode());
			String m = node.getCbusMessage();
			//System.out.println("m="+m);
			// go through the supplied message
			List<Integer> bytes = new ArrayList<Integer>();
			for (int i=0; i<m.length(); i++) {	// start at the end
				char c1 = m.charAt(i);
				i++;
				char c2 = m.charAt(i);
				//System.out.println("c1="+c1+" c2="+c2);
				Integer b = Util.fromHex(c1, c2);
				bytes.add(b);
			}
			//System.out.println("bytes="+bytes);
			// check that the number of bytes supplied matched the length defined by opcode
			int opcode = bytes.get(0);
			int len = (opcode >> 5)+1;
			if (bytes.size() != len) {
				System.out.println("CBUS message - number of bytes doesn't match the OPC");
			}
			// output the cbus message bytes
			for (int b: bytes) {
				datas.add(b);	
			}
		}
		return null;
	}

	@Override
	public Object visit(ASTExpression node, Object data) {
		return node.jjtGetChild(0).jjtAccept(this, data);
	}

	@Override
	public Object visit(ASTOrExpression node, Object data) {
		if (node.jjtGetNumChildren() >  1) {
			Expression e = new Expression();
			expressions.add(e);
			Expression fe = e;
			e.opCode = NvOpCode.OR;
			e.op1 = new Operand();
			e.op1.expression = (Expression) node.jjtGetChild(0).jjtAccept(this, data);	// first
			
			for (int i=2; i<node.jjtGetNumChildren(); i++) {
				Expression e2 = new Expression();
				expressions.add(e2);
				e2.opCode = NvOpCode.OR;
				e.op2 = new Operand();
				e.op2.expression = e2;
				
				e2.op1 = new Operand();
				e2.op1.expression =	(Expression) node.jjtGetChild(i-1).jjtAccept(this, data);
				e = e2;
			}
			e.op2 = new Operand();
			e.op2.expression = (Expression) node.jjtGetChild(node.jjtGetNumChildren()-1).jjtAccept(this, data); //last

			return fe;
		} 
		return node.jjtGetChild(0).jjtAccept(this, data);
	}

	@Override
	public Object visit(ASTAndExpression node, Object data) {
		if (node.jjtGetNumChildren() >  1) {
			Expression e = new Expression();
			expressions.add(e);
			Expression fe = e;
			e.opCode = NvOpCode.AND;
			e.op1 = new Operand();
			e.op1.expression = (Expression) node.jjtGetChild(0).jjtAccept(this, data);	// first
			
			for (int i=2; i<node.jjtGetNumChildren(); i++) {
				Expression e2 = new Expression();
				expressions.add(e2);
				e2.opCode = NvOpCode.AND;
				e.op2 = new Operand();
				e.op2.expression = e2;
				
				e2.op1 = new Operand();
				e2.op1.expression =	(Expression) node.jjtGetChild(i-1).jjtAccept(this, data);
				e = e2;
			}
			e.op2 = new Operand();
			e.op2.expression = (Expression) node.jjtGetChild(node.jjtGetNumChildren()-1).jjtAccept(this, data); //last
			return fe;
		} 
		return node.jjtGetChild(0).jjtAccept(this, data);
	}

	@Override
	public Object visit(ASTUnaryBooleanExpression node, Object data) {
		if (node.getOp()) {
			Expression e = new Expression();
			expressions.add(e);
			e.opCode = NvOpCode.NOT;
			e.op1 = new Operand();
			e.op1.expression = (Expression) node.jjtGetChild(0).jjtAccept(this, data);
			return e;
		}
		return node.jjtGetChild(0).jjtAccept(this, data);
	}

	@Override
	public Object visit(ASTRelationalExpression node, Object data) {
		if (node.jjtGetNumChildren() > 1) {
			Expression e = new Expression();
			String op = node.getOpCode();
			expressions.add(e);
			if ("is".equals(op)) {
				e.opCode = NvOpCode.EQUALS;
			} else if ("equals".equals(op)) {
				e.opCode = NvOpCode.EQUALS;
			} else if ("=".equals(op)) {
				e.opCode = NvOpCode.EQUALS;
			} else if ("!=".equals(op)) {
				e.opCode = NvOpCode.NOTEQUALS;
			} else if ("<".equals(op)) {
				e.opCode = NvOpCode.LESS;
			} else if ("<=".equals(op)) {
				e.opCode = NvOpCode.LESSEQUAL;
			} else if (">".equals(op)) {
				e.opCode = NvOpCode.MORE;
			} else if (">=".equals(op)) {
				e.opCode = NvOpCode.MOREEQUAL;
			} else {
				System.out.println("Error unknown RelationExpression operator"+op);
				return node.jjtGetChild(0).jjtAccept(this, data);
			}
			e.op1 = new Operand();
			e.op1.expression = (Expression) node.jjtGetChild(0).jjtAccept(this, data);
			e.op2 = new Operand();
			e.op2.expression = (Expression) node.jjtGetChild(1).jjtAccept(this, data);
			return e;
		}
		return node.jjtGetChild(0).jjtAccept(this, data);
	}

	@Override
	public Object visit(ASTPrimaryBooleanExpression node, Object data) {
		if (node.getOpCode() == OpCodes.STATE) {
			Expression e = new Expression();
			expressions.add(e);
			
			ASTIdentifier ei;
			ei = (ASTIdentifier) node.jjtGetChild(0);
			ASTEventState es;
			es = (ASTEventState) node.jjtGetChild(1);

			if (es.getState() == EventState.ON) {
				e.opCode = NvOpCode.STATE_ON;
			} else if (es.getState() == EventState.OFF) {
				e.opCode = NvOpCode.STATE_OFF;
			} else if (es.getState() == EventState.EXPLICIT_OFF) {
				e.opCode = NvOpCode.EXPLICIT_STATE_OFF;
			} else if (es.getState() == EventState.EXPLICIT_ON) {
				e.opCode = NvOpCode.EXPLICIT_STATE_ON;
			} else {
				e.opCode = NvOpCode.EXPLICIT_STATE_UNKNOWN;
			} 
			
			int ev = Variables.getIndex(ei.getName());

			if (ev > 0) {
				e.op1 = new Operand();
				e.op1.number = ev;
			} else {
				System.out.println("Error: undefined variable \""+ei.getName()+"\"");
			}
			return e;
		}
		if (node.getOpCode() == OpCodes.SEQUENCE) {
			Expression e = new Expression();
			expressions.add(e);
			e.opCode = NvOpCode.SEQUENCE;
			e.op1 = new Operand();
			e.op1.number = node.jjtGetNumChildren();
			e.op2 = new Operand();
			e.op2.number = datas.size();	// save current pointer 
			for (int i=0; i<node.jjtGetNumChildren(); i++) {
				datas.add((Integer) node.jjtGetChild(i).jjtAccept(this, data));
			}
			return e;
		} 
		if (node.getOpCode() == OpCodes.RECEIVED) {
			Expression e = new Expression();
			expressions.add(e);
			e.opCode = NvOpCode.RECEIVED;
			e.op1 = new Operand();
			e.op1.number = (Integer) node.jjtGetChild(0).jjtAccept(this, data);
			return e;
		} 
		if (node.getOpCode() == OpCodes.BEFORE) {
			Expression e = new Expression();
			expressions.add(e);
			e.opCode = NvOpCode.BEFORE;
			e.op1 = new Operand();
			e.op1.number = (Integer) node.jjtGetChild(0).jjtAccept(this, data);
			e.op2 = new Operand();
			e.op2.number = (Integer) node.jjtGetChild(1).jjtAccept(this, data);
			return e;
		}
		if (node.getOpCode() == OpCodes.AFTER) {
			Expression e = new Expression();
			expressions.add(e);
			e.opCode = NvOpCode.AFTER;
			e.op1 = new Operand();
			e.op1.number = (Integer) node.jjtGetChild(0).jjtAccept(this, data);
			e.op2 = new Operand();
			e.op2.number = (Integer) node.jjtGetChild(1).jjtAccept(this, data);
			return e;
		}
		if (node.getOpCode() == OpCodes.INPUT) {
			Expression e = new Expression();
			expressions.add(e);
			e.opCode = NvOpCode.INPUT;
			//System.out.println("INPUT index="+node.getIndex());
			e.op1 = new Operand();
			e.op1.number = Integer.parseInt(node.getIndex());
			return e;
		}
		return node.jjtGetChild(0).jjtAccept(this, data);
	}

	@Override
	public Object visit(ASTMessage node, Object data) {
		ASTMessageState ms;
		ms = (ASTMessageState) node.jjtGetChild(0);
		ms.jjtAccept(this, data);
		
		ASTIdentifier ei;
		ei = (ASTIdentifier) node.jjtGetChild(1);
		ei.jjtAccept(this, data);
		
		int ev = Variables.getIndex(ei.getName());
		if (ms.getState() == MessageState.ON) {
			ev += 0x80;
		}
		return ev;
	}

	@Override
	public Object visit(ASTAdditiveExpression node, Object data) {
		if (node.jjtGetNumChildren() >  1) {
			Expression e = new Expression();
			expressions.add(e);
			Expression fe = e;
			String op = node.getOpCode();
			if ("+".equals(op)) {
				e.opCode = NvOpCode.PLUS;
			} else if ("-".equals(op)){
				e.opCode = NvOpCode.MINUS;
			} else {
				System.out.println("Error unknown AdditiveExpression operator"+op);
			}
			e.op1 = new Operand();
			e.op1.expression = (Expression) node.jjtGetChild(0).jjtAccept(this, data);
			
			for (int i=2; i<node.jjtGetNumChildren(); i++) {
				Expression e2 = new Expression();
				expressions.add(e2);
				op = node.getOpCode();
				if ("+".equals(op)) {
					e2.opCode = NvOpCode.PLUS;
				} else if ("-".equals(op)){
					e2.opCode = NvOpCode.MINUS;
				} else {
					System.out.println("Error unknown AdditiveExpression operator"+op);
				}
				e.op2 = new Operand();
				e.op2.expression = e2;
				
				e2.op1 = new Operand();
				e2.op1.expression =	(Expression) node.jjtGetChild(i-1).jjtAccept(this, data);
				e = e2;
			}
			e.op2 = new Operand();
			e.op2.expression = (Expression) node.jjtGetChild(node.jjtGetNumChildren()-1).jjtAccept(this, data); //last
			return fe;
		} 
		return node.jjtGetChild(0).jjtAccept(this, data);

	}

	@Override
	public Object visit(ASTPrimaryIntegerExpression node, Object data) {
		if (node.getOpCode() == ASTPrimaryIntegerExpression.OpCodes.COUNT) {
			Expression e = new Expression();
			expressions.add(e);
			e.opCode = NvOpCode.COUNT;
			e.op1 = new Operand();
			e.op1.number = (Integer) node.jjtGetChild(0).jjtAccept(this, data);
			return e;
		} else if (node.getOpCode() == ASTPrimaryIntegerExpression.OpCodes.INTEGER) {
			Expression e = new Expression();
			expressions.add(e);
			e.opCode = NvOpCode.INTEGER;
			e.op1 = new Operand();
			e.op1.number = node.getInt();
			return e;
		}
		return node.jjtGetChild(0).jjtAccept(this, data);
	}

	@Override
	public Object visit(ASTIdentifier node, Object data) {
		return node.getName();
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
	public Object visit(ASTEventState node, Object data) {
		return null;
	}

}
