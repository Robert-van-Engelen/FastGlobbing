# Fast String Matching with Wildcards, Globs, and Gitignore-Style Globs

Wildcard string matching and globbing isn’t as trivial as it may seem at first. In fact, mistakes in the past resulted in serious vulnerabilities such as denial of service. Simple patches, such as limiting the number of matches to limit the CPU time, have been applied to fix implementations that suffer exponential blow-up in execution time.

This non-recursive algorithm performs safe and fast gitignore-style wildcard matching.  Implementations in C, Java, Javascript and Python are included.

please read my CodeProject article ["Fast String Matching with Wildcards, Globs, and Gitignore-Style Globs - How Not to Blow it Up"](https://www.codeproject.com/Articles/5163931/Fast-String-Matching-with-Wildcards-Globs-and-Giti?msg=5870295#xx5870295xx) for all the details.

