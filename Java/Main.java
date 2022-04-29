// File:        Main.java
// Author:      Robert A. van Engelen, engelen@genivia.com
// Date:        September 12, 2019
// License:     The Code Project Open License (CPOL)
//              https://www.codeproject.com/info/cpol10.aspx

import java.lang.*;
import Globbing.*;

public class Main {
  public static void main(String[] arg)
  {
    if (arg.length > 1)
    {
      System.out.println(Globbing.Wild.match(   arg[0], arg[1]) ? "match         = Y" : "match         = N");
      System.out.println(Globbing.Glob.match(   arg[0], arg[1]) ? "glob match    = Y" : "glob match    = N");
      System.out.println(Globbing.Gitglob.match(arg[0], arg[1]) ? "gitglob match = Y" : "gitglob match = N");
    }
    else
    {
      System.out.println("Usage: match 'string' 'pattern'");
    }
  }
}
