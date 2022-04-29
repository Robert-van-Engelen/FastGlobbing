// File:        Glob.java
// Author:      Robert A. van Engelen, engelen@genivia.com
// Date:        September 12, 2019
// License:     The Code Project Open License (CPOL)
//              https://www.codeproject.com/info/cpol10.aspx

package Globbing;

public class Glob {
  // dotglob: *. ?, and [] match a . (dotfile) at the begin or after each /
  public static boolean dotglob = true;
  // enable case-insensitive glob matching
  public static boolean nocaseglob = false;
  // case-insensitive matching
  private static char icase(char c)
  {
    return nocaseglob ? Character.toLowerCase(c) : c;
  }
  // glob matching with *, ?, and [], use \ to escape meta-characters
  public static boolean match(String text, String glob)
  {
    int i = 0;
    int j = 0;
    int n = text.length();
    int m = glob.length();
    int text_backup = -1;
    int glob_backup = -1;
    boolean nodot = !dotglob;
    while (i < n)
    {
      if (j < m)
      {
        switch (glob.charAt(j))
        {
          case '*':
            // no dotglob: match anything except . after /
            if (nodot && text.charAt(i) == '.')
              break;
            // new star-loop: backup positions in pattern and text
            text_backup = i;
            glob_backup = ++j;
            continue;
          case '?':
            // no dotglob: match anything except . after /
            if (nodot && text.charAt(i) == '.')
              break;
            // match any character except /
            if (text.charAt(i) == '/')
              break;
            i++;
            j++;
            continue;
          case '[':
          {
            // no dotglob: match anything except . after /
            if (nodot && text.charAt(i) == '.')
              break;
            // match any character except /
            if (text.charAt(i) == '/')
              break;
            char lastchr;
            boolean matched = false;
            boolean reverse = j + 1 < m && (glob.charAt(j + 1) == '^' || glob.charAt(j + 1) == '!');
            // inverted character class
            if (reverse)
              j++;
            // match character class
            for (lastchr = 256; ++j < m && glob.charAt(j) != ']'; lastchr = icase(glob.charAt(j)))
              if (lastchr < 256 && glob.charAt(j) == '-' && j + 1 < m && glob.charAt(j + 1) != ']' ?
                  icase(text.charAt(i)) <= icase(glob.charAt(++j)) && icase(text.charAt(i)) >= icase(lastchr) :
                  icase(text.charAt(i)) == icase(glob.charAt(j)))
                matched = true;
            if (matched == reverse)
              break;
            i++;
            if (j < m)
              j++;
            continue;
          }
          case '\\':
            // literal match \-escaped character
            if (j + 1 < m)
              j++;
            // FALLTHROUGH
          default:
            // match the current non-NUL character
            if (icase(glob.charAt(j)) != icase(text.charAt(i)))
              break;
            nodot = !dotglob && glob.charAt(j) == '/';
            i++;
            j++;
            continue;
        }
      }
      if (glob_backup == -1 || text.charAt(text_backup) == '/')
        return false;
      // star-loop: backtrack to the last * but do not jump over /
      i = ++text_backup;
      j = glob_backup;
    }
    // ignore trailing stars
    while (j < m && glob.charAt(j) == '*')
      j++;
    // at end of text means success if nothing else is left to match
    return j >= m;
  }
}
