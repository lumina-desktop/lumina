## Core Components of the Lumina Desktop

It is **strongly-encouraged** that the "core" of Lumina is built and distributed as a single package. Each of the utilities listed here will explicitly require/use each other for essential functionality.

Binaries:
 * `start-lumina-desktop`: (lumina-session directory) Use this to start the desktop session
 * `lumina-desktop`: (lumina-desktop directory) Use this to communicate with the currently-running session
 * `lumina-open`: (lumina-open directory) Use this to launch external applications. Complement to the `xdg-open` utility.
 * `lumina-info`: (lumina-info directory) View basic information about the desktop itself such as version, license, etc.

Other Files that get installed if "core" is built directly:
 * PREFIX/share/icons/material-design-[light/dark]: Basic SVG icon theme.
 * PREFIX/share/lumina-desktop/themes: Built-in themes for the Lumina desktop
 * PREFIX/share/lumina-desktop/colors: Built-in color schemes for the Lumina desktop
 * PREFIX/share/lumina-desktop/menu-scripts: Built-in scripts for creating menu entries on-demand.
 * PREFIX/share/lumina-desktop/globs2: Fallback XDG mimetype database (in case the system does not have one available)

### Library Dependencies

1. Qt 5.4+ (specific modules listed below)
   * concurrent
   * core
   * gui
   * network
   * multimedia
   * multimediawidgets
   * svg
   * widgets
   * x11extras
   * NOTE: Qt5 platform theme and developer libraries are also required to build the Lumina theme engine plugin (the "qtbase5-private-dev" package on Debian/Ubuntu)
2. X11 Libraries
   * libXdamage
3. XCB Libraries
   * libxcb
   * libxcb-ewmh
   * libxcb-image
   * libxcb-composite
   * libxcb-damage
   * libxcb-randr
   * libxcb-util
4. Other Libraries
   * libc

### Runtime Dependencies (utilities)
1. Required Utilities
   * `xinit`: Used by `start-lumina-desktop` for launching an X11 session if needed.
   * `fluxbox`: Used for window management within the desktop session
   * `xscreensaver`: Screensaver/locker for the desktop session
2. Optional (but recommended) Utilities
   * `numlockx`: Used to toggle the numlock key on session start
   * `compton` *or* `xcompmgr`: Compositor to provide cross-application transparency effects and graphical smoothing
   * `xterm`, `qterminal` or some other graphical terminal: XDG standards require the availability of a graphical terminal to launch particular types of applications.
3. OS-specific utilities (optional, but allow for better OS integration)
   1. FreeBSD/TrueOS
      * AppCafe (graphical package manager)
      * SysAdm (system control panel)
      * `pc-sysconfig`, `intel_backlight`, `xbrightness` : screen brightness control utilities
      * `pc-mixer`: graphical front-end to `mixer`
      * `pc-sysconfig`: Allow system into be put into suspend mode (if supported by hardware)
   2. DragonFlyBSD
      * `xbrightness`: screen brightness control (fake brightness - will not change power consumption)
   3. OpenBSD
      * `xbacklight`: screen brightness control
   4. NetBSD
      * [No OS Integrations committed yet]
   5. Generic "Linux"
      * `xbacklight`: screen brightness control
      * `amixer`: Audio mixer control
      * `pavucontrol`: Graphical audio mixer utility
      * `acpi`: Battery detection/status
   6. Slackware Linux
      * `xbacklight`: screen brightness control
      * `amixer`: Audio mixer control
      * `pavucontrol`: Graphical audio mixer utility
      * `acpi`: Battery detection/status
      * `dbus-send`: System shutdown/restart
   7. Debian GNU/kFreeBSD
      * `xbacklight`: screen brightness control
      * `pavucontrol`: Graphical audio mixer utility
   8. Gentoo Linux
      * `xbacklight`: screen brightness control
      * `amixer`: Audio mixer control
      * `pavucontrol`: Graphical audio mixer utility
      * `acpi`: Battery detection/status
      * `dbus-send`: System shutdown/restart
    9. Debian GNU/Linux
       * `xbacklight`: screen brightness control
       * `amixer`: Audio mixer control
       * `pavucontrol`: Graphical audio mixer utility
       * `acpi`: Battery detection/status
       * `dbus-send`: System shutdown/restart
