import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;

import computeparser.ComputeGrammar;
import computeparser.ComputeGrammarVisitor;
import computeparser.SimpleNode;
import computeparser.visitor.GridConnectDefinesVisitor;
import computeparser.visitor.GridConnectNvVisitor;
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
		System.out.println("/tif filename is specified then the file is parsed for rules otherwise standard inut is used.");
	}
	private enum OutputFormat {
		TEXT,
		GRID_CONNECT,
		FCU_XML,
		DEBUG
	};
	
	public static void main(String args []) throws FileNotFoundException
	{
		InputStream is = null;
		OutputFormat outputFormat = OutputFormat.TEXT;
		
	    System.out.println("Rule Compiler v5.1 for rule format v2");
		
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
	      }
	      dv.visit(n, null);
	      if (nv != null) nv.visit(n, null);

	    }
	    catch (Exception e)
	    {
	      System.out.println("Oops.");
	      System.out.println(e.getMessage());
	      //e.printStackTrace();
	    }
	  }
}
