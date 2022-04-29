// File:        Gitglob.java
// Author:      Robert A. van Engelen, engelen@genivia.com
// Date:        September 12, 2019
// License:     The Code Project Open License (CPOL)
//              https://www.codeproject.com/info/cpol10.aspx

package Globbing;

public class Gitglob {
  // dotglob: *. ?, and [] match a . (dotfile) at the begin or after each /
  public static boolean dotglob = true;
  // enable case-insensitive glob matching
  public static boolean nocaseglob = false;
  // case-insensitive matching
  private static char icase(char c)
  {
    return nocaseglob ? Character.toLowerCase(c) : c;
  }
  // gitignore-style glob matching with *, **, ?, and [], use \ to escape meta-characters
  public static boolean match(String text, String glob)
  {
    int i = 0;
    int j = 0;
    int n = text.length();
    int m = glob.length();
    int text1_backup = -1;
    int glob1_backup = -1;
    int text2_backup = -1;
    int glob2_backup = -1;
    boolean nodot = !dotglob;
    // match pathname if glob contains a / otherwise match the basename
    if (j + 1 < m && glob.charAt(j) == '/')
    {
      // if pathname starts with ./ then ignore these pairs
      while (i + 1 < n && text.charAt(i) == '.' && text.charAt(i + 1) == '/')
        i += 2;
      // if pathname starts with / then ignore it
      if (i < n && text.charAt(i) == '/')
        i++;
      j++;
    }
    else if (glob.indexOf('/') == -1)
    {
      int sep = text.lastIndexOf('/');
      if (sep != -1)
        i = sep + 1;
    }
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
            if (++j < m && glob.charAt(j) == '*')
            {
              // trailing ** match everything after /
              if (++j >= m)
                return true;
              // ** followed by a / match zero or more directories
              if (glob.charAt(j) != '/')
                return false;
              // new **-loop, discard *-loop
              text1_backup = -1;
              glob1_backup = -1;
              text2_backup = i;
              glob2_backup = ++j;
              continue;
            }
            // trailing * matches everything except /
            text1_backup = i;
            glob1_backup = j;
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
              if (lastchr < 256 && glob.charAt(j) == '-' && j + 1 < m  && glob.charAt(j + 1) != ']' ?
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
      if (glob1_backup != -1 && text.charAt(text1_backup) != '/')
      {
        // *-loop: backtrack to the last * but do not jump over /
        i = ++text1_backup;
        j = glob1_backup;
        continue;
      }
      if (glob2_backup != -1)
      {
        // **-loop: backtrack to the last **
        i = ++text2_backup;
        j = glob2_backup;
        continue;
      }
      return false;
    }
    // ignore trailing stars
    while (j < m && glob.charAt(j) == '*')
      j++;
    // at end of text means success if nothing else is left to match
    return j >= m;
  }
}
