import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;

import computeparser.ComputeGrammar;
import computeparser.SimpleNode;
import computeparser.visitor.CbusDefinesVisitor;
import computeparser.visitor.CbusNvVisitor;
import computeparser.visitor.DefinesVisitor;
import computeparser.visitor.NvVisitor;

public class Compute {

	public static void main(String args []) throws FileNotFoundException
	  {
		InputStream is = null;
		boolean cbus = false;
		
	    System.out.println("Rule Compiler v2");
		
		if (args.length > 0) {
			for (int i=0; i<args.length; i++) {
				if (args[i].startsWith("-")) {
					if ("-cbus".equals(args[i])) {
						cbus = true;
					} else {
						System.out.println("Unknown argument:"+args[i]);
					}
				} else {
					System.out.println("Reading from file..."+args[i]);
					is = new FileInputStream(new File(args[i]));
				}
			}
			
		} else {
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

	      
	      if (cbus) {
	    	  CbusDefinesVisitor dv = new CbusDefinesVisitor();
	    	  dv.visit(n, null);
	      
	    	  CbusNvVisitor nv = new CbusNvVisitor();
	    	  nv.visit(n, null);
	      } else {
	    	  System.out.println("*****Defines");
	    	  DefinesVisitor dv = new DefinesVisitor();
	    	  dv.visit(n, null);
	      
	    	  System.out.println("*****Nvs");
	    	  NvVisitor nv = new NvVisitor();
	    	  nv.visit(n, null);
	      }

	    }
	    catch (Exception e)
	    {
	      System.out.println("Oops.");
	      System.out.println(e.getMessage());
	      //e.printStackTrace();
	    }
	  }
}
