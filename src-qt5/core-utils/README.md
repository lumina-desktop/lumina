## Core Utilities of the Lumina Desktop

It is **strongly-encouraged** that the "core-utils" of Lumina are built and distributed as a single package.

Binaries:
 * `lumina-config`: (lumina-config directory) Graphical utility for configuring the desktop session
 * `lumina-search`: (lumina-search directory): Graphical utility to search for applications or files and launch them
 * `lumina-xconfig`: (lumina-xconfig directory: Graphical utility to manage multiple-monitor arrangements

### Library Dependencies

1. Qt 5.2+ (specific modules listed below)
   * concurrent
   * core
   * gui
   * network
   * multimedia
   * svg
   * widgets
   * x11extras
2. Other Libraries
   * libc

### Runtime Utilities

1. Required utilities
   * `xrandr`	: Used for multi-monitor controls in `lumina-xconfig`
