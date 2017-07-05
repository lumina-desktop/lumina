## Archive Manager for the Lumina Desktop

This is a graphical archive manager based around the `tar` utility, which also provides limited support to copying USB images to a USB device via the `dd`utility.

Binaries:
 * `lumina-archiver`: Graphical utility for configuring the desktop session

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

### Runtime Utilities

1. Required utilities
   * `tar`: Used for the backend archive control system
   * `dd`: Used to copy an image file ("*.img") to a USB device
