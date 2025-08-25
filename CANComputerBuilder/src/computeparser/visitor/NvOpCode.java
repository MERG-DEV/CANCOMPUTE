package computeparser.visitor;

public enum NvOpCode {
	SEQUENCE			(222),	// DE followed by count and then that number of event indexes

	RECEIVED			(224),	// E0 followed by one NV giving the event index
    STATE_ON			(225),	// E1 followed by one NV giving the event index
	STATE_OFF			(226),	// E2 followed by one NV giving the event index
	EXPLICIT_STATE_ON 	(227),  // E3 followed by one NV giving the event index
    EXPLICIT_STATE_OFF	(228),  // E4 followed by one NV giving the event index
    EXPLICIT_STATE_UNKNOWN(229),// E5 followed by one NV giving the event index
	
	BEFORE				(230),	// E6 followed by two NV giving the event indexes
	
	AFTER				(234),	// EA followed by two NV giving the event indexes
	THEN				(235),	// EB followed by sequence of actions
	INTEGER				(236),	// EC followed by one NV of the integer value
	PLUS				(237),	// ED followed by two integer expressions
	MINUS				(238),	// EE followed by two integer expressions
	EQUALS				(239),	// EF followed by two integer expressions
	NOTEQUALS			(240),	// F0 followed by two integer expressions
	LESS				(241),	// F1 followed by two integer expressions
	LESSEQUAL			(242),	// F2 followed by two integer expressions
	MORE				(243),	// F3 followed by two integer expressions
	MOREEQUAL			(244),	// F4 followed by two integer expressions
	RULE				(245),	// F5 followed by one NV giving the time period, one boolean expression and a sequence of actions (DELAY, SEND_ON or SEND_OFF)
	AND					(246),	// F6 followed by two boolean expressions
	NOT					(247),	// F7 followed by one boolean expression
	OR					(248),	// F8 followed by two boolean expressions
    INPUT       		(249),  // F9 followed by index
	COUNT				(250),	// FA followed by one NV giving the event index
	
	DELAY				(251),	// FB followed by one NV giving the time period in 0.1sec units
    CBUS        		(252),  // FC followed by a CBUS message
	SEND				(253),	// FD followed by one NV giving the event index
	END					(254);  // FE

	
	private int code;
	
	private NvOpCode(int code) {
		this.code = code;
	}
	public int code() {
		return code;
	}
	public static NvOpCode find(int nv) {
		for(NvOpCode op : NvOpCode.values()) {
			if (op.code() == nv) return op;
		}
		return null;
	}
}
