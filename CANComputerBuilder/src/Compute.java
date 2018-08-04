import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;

import computeparser.ComputeGrammar;
import computeparser.SimpleNode;
import computeparser.visitor.DefinesVisitor;
import computeparser.visitor.NvVisitor;
import computeparser.visitor.PrintVisitor;

public class Compute {

	public static void main(String args []) throws FileNotFoundException
	  {
		InputStream is = null;
		
		if (args.length > 0) {
			System.out.println("Reading from file...");
			is = new FileInputStream(new File(args[0]));
		} else {
			System.out.println("Reading from standard input...");
			is = System.in;
		}
	    
	    new ComputeGrammar(is);
	    System.out.println("Parsing...");
	    try
	    {
	      SimpleNode n = ComputeGrammar.Start();
	      System.out.println("*****Print");
	      PrintVisitor pv = new PrintVisitor();
	      pv.visit(n, null);
	      
	      System.out.println("*****Defines");
	      DefinesVisitor dv = new DefinesVisitor();
	      dv.visit(n, null);
	      
	      System.out.println("*****Nvs");
	      NvVisitor nv = new NvVisitor();
	      nv.visit(n, null);

	    }
	    catch (Exception e)
	    {
	      System.out.println("Oops.");
	      System.out.println(e.getMessage());
	      e.printStackTrace();
	    }
	  }
}
