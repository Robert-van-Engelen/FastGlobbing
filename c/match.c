// File:        match.c
// Author:      Robert A. van Engelen, engelen@genivia.com
// Date:        August 5, 2019
// License:     The Code Project Open License (CPOL)
//              https://www.codeproject.com/info/cpol10.aspx

#include <stdio.h>
#include <string.h>
#include <ctype.h>

// set to 1 to enable ~ (tilde) expansion using the HOME environment variable
#define TILDE 1

#if TILDE
#include <stdlib.h> // getenv()
#endif

// set to 1 to enable dotglob: *. ?, and [] match a . (dotfile) at the begin or after each /
#define DOTGLOB 1

// set to 1 to enable case-insensitive glob matching
#define NOCASEGLOB 0

#define CASE(c) (NOCASEGLOB ? tolower(c) : (c))

// Windows \ versus normal / path separator
#ifdef OS_WIN
#define PATHSEP '\\'
#else
#define PATHSEP '/'
#endif

enum { FALSE = 0, TRUE = 1, ABORT = 2 };

// returns TRUE if text string matches wild pattern with * and ?
int naive_recursive_match(const char *text, const char *wild)
{
  while (*text != '\0')
  {
    if (*wild == '*')
    {
      // any number of stars act as one star
      while (*++wild == '*')
        continue;
      // a star at the end of the pattern matches any text
      if (*wild == '\0')
        return TRUE;
      // star-loop: match the rest of the pattern and text
      while (naive_recursive_match(text, wild) == FALSE && *text != '\0')
        text++;
      return *text != '\0' ? TRUE : FALSE;
    }
    // ? matches any character or we match the current non-NUL character
    if (*wild != '?' && *wild != *text)
      return FALSE;
    text++;
    wild++;
  }
  // ignore trailing stars
  while (*wild == '*')
    wild++;
  // at end of text means success if nothing else is left to match
  return *wild == '\0' ? TRUE : FALSE;
}

// returns TRUE if text string matches wild pattern with * and ?
int recursive_match(const char *text, const char *wild)
{
  while (*text != '\0')
  {
    if (*wild == '*')
    {
      int ret;
      // any number of stars act as one star
      while (*++wild == '*')
        continue;
      // a star at the end of the pattern matches any text
      if (*wild == '\0')
        return TRUE;
      // star-loop: match the rest of the pattern and text
      while ((ret = recursive_match(text, wild)) == FALSE && *text != '\0')
        text++;
      // ABORT recursion when failing the last star-loop
      if (ret != TRUE)
        return ABORT;
      return *text != '\0' ? TRUE : FALSE;
    }
    // ? matches any character or we match the current non-NUL character
    if (*wild != '?' && *wild != *text)
      return FALSE;
    text++;
    wild++;
  }
  // ignore trailing stars
  while (*wild == '*')
    wild++;
  // at end of text means success if nothing else is left to match
  return *wild == '\0' ? TRUE : FALSE;
}

// returns TRUE if text string matches wild pattern with * and ?
int match(const char *text, const char *wild)
{
  const char *text_backup = NULL;
  const char *wild_backup = NULL;
  while (*text != '\0')
  {
    if (*wild == '*')
    {
      // new star-loop: backup positions in pattern and text
      text_backup = text;
      wild_backup = ++wild;
    }
    else if (*wild == '?' || *wild == *text)
    {
      // ? matched any character or we matched the current non-NUL character
      text++;
      wild++;
    }
    else
    {
      // if no stars we fail to match
      if (wild_backup == NULL)
        return FALSE;
      // star-loop: backtrack to the last * by restoring the backup positions in the pattern and text
      text = ++text_backup;
      wild = wild_backup;
    }
  }
  // ignore trailing stars
  while (*wild == '*')
    wild++;
  // at end of text means success if nothing else is left to match
  return *wild == '\0' ? TRUE : FALSE;
}

// returns TRUE if text string matches glob-like pattern with * and ?
int globly_match(const char *text, const char *glob)
{
  const char *text_backup = NULL;
  const char *glob_backup = NULL;
  while (*text != '\0')
  {
    if (*glob == '*')
    {
      // new star-loop: backup positions in pattern and text
      text_backup = text;
      glob_backup = ++glob;
    }
    else if ((*glob == '?' && *text != '/') || *glob == *text)
    {
      // ? matched any character except /, or we matched the current non-NUL character
      text++;
      glob++;
    }
    else
    {
      if (glob_backup == NULL || *text_backup == '/')
        return FALSE;
      // star-loop: backtrack to the last * but do not jump over /
      text = ++text_backup;
      glob = glob_backup;
    }
  }
  // ignore trailing stars
  while (*glob == '*')
    glob++;
  // at end of text means success if nothing else is left to match
  return *glob == '\0' ? TRUE : FALSE;
}

// returns TRUE if text string matches glob pattern with * and ?
int glob_match(const char *text, const char *glob)
{
  const char *text_backup = NULL;
  const char *glob_backup = NULL;
  int nodot = !DOTGLOB;
  while (*text != '\0')
  {
    switch (*glob)
    {
      case '*':
        // match anything except . after /
        if (nodot && *text == '.')
          break;
        // new star-loop: backup positions in pattern and text
        text_backup = text;
        glob_backup = ++glob;
        continue;
      case '?':
        // match anything except . after /
        if (nodot && *text == '.')
          break;
        // match any character except /
        if (*text == PATHSEP)
          break;
        text++;
        glob++;
        continue;
      case '[':
      {
        int lastchr;
        int matched;
        int reverse;
        // match anything except . after /
        if (nodot && *text == '.')
          break;
        // match any character in [...] except /
        if (*text == PATHSEP)
          break;
        // inverted character class
        reverse = glob[1] == '^' || glob[1] == '!' ? TRUE : FALSE;
        if (reverse)
          glob++;
        // match character class
        matched = FALSE;
        for (lastchr = 256; *++glob != '\0' && *glob != ']'; lastchr = CASE(*glob))
          if (lastchr < 256 && *glob == '-' && glob[1] != ']' && glob[1] != '\0' ?
              CASE(*text) <= CASE(*++glob) && CASE(*text) >= lastchr :
              CASE(*text) == CASE(*glob))
            matched = TRUE;
        if (matched == reverse)
          break;
        text++;
        if (*glob != '\0')
          glob++;
        continue;
      }
      case '\\':
        // literal match \-escaped character
        glob++;
        // FALLTHROUGH
      default:
        // match the current non-NUL character
        if (CASE(*glob) != CASE(*text) && !(*glob == '/' && *text == PATHSEP))
          break;
        // do not match a . with *, ? [] after /
        nodot = !DOTGLOB && *glob == '/';
        text++;
        glob++;
        continue;
    }
    if (glob_backup == NULL || *text_backup == PATHSEP)
      return FALSE;
    // star-loop: backtrack to the last * but do not jump over /
    text = ++text_backup;
    glob = glob_backup;
  }
  // ignore trailing stars
  while (*glob == '*')
    glob++;
  // at end of text means success if nothing else is left to match
  return *glob == '\0' ? TRUE : FALSE;
}

// returns TRUE if text string matches gitignore-style glob pattern
int gitignore_glob_match(const char *text, const char *glob)
{
  const char *text1_backup = NULL;
  const char *glob1_backup = NULL;
  const char *text2_backup = NULL;
  const char *glob2_backup = NULL;
  int nodot = !DOTGLOB;
#if TILDE
  // expand ~ (requires the HOME environment variable)
  if (*glob == '~')
  {
    const char *home = getenv("HOME");
    size_t len;
    if (home == NULL)
      return FALSE;
    if (glob[1] == '/' || glob[1] == '\0')
    {
      len = strlen(home);
    }
    else
    {
      const char *sep = strrchr(home, '/');
      len = sep == NULL ? 0 : sep - home + 1;
    }
    if (strncmp(home, text, len) != 0)
      return FALSE;
    text += len;
    glob++;
  }
  else
#endif
  // match pathname if glob contains a / otherwise match the basename
  if (*glob == '/')
  {
    // if pathname starts with ./ then ignore these pairs
    while (*text == '.' && text[1] == PATHSEP)
      text += 2;
    // if pathname starts with a / then ignore it
    if (*text == PATHSEP)
      text++;
    glob++;
  }
  else if (strchr(glob, '/') == NULL)
  {
    const char *sep = strrchr(text, PATHSEP);
    if (sep)
      text = sep + 1;
  }
  while (*text != '\0')
  {
    switch (*glob)
    {
      case '*':
        // match anything except . after /
        if (nodot && *text == '.')
          break;
        if (*++glob == '*')
        {
          // trailing ** match everything after /
          if (*++glob == '\0')
            return TRUE;
          // ** followed by a / match zero or more directories
          if (*glob != '/')
            return FALSE;
          // new **-loop, discard *-loop
          text1_backup = NULL;
          glob1_backup = NULL;
          text2_backup = text;
          glob2_backup = glob;
          continue;
        }
        // trailing * matches everything except /
        text1_backup = text;
        glob1_backup = glob;
        continue;
      case '?':
        // match anything except . after /
        if (nodot && *text == '.')
          break;
        // match any character except /
        if (*text == PATHSEP)
          break;
        text++;
        glob++;
        continue;
      case '[':
      {
        int lastchr;
        int matched;
        int reverse;
        // match anything except . after /
        if (nodot && *text == '.')
          break;
        // match any character in [...] except /
        if (*text == PATHSEP)
          break;
        // inverted character class
        reverse = glob[1] == '^' || glob[1] == '!' ? TRUE : FALSE;
        if (reverse)
          glob++;
        // match character class
        matched = FALSE;
        for (lastchr = 256; *++glob != '\0' && *glob != ']'; lastchr = CASE(*glob))
          if (lastchr < 256 && *glob == '-' && glob[1] != ']' && glob[1] != '\0' ?
              CASE(*text) <= CASE(*++glob) && CASE(*text) >= lastchr :
              CASE(*text) == CASE(*glob))
            matched = TRUE;
        if (matched == reverse)
          break;
        text++;
        if (*glob != '\0')
          glob++;
        continue;
      }
      case '\\':
        // literal match \-escaped character
        glob++;
        // FALLTHROUGH
      default:
        // match the current non-NUL character
        if (CASE(*glob) != CASE(*text) && !(*glob == '/' && *text == PATHSEP))
          break;
        // do not match a . with *, ? [] after /
        nodot = !DOTGLOB && *glob == '/';
        text++;
        glob++;
        continue;
    }
    if (glob1_backup != NULL && *text1_backup != PATHSEP)
    {
      // *-loop: backtrack to the last * but do not jump over /
      text = ++text1_backup;
      glob = glob1_backup;
      continue;
    }
    if (glob2_backup != NULL)
    {
      // **-loop: backtrack to the last **
      text = ++text2_backup;
      glob = glob2_backup;
      continue;
    }
    return FALSE;
  }
  // ignore trailing stars
  while (*glob == '*')
    glob++;
  // at end of text means success if nothing else is left to match
  return *glob == '\0' ? TRUE : FALSE;
}

int main(int argc, char **argv)
{
  if (argc > 2)
  {
    printf("naive recursive match = %s\n", naive_recursive_match(argv[1], argv[2]) == TRUE ? "Y" : "N");
    printf("recursive match       = %s\n", recursive_match(      argv[1], argv[2]) == TRUE ? "Y" : "N");
    printf("match                 = %s\n", match(                argv[1], argv[2]) == TRUE ? "Y" : "N");
    printf("globly match          = %s\n", globly_match(         argv[1], argv[2]) == TRUE ? "Y" : "N");
    printf("glob match            = %s\n", glob_match(           argv[1], argv[2]) == TRUE ? "Y" : "N");
    printf("gitignore glob match  = %s\n", gitignore_glob_match( argv[1], argv[2]) == TRUE ? "Y" : "N");
  }
  else
  {
    printf("Usage: match 'string' 'pattern'\n");
  }
}
