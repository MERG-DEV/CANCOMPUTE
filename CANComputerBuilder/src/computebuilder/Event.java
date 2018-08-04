package computebuilder;


public class Event {
	private int nn;
	private int en;

	public void setNN(int n) {
		nn = n;
	}

	public void setEN(int e) {
		en = e;		
	}
	public int getNN() {
		return nn;
	}
	public int getEN() {
		return en;
	}
	public String toString() {
		return nn+":"+en;
	}

	public void print() {
		System.out.println("Event nn="+nn+":en="+en);
		
	}
	
	@Override
	public boolean equals(Object o) {
		if (! (o instanceof Event)) return false;
		Event e = (Event)o;
		if (nn != e.nn) return false;
		if (en != e.en) return false;
		return true;
	}
	
	@Override
	public int hashCode() {
		return nn*7 + en;
	}
}
