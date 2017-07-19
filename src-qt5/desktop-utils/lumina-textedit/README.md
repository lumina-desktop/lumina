## Plaintext Editor for the Lumina Desktop

This is a plaintext editor with a number of advanced features:

Key Features:
 * Multiple file support (one file per tab)
 * Find/Replace support
 * Flexible Interface
    * Tabs can be moved to any edge of the window
    * Line numbering support
    * Line-wrap support
    * Change text font/size independently from the file itself
    * Colors used for highlighting support can be easily customized
 * Syntax Highlighting Support
    * Simple JSON file manifest for support rules
    * Automatic document settings
       * Monospaced fonts on/off
       * Character limit per line
       * Highlight excess whitespace on the end of lines
       * Enable line-wrap by default on/off

Syntax files:
 * File format can be found in the [syntax_support/README.md](https://github.com/trueos/lumina/blob/master/src-qt5/desktop-utils/lumina-textedit/syntax_rules/README.md) file.
 * Syntax files are installed in PREFIX/share/lumina-desktop/syntax_rules with a ".syntax" suffix.
 * Additional files can be provided/installed by the user within the "~/.config/lumina-desktop/syntax_rules" directory.

### Library Dependencies

1. Qt 5.2+ (specific modules listed below)
   * concurrent
   * core
   * gui
   * network
   * multimedia
   * svg
   * widgets
2. Other Libraries
   * libc


### Command Line Arguments

* Any files listed after `lumina-texteditor` will open up new tabs of the editor
 * Example : `lumina-textedit ./test1.txt ./test2.txt ./test3.txt`
* Wildcards are allowed to open multiple files in any path
 * Example : `lumina-textedit ./testdir/*
* `lte` exists as a symlink for `lumina-textedit` for convenience
 

