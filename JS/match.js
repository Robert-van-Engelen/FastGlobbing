// File:        match.js
// Author:      Robert A. van Engelen, engelen@genivia.com
// Date:        September 12, 2019
// License:     The Code Project Open License (CPOL)
//              https://www.codeproject.com/info/cpol10.aspx
//
// Usage with Node.js REPL:
//
//      $ node
//      > var g = require("./match");
//      > g.test_match("abc", "a*");

// set to true to enable dotglob: *. ?, and [] match a . (dotfile) at the begin or after each /
exports.dotglob = true;

// note: no case-insensitive glob matching option implemented in this version
exports.nocaseglob = false;

// wildcard matching with * and ?
exports.match = function (text, wild) {
  var i = 0;
  var j = 0;
  var n = text.length;
  var m = wild.length;
  var text_backup = -1;
  var wild_backup = -1;
  while (i < n) {
    if (j < m && wild.charAt(j) === "*") {
      // new star-loop: backup positions in pattern and text
      text_backup = i;
      wild_backup = ++j;
    } else if (j < m && (wild.charAt(j) === "?" || wild.charAt(j) === text.charAt(i))) {
      // ? matched any character or we matched the current non-NUL character
      i++;
      j++;
    } else {
      // if no stars we fail to match
      if (wild_backup === -1)
        return false;
      // star-loop: backtrack to the last * by restoring the backup positions in the pattern and text
      i = ++text_backup;
      j = wild_backup;
    }
  }
  // ignore trailing stars
  while (j < m && wild.charAt(j) === "*")
    j++;
  // at end of text means success if nothing else is left to match
  return j >= m;
};

// glob matching with *, ?, and [], use \ to escape meta-characters
exports.glob_match = function (text, glob) {
  var i = 0;
  var j = 0;
  var n = text.length;
  var m = glob.length;
  var text_backup = -1;
  var glob_backup = -1;
  var nodot = !this.dotglob;
  while (i < n) {
    if (j < m) {
      switch (glob.charAt(j)) {
        case "*":
          // match anything except . after /
          if (nodot && text.charAt(i) === '.')
            break;
          // new star-loop: backup positions in pattern and text
          text_backup = i;
          glob_backup = ++j;
          continue;
        case "?":
          // match anything except . after /
          if (nodot && text.charAt(i) === '.')
            break;
          // match anything except /
          if (text.charAt(i) === "/")
            break;
          i++;
          j++;
          continue;
        case "[":
          // match anything except . after /
          if (nodot && text.charAt(i) === '.')
            break;
          // match anything except /
          if (text.charAt(i) === "/")
            break;
          var lastchr;
          var matched = false;
          var reverse = j + 1 < m && (glob.charAt(j + 1) === "^" || glob.charAt(j + 1) === "!");
          // inverted character class
          if (reverse)
            j++;
          // match character class
          for (lastchr = null; ++j < m && glob.charAt(j) !== "]"; lastchr = glob.charCodeAt(j))
            if (lastchr != null && glob.charAt(j) === "-" && j + 1 < m && glob.charAt(j + 1) !== "]" ?
                text.charCodeAt(i) <= glob.charCodeAt(++j) && text.charCodeAt(i) >= lastchr :
                text.charCodeAt(i) === glob.charCodeAt(j))
              matched = true;
          if (matched === reverse)
            break;
          i++;
          if (j < m)
            j++;
          continue;
        case "\\":
          // literal match \-escaped character
          if (j + 1 < m)
            j++;
          // FALLTHROUGH
        default:
          // match the current non-NUL character
          if (glob.charAt(j) !== text.charAt(i))
            break;
          // do not match a . with *, ? [] after /
          nodot = !this.dotglob && glob.charAt(j) === '/';
          i++;
          j++;
          continue;
      }
    }
    if (glob_backup === -1 || text.charAt(text_backup) === "/")
      return false;
    // star-loop: backtrack to the last * but do not jump over /
    i = ++text_backup;
    j = glob_backup;
  }
  // ignore trailing stars
  while (j < m && glob.charAt(j) === "*")
    j++;
  // at end of text means success if nothing else is left to match
  return j >= m;
};

// gitignore-style glob matching with *, **, ?, and [], use \ to escape meta-characters
exports.gitignore_glob_match = function (text, glob) {
  var i = 0;
  var j = 0;
  var n = text.length;
  var m = glob.length;
  var text1_backup = -1;
  var glob1_backup = -1;
  var text2_backup = -1;
  var glob2_backup = -1;
  var nodot = !this.dotglob;
  // match pathname if glob contains a / otherwise match the basename
  if (j + 1 < m && glob.charAt(j) === "/") {
    // if pathname starts with ./ then ignore these pairs
    while (i + 1 < n && text.charAt(i) === "." && text.charAt(i + 1) === "/")
      i += 2;
    // if pathname starts with a / then ignore it
    if (i < n && text.charAt(i) === "/")
      i++;
    j++;
  } else if (glob.indexOf('/') === -1) {
    var sep = text.lastIndexOf('/');
    if (sep !== -1)
      i = sep + 1;
  }
  while (i < n) {
    if (j < m) {
      switch (glob.charAt(j)) {
        case "*":
          // match anything except . after /
          if (nodot && text.charAt(i) === '.')
            break;
          if (++j < m && glob.charAt(j) === "*") {
            // trailing ** match everything after /
            if (++j >= m)
              return true;
            // ** followed by a / match zero or more directories
            if (glob.charAt(j) !== "/")
              return false;
            // new **-loop, discard *-loop
            text1_backup = -1;
            glob1_backup = -1;
            text2_backup = i;
            glob2_backup = j;
            if (text.chatAt(i) === '/')
              j++;
            continue;
          }
          // trailing * matches everything except /
          text1_backup = i;
          glob1_backup = j;
          continue;
        case "?":
          // match anything except . after /
          if (nodot && text.charAt(i) === '.')
            break;
          // match anything except /
          if (text.charAt(i) === "/")
            break;
          i++;
          j++;
          continue;
        case "[":
          // match anything except . after /
          if (nodot && text.charAt(i) === '.')
            break;
          // match anything except /
          if (text.charAt(i) === "/")
            break;
          var lastchr;
          var matched = false;
          var reverse = j + 1 < m && (glob.charAt(j + 1) === "^" || glob.charAt(j + 1) === "!");
          // inverted character class
          if (reverse)
            j++;
          // match character class
          for (lastchr = null; ++j < m && glob.charAt(j) !== "]"; lastchr = glob.charCodeAt(j))
            if (lastchr != null && glob.charAt(j) === "-" && j + 1 < m && glob.charAt(j + 1) !== "]" ?
                text.charCodeAt(i) <= glob.charCodeAt(++j) && text.charCodeAt(i) >= lastchr :
                text.charCodeAt(i) === glob.charCodeAt(j))
              matched = true;
          if (matched === reverse)
            break;
          i++;
          if (j < m)
            j++;
          continue;
        case "\\":
          // literal match \-escaped character
          if (j + 1 < m)
            j++;
          // FALLTHROUGH
        default:
          // match the current non-NUL character
          if (glob.charAt(j) !== text.charAt(i))
            break;
          // do not match a . with *, ? [] after /
          nodot = !this.dotglob && glob.charAt(j) === '/';
          i++;
          j++;
          continue;
      }
    }
    if (glob1_backup !== -1 && text.charAt(text1_backup) !== "/") {
      // *-loop: backtrack to the last * but do not jump over /
      i = ++text1_backup;
      j = glob1_backup;
      continue;
    }
    if (glob2_backup !== -1) {
      // **-loop: backtrack to the last **
      i = ++text2_backup;
      j = glob2_backup;
      continue;
    }
    return false;
  }
  // ignore trailing stars
  while (j < m && glob.charAt(j) === "*")
    j++;
  // at end of text means success if nothing else is left to match
  return j >= m;
};

exports.test_match = function (text, pattern) {
  if (this.match(text, pattern))
    console.log("match                = Y");
  else
    console.log("match                = N");
  if (this.glob_match(text, pattern))
    console.log("glob match           = Y");
  else
    console.log("glob match           = N");
  if (this.gitignore_glob_match(text, pattern))
    console.log("gitignore glob match = Y");
  else
    console.log("gitignore glob match = N");
};
