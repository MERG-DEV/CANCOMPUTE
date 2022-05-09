import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import computeparser.ComputeGrammar;
import computeparser.ComputeGrammarVisitor;
import computeparser.SimpleNode;
import computeparser.visitor.GridConnectDefinesVisitor;
import computeparser.visitor.GridConnectNvVisitor;
import computeparser.visitor.HexVisitor;
import computeparser.visitor.TextDefinesVisitor;
import computeparser.visitor.TextNvVisitor;
import computeparser.visitor.fcuDefinesVisitor;
import computeparser.visitor.fcuNvVisitor;
import computeparser.visitor.PrintVisitor;

public class Compute {
	private static void usage() {
		System.out.println("Usage: Compute [-o g|f|t] [filename]");
		System.out.println("/t-og\toutputs in grid connect format");
		System.out.println("/t-of\toutputs in FCU XML format");
		System.out.println("/t-od\tprints debug of rules");
		System.out.println("/t-ot\toutputs in text format (default)");
		System.out.println("/t-oh[module version]\toutputs in hex format");
		System.out.println("/tif filename is specified then the file is parsed for rules otherwise standard inut is used.");
	}
	private enum OutputFormat {
		TEXT,
		GRID_CONNECT,
		FCU_XML,
		DEBUG,
		HEX
	};
	private static final Map<String, Integer> dataVersions;
    static {
        Map<String, Integer> aMap = new HashMap<String, Integer>();
        aMap.put("2", 1);
        dataVersions = Collections.unmodifiableMap(aMap);
    }
	
	public static void main(String args []) throws FileNotFoundException
	{
		InputStream is = null;
		OutputFormat outputFormat = OutputFormat.TEXT;
		String moduleVersion = "v2";	// default version
		Integer dataVersion = 1;

		
	    System.out.println("Rule Compiler v6.1 for rule format v3");
		
		if (args.length > 0) {
			for (int i=0; i<args.length; i++) {
				if (args[i].startsWith("-")) {
					if ("-og".equals(args[i])) {
						outputFormat = OutputFormat.GRID_CONNECT;
					} else if ("-ot".equals(args[i])) {
						outputFormat = OutputFormat.TEXT;
					} else if ("-of".equals(args[i])) {
						outputFormat = OutputFormat.FCU_XML;
					} else if ("-od".equals(args[i])) {
						outputFormat = OutputFormat.DEBUG;
					} else if ((args[i] != null) && (args[i].startsWith("-oh"))) {
						outputFormat = OutputFormat.HEX;
						String suppliedVersion;
						if (args[i].length() > 3) {
							suppliedVersion = args[i].substring(3);
						} else {
							suppliedVersion = moduleVersion;
						}
						// now trim for just major version
						int start = 0;
						int end = 0;
						try {
							while (! Character.isDigit(suppliedVersion.charAt(start))) start++;
							end = start+1;
							while (end<suppliedVersion.length()) {
								if (! Character.isDigit(suppliedVersion.charAt(end))) break;
								end++;
							}
							System.out.println("start="+start+" end="+end);
							moduleVersion = moduleVersion.substring(start, end);
						} catch (IndexOutOfBoundsException e) {
							// leave moduleVersion at the default
							System.out.println("Can't understand module version "+suppliedVersion+" - using "+moduleVersion);
						}
						System.out.println("module version="+moduleVersion);
						
						dataVersion = dataVersions.get(moduleVersion);
						if (dataVersion == null) {
							System.out.println("Don't have a dataVersion for module version "+moduleVersion+" assuming 1.");
							dataVersion = 1;
						}
					} else {
						System.out.println("Unknown argument:"+args[i]);
						usage();
					}
				} else {
					System.out.println("Reading from file..."+args[i]);
					is = new FileInputStream(new File(args[i]));
				}
			}
			
		} 
		if (is == null){
			System.out.println("Reading from standard input...");
			is = System.in;
		}
	    
	    new ComputeGrammar(is);
	    System.out.println("Parsing...");
	    try
	    {
	      SimpleNode n = ComputeGrammar.Start();
	      //System.out.println("*****Print");
	      //PrintVisitor pv = new PrintVisitor();
	      //pv.visit(n, null);

	      ComputeGrammarVisitor dv = null;
	      ComputeGrammarVisitor nv = null;
	      switch (outputFormat) {
	      case GRID_CONNECT:
	    	  dv = new GridConnectDefinesVisitor();
	    	  nv = new GridConnectNvVisitor();
	    	  break;
	      case TEXT:
	    	  dv = new TextDefinesVisitor();
	    	  nv = new TextNvVisitor();
	    	  break;
	      case FCU_XML:
	    	  dv = new fcuDefinesVisitor();
	    	  nv = new fcuNvVisitor();
	    	  break;
	      case DEBUG:
	    	  dv = new PrintVisitor();
	    	  break;
	      case HEX:
	    	  dv = new HexVisitor(dataVersion);
	    	  break;
	      }
	      dv.visit(n, null);
	      if (nv != null) nv.visit(n, null);

	    }
	    catch (Exception e)
	    {
	      System.out.println("Oops.");
	      System.out.println(e.getMessage());
	      e.printStackTrace();
	    }
	  }
}
