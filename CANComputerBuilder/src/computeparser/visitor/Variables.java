package computeparser.visitor;

import java.util.HashMap;
import java.util.Map;

public class Variables {
	static private Map<String, Integer> defines = new HashMap<String, Integer>();
	static private int eventIndex = 1;
	
	static public int getIndex(String name) {
		if (defines.get(name) == null) {
			return 0;
		};
		return defines.get(name);
	}
	
	static public int getIndex() {
		return eventIndex;
	}

	static public void setIndex(String name) {		
		defines.put(name, eventIndex++);
	}
}
