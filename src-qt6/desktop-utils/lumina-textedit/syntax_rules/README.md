Information about the syntax highlighting rules:

# General File format
The bulk of the file is a JSON text structure, with the exception of a small comments section at the top where every line starts with a "#".

# Comments
A small comment section may be placed at the top of the file where every line starts with a "#". This is not a part of the JSON format, but instead something that the Lumina text editor will scan for and remove prior to loading the rest of the file as a JSON document.

# Requirements
1. A "meta" object containing the following variables (meta information about the rules):
   1. "name" : The name that will be shown to the user for this set of syntax rules.
   2. If this syntax file is to be automatically applied to particular file type, then at least one of the following options must be set:
      1. "file_suffix" : An array of file extensions which are supported by this syntax rule set (Example: temp.foo will be matched by "file_suffix"=["foo"] )
      2. "file_regex" : A regular expression which should be used to find if the filename matches this rule set.
      3. "first_line_match" : *(only used if no filename rules matched)* Exact match for the first line of text in the file (Example: "#!/bin/sh")
      4. "first_line_regex" : *(only used if no filename rules matched)* Regular expression to use when find a match for the first line of text in the file
2. A "format" object containing the following variables (file-wide formatting):
   1. "columns_per_line" : (integer, optional) For file formats with line-length restrictions, this will automatically highlight/flag any "overage" of the designated limit.
   2. "highlight_whitespace_eol" : (boolian, optional) Highlight any excess whitespace at the end of a line.
   3. "font_type" : (optional) One of the following ["all", "monospace"]. This is for assisting with file formats that need characters to line up within the file (all columns are in the same place per line, etc).
   4. "line_wrap" : (boolian) Automatically enable/disable line wrapping for this type of file
   5. "tab_width" : (integer - 8 by default) Have tabs automatically take up this many characters.
3. A "rules" array containing each of the individual rules (earlier rules are applied before later ones). The required fields for a rule are:
   1. "name" : Not directly used by LTE (yet) - but is useful for noting the purpose of each rule
   2. Exactly **one** of the following options must also be included:
      1. "words" : Array of exact words/text which should be matched (automatically converted to a regular expression with a break on either side of the word)
      2. "regex" : single-line regular expression to be used for finding matching text
      3. "regex_start" **and** "regex_end" : multi-line regular expression. Everything between the start/end matches will be highlighted.
   3. At least **one** of the following fields should also be supplied:
      1. "foreground" : Font color of the matching text (see the Colors section for additional information)
      2. "background" : Highlighting color of the matching text (see the Colors section for additional information)
      3. "font_weight" : One of the following ["bold","normal", "light"]. Changes the thickness of the font for the matching text
      4. "font_style" : One of the following ["italic", "normal"]. Change the style of the font fo the matching text.

# Colors
There are a number of built-in colors which may be defined by the user, and these can be used by passing in the following:
`"colors/[name of color]"`
The currently-valid colors are: ["keyword", "altkeyword", "class", "text", "function", "comment", "bracket-found", "bracket-missing"].

Alternatively, an RGB (0-255) or Hex color code may be used instead (please limit this though - it can conflict with the user's preferred color scheme quite badly)

Examples:
 `"foreground" : "rgb(10,10,255)"`
 `"background" : "colors/text"`
 `"foreground" : "#0F0F0F"`
