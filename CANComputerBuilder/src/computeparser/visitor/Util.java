package computeparser.visitor;

public class Util {
	/**
	 * Convert 2 hex characters to its integer value 0-255.
	 * @param c1
	 * @param c2
	 * @return
	 */
	public static int fromHex(char c1, char c2){
		return hexChar(c1)*16 + hexChar(c2);
	}
	/**
	 * Convert a single hex character to its integer value 0-16.
	 * @param c
	 * @return
	 */
	public static int hexChar(char c){
		if (c <= '9') return (c-'0');
		if (c <= 'Z') return (c-'A'+10);
		return (c-'a'+10);
	}
	/**
	 * Convert an integer (0-255) to a pair of hex digits.
	 * @param i
	 * @return
	 */
	public static String hexPair(int ih) {
		String h = "00"+Integer.toHexString(ih);
		int l = h.length();
		return h.substring(l-2,l).toUpperCase();
	}
	/**
	 * Convert an integer (0-65535) to 4 hex digits.
	 * @param ih
	 * @return
	 */
	public static String hexQuad(int ih) {
		String h = "0000"+Integer.toHexString(ih);
		int l = h.length();
		return h.substring(l-4,l).toUpperCase();
	}
	/**
	 * Convert an integer (0-65535) to 4 hex digits. Low byte first.
	 * @param ih
	 * @return
	 */
	public static String hexSwapQuad(int ih) {
		String h = hexPair(ih%256);
		h += hexPair(ih/256);
		return h;
	}
	/**
	 * Convert an integer to a String with padded with spaces.
	 * @param i
	 * @return
	 */
	public static String dec(int i) {
		String ret=""+i;
		if (i < 10) ret += " ";
		if (i < 100) ret += " ";
		return ret;
	}




}
