## Streaming Media Player for the Lumina Desktop

This is a media player for local files with an emphasis on supporting web-based streaming services in a modular fashion.

### Library Dependencies

1. Qt 5.7+ (specific modules listed below)
   * concurrent
   * core
   * gui
   * network
   * multimedia
   * multimediawidgets
   * svg
   * widgets
2. Other Libraries
   * libc

## Runtime Dependencies (optional)
* `pianobar`: Allow streaming from the "Pandora" online radio service

### Command Line Arguments

* Any files listed after `lumina-mediaplayer` will be added to the playlist
 * Example : `lumina-mediaplayer ./test1.mp3 ./test2.mp3 ./test3.mp3`
* Wildcards are allowed to open multiple files in any path
 * Example : `lumina-mediaplayer ./testdir/*

