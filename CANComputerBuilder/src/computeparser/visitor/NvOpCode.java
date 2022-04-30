package computeparser.visitor;

public enum NvOpCode {
	SEQUENCE			(222),	// followed by count and then that number of event indexes

	RECEIVED			(224),	// followed by one NV giving the event index
    STATE_ON			(225),	// followed by one NV giving the event index
	STATE_OFF			(226),	// followed by one NV giving the event index
	EXPLICIT_STATE_ON 	(227),    // E3 followed by one NV giving the event index
    EXPLICIT_STATE_OFF	(228),   // E4 followed by one NV giving the event index
    EXPLICIT_STATE_UNKNOWN(229),   // E5 followed by one NV giving the event index
	
	BEFORE				(230),	// followed by two NV giving the event indexes
	
	AFTER				(234),	// followed by two NV giving the event indexes
	THEN				(235),	// followed by sequence of actions
	INTEGER				(236),	// followed by one NV of the integer value
	PLUS				(237),	// followed by two integer expressions
	MINUS				(238),	// followed by two integer expressions
	EQUALS				(239),	// followed by two integer expressions
	NOTEQUALS			(240),	// followed by two integer expressions
	LESS				(241),	// followed by two integer expressions
	LESSEQUAL			(242),	// followed by two integer expressions
	MORE				(243),	// followed by two integer expressions
	MOREEQUAL			(244),	// followed by two integer expressions
	RULE				(245),	// followed by one NV giving the time period, one boolean expression and a sequence of actions (DELAY, SEND_ON or SEND_OFF)
	AND					(246),	// followed by two boolean expressions
	NOT					(247),	// followed by one boolean expression
	OR					(248),	// followed by two boolean expressions
    INPUT       		(249),   // F9 followed by index
	COUNT				(250),	// followed by one NV giving the event index
	
	DELAY				(251),	// followed by one NV giving the time period in 0.1sec units
    CBUS        		(252),   // FC followed by a CBUS message
	SEND				(253),	// followed by one NV giving the event index
	END					(254);

	
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
