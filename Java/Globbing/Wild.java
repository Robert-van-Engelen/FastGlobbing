// File:        Wild.java
// Author:      Robert A. van Engelen, engelen@genivia.com
// Date:        September 12, 2019
// License:     The Code Project Open License (CPOL)
//              https://www.codeproject.com/info/cpol10.aspx

package Globbing;

public class Wild {
  // wildcard matching with * and ?
  public static boolean match(String text, String wild)
  {
    int i = 0;
    int j = 0;
    int n = text.length();
    int m = wild.length();
    int text_backup = -1;
    int wild_backup = -1;
    while (i < n)
    {
      if (j < m && wild.charAt(j) == '*')
      {
        // new star-loop: backup positions in pattern and text
        text_backup = i;
        wild_backup = ++j;
      }
      else if (j < m && (wild.charAt(j) == '?' || wild.charAt(j) == text.charAt(i)))
      {
        // ? matched any character or we matched the current non-NUL character
        i++;
        j++;
      }
      else
      {
        // if no stars we fail to match
        if (wild_backup == -1)
          return false;
        // star-loop: backtrack to the last * by restoring the backup positions in the pattern and text
        i = ++text_backup;
        j = wild_backup;
      }
    }
    // ignore trailing stars
    while (j < m && wild.charAt(j) == '*')
      j++;
    // at end of text means success if nothing else is left to match
    return j >= m;
  }
}
