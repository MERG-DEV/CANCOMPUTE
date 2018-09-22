package computeparser.visitor;

public enum NvOpCode {
	RECEIVED_ON			(223),	// followed by one NV giving the event index
	RECEIVED_OFF		(224),	// followed by one NV giving the event index
	STATE_ON			(225),	// followed by one NV giving the event index
	STATE_OFF			(226),	// followed by one NV giving the event index
	BEFORE_OFF_OFF		(227),	// followed by two NV giving the event indexes
	BEFORE_OFF_ON		(228),	// followed by two NV giving the event indexes
	BEFORE_ON_OFF		(229),	// followed by two NV giving the event indexes
	BEFORE_ON_ON		(230),	// followed by two NV giving the event indexes
	AFTER_OFF_OFF		(231),	// followed by two NV giving the event indexes
	AFTER_OFF_ON		(232),	// followed by two NV giving the event indexes
	AFTER_ON_OFF		(233),	// followed by two NV giving the event indexes
	AFTER_ON_ON			(234),	// followed by two NV giving the event indexes
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
	COUNT_ON			(249),	// followed by one NV giving the event index
	COUNT_OFF			(250),	// followed by one NV giving the event index
	DELAY				(251),	// followed by one NV giving the time period in 0.1sec units
	SEND_ON				(252),	// followed by one NV giving the event index
	SEND_OFF			(253),	// followed by one NV giving the event index
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
