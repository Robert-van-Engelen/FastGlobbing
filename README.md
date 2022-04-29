# Fast String Matching with Wildcards, Globs, and Gitignore-Style Globs

Wildcard string matching and globbing isn’t as trivial as it may seem at first.  In fact, mistakes in the past resulted in serious vulnerabilities such as denial of service due to exponential blow-up in execution time.

This fast non-recursive algorithm performs safe gitignore-style wildcard matching.  Implementations in C, C++, Java, Javascript and Python are included.

Please read my CodeProject article ["Fast String Matching with Wildcards, Globs, and Gitignore-Style Globs - How Not to Blow it Up"](https://www.codeproject.com/Articles/5163931/Fast-String-Matching-with-Wildcards-Globs-and-Giti?msg=5870295#xx5870295xx) for all the gory details.

The fast globbing method is used by the [ugrep](https://github.com/Genivia/ugrep) fast file search tool to match file and directory names.

## Gitignore-style globbing rules

| pattern | matches
| ------- | -------
| `*`     | anything except a `/`
| `?`     | any one character except a `/`
| `[a-z]` | one character in the selected range of characters
| `[^a-z]`| one character not in the selected range of characters
| `[!a-z]`| one character not in the selected range of characters
| `/`     | used at the begin of a glob, matches if pathname has no `/`
| `**/`   | zero or more directories
| `/**`   | when at the end of a glob, matches everything after the `/`
| `\?`    | a `?` (or any character specified after the backslash)
| `~`     | the user's HOME directory (C/C++ versions with TILDE enabled)

## Examples

| pattern    | matches
| ---------- | -------
| `*`        | `a`, `b`, `x/a`, `x/y/b`         
| `a`        | `a`, `x/a`, `x/y/a` but not `b`, `x/b`, `a/a/b`
| `/*`       | `a`, `b` but not `x/a`, `x/b`, `x/y/a`
| `/a`       | `a` but not `x/a`, `x/y/a`
| `a?b`      | `axb`, `ayb` but not `a`, `b`, `ab`, `a/b`
| `a[xy]b`   | `axb`, `ayb` but not `a`, `b`, `azb`
| `a[a-z]b`  | `aab`, `abb`, `acb`, `azb` but not `a`, `b`, `a3b`, `aAb`, `aZb`
| `a[^xy]b`  | `aab`, `abb`, `acb`, `azb` but not `a`, `b`, `axb`, `ayb`
| `a[^a-z]b` | `a3b`, `aAb`, `aZb` but not `a`, `b`, `aab`, `abb`, `acb`, `azb`
| `a/*/b`    | `a/x/b`, `a/y/b` but not `a/b`, `a/x/y/b`
| `**/a`     | `a`, `x/a`, `x/y/a` but not `b`, `x/b`
| `a/**/b`   | `a/b`, `a/x/b`, `a/x/y/b` but not `x/a/b`, `a/b/x`
| `a/**`     | `a/x`, `a/y`, `a/x/y` but not `a`, `b/x`
| `a\?b`     | `a?b` but not `a`, `b`, `ab`, `axb`, `a/b`

