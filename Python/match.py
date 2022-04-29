# File:        match.py
# Author:      Robert A. van Engelen, engelen@genivia.com
# Date:        September 17, 2019
# License:     The Code Project Open License (CPOL)
#              https://www.codeproject.com/info/cpol10.aspx
#
# See also: Python modules fnmatch and glob
#
# Usage with Python 2.7 and 3:
#
#       $ python
#       >>> exec(open('match.py').read())
#       >>> match("abc", "a*")
#       >>> glob_match("abc", "a*")
#       >>> gitignore_glob_match("abc", "a*")
#
# It is suggested to create a class with one or more of the three functions listed below

# set to true to enable dotglob: *. ?, and [] match a . (dotfile) at the begin or after each /
dotglob = True

# note: no case-insensitive glob matching option implemented in this version
nocaseglob = False

# wildcard matching with * and ?
def match(text, wild):
    i = 0
    j = 0
    n = len(text)
    m = len(wild)
    text_backup = -1
    wild_backup = -1
    while i < n:
        if j < m and wild[j] == '*':
            # new star-loop: backup positions in pattern and text
            text_backup = i
            j += 1
            wild_backup = j
        elif j < m and (wild[j] == '?' or wild[j] == text[i]):
            # ? matched any character or we matched the current non-NUL character
            i += 1
            j += 1
        else:
            # if no stars we fail to match
            if wild_backup == -1:
                return False
            # star-loop: backtrack to the last * by restoring the backup positions in the pattern and text
            text_backup += 1
            i = text_backup
            j = wild_backup
    # ignore trailing stars
    while j < m and wild[j] == '*':
        j += 1
    # at end of text means success if nothing else is left to match
    return j >= m

# glob matching with *, ?, and [], use \ to escape meta-characters
def glob_match(text, glob):
    i = 0
    j = 0
    n = len(text)
    m = len(glob)
    text_backup = -1
    glob_backup = -1
    nodot = not dotglob
    while i < n:
        if j < m:
            if glob[j] == '*':
                # match anything except . after /
                if not nodot or text[i] != '.':
                    # new star-loop: backup positions in pattern and text
                    text_backup = i
                    j += 1
                    glob_backup = j
                    continue
            elif glob[j] == '?':
                # match anything except . after /
                if not nodot or text[i] != '.':
                    # match anything except /
                    if text[i] != '/':
                        i += 1
                        j += 1
                        continue
            elif glob[j] == '[':
                # match anything except . after /
                if not nodot or text[i] != '.':
                    # match anything except /
                    if text[i] != '/':
                        j += 1
                        # inverted character class
                        reverse = j < m and (glob[j] == '^' or glob[j] == '!')
                        if reverse:
                            j += 1
                        # match character class
                        lastchr = False
                        matched = False
                        while j < m and glob[j] != ']':
                            if lastchr != False and glob[j] == '-' and j + 1 < m and glob[j + 1] != ']':
                                j += 1
                                if text[i] <= glob[j] and text[i] >= lastchr:
                                    matched = True
                            elif text[i] == glob[j]:
                                matched = True
                            lastchr = glob[j]
                            j += 1
                        if matched != reverse:
                            i += 1
                            if j < m:
                                j += 1
                            continue
            elif glob[j] == '\\':
                # literal match \-escaped character
                if j + 1 < m:
                    j += 1
                # match the current non-NUL character
                if glob[j] == text[i]:
                    # do not match a . with *, ? [] after /
                    nodot = not dotglob and glob[j] == '/'
                    i += 1
                    j += 1
                    continue
            else:
                # match the current non-NUL character
                if glob[j] == text[i]:
                    # do not match a . with *, ? [] after /
                    nodot = not dotglob and glob[j] == '/'
                    i += 1
                    j += 1
                    continue
        if glob_backup == -1 or text[text_backup] == '/':
            return False
        # star-loop: backtrack to the last * but do not jump over /
        text_backup += 1
        i = text_backup
        j = glob_backup
    # ignore trailing stars
    while j < m and glob[j] == '*':
        j += 1
    # at end of text means success if nothing else is left to match
    return j >= m

# gitignore-style glob matching with *, **, ?, and [], use \ to escape meta-characters
def gitignore_glob_match(text, glob):
    i = 0
    j = 0
    n = len(text)
    m = len(glob)
    text1_backup = -1
    glob1_backup = -1
    text2_backup = -1
    glob2_backup = -1
    nodot = not dotglob
    # match pathname if glob contains a / otherwise match the basename
    if j + 1 < m and glob[j] == '/':
        # if pathname starts with ./ then ignore these pairs
        while i + 1 < n and text[i] == '.' and text[i + 1] == '/':
            i += 2
        # if pathname starts with / then ignore it
        if i < n and text[i] == '/':
            i += 1
        j += 1
    elif glob.find('/') == -1:
        sep = text.rfind('/')
        if sep != -1:
            i = sep + 1
    while i < n:
        if j < m:
            if glob[j] == '*':
                # match anything except . after /
                if not nodot or text[i] != '.':
                    j += 1
                    if j < m and glob[j] == '*':
                        j += 1
                        # trailing ** match everything after /
                        if j >= m:
                            return True
                        # ** followed by a / match zero or more directories
                        if glob[j] != '/':
                            return False
                        # new **-loop, discard *-loop
                        text1_backup = -1
                        glob1_backup = -1
                        text2_backup = i
                        j += 1
                        glob2_backup = j
                        continue
                    # trailing * matches everything except /
                    text1_backup = i
                    glob1_backup = j
                    continue
            elif glob[j] == '?':
                # match anything except . after /
                if not nodot or text[i] != '.':
                    # match anything except /
                    if text[i] != '/':
                        i += 1
                        j += 1
                        continue
            elif glob[j] == '[':
                # match anything except . after /
                if not nodot or text[i] != '.':
                    # match anything except /
                    if text[i] != '/':
                        j += 1
                        # inverted character class
                        reverse = j < m and (glob[j] == '^' or glob[j] == '!')
                        if reverse:
                            j += 1
                        # match character class
                        lastchr = False
                        matched = False
                        while j < m and glob[j] != ']':
                            if lastchr != False and glob[j] == '-' and j + 1 < m and glob[j + 1] != ']':
                                j += 1
                                if text[i] <= glob[j] and text[i] >= lastchr:
                                    matched = True
                            elif text[i] == glob[j]:
                                matched = True
                            lastchr = glob[j]
                            j += 1
                        if matched != reverse:
                            i += 1
                            if j < m:
                                j += 1
                            continue
            elif glob[j] == '\\':
                # literal match \-escaped character
                if j + 1 < m:
                    j += 1
                # match the current non-NUL character
                if glob[j] == text[i]:
                    # do not match a . with *, ? [] after /
                    nodot = not dotglob and glob[j] == '/'
                    i += 1
                    j += 1
                    continue
            else:
                # match the current non-NUL character
                if glob[j] == text[i]:
                    # do not match a . with *, ? [] after /
                    nodot = not dotglob and glob[j] == '/'
                    i += 1
                    j += 1
                    continue
        if glob1_backup != -1 and text[text1_backup] != '/':
            # *-loop: backtrack to the last * but do not jump over /
            text1_backup += 1
            i = text1_backup
            j = glob1_backup
            continue
        if glob2_backup != -1:
            # **-loop: backtrack to the last **
            text2_backup += 1
            i = text2_backup
            j = glob2_backup
            continue
        return False
    # ignore trailing stars
    while j < m and glob[j] == '*':
        j += 1
    # at end of text means success if nothing else is left to match
    return j >= m
