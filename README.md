Lumina
======

This is the official source tree for the Lumina Desktop Environment.

Lumina is a lightweight, plugin-based desktop for Unix-like operating systems. Each user can arrange the whole interface (panels, desktop plugins, menus) as they like, and a system administrator can set a system-wide default layout.

This README covers the **Qt6 version** of Lumina, which lives in [src-qt6/](src-qt6/) and is built with CMake. The older Qt5 tree is still in [src-qt5/](src-qt5/) for platforms that cannot move to Qt6 yet; see [Building the Qt5 tree](#building-the-qt5-tree).

- Website: https://lumina-desktop.org/
- Telegram: ["Lumina Desktop" public channel](https://t.me/luminadesktop)

![](https://github.com/lumina-desktop/lumina-themes/blob/master/artwork/screenshots/Screenshot-2021-10-02-15-55-58.png)

**Contents**

- [What gets built](#what-gets-built)
- [Requirements](#requirements)
- [Installing dependencies on FreeBSD](#installing-dependencies-on-freebsd)
- [Installing dependencies on Linux](#installing-dependencies-on-linux)
- [Building and installing](#building-and-installing)
- [Starting the desktop](#starting-the-desktop)
- [Building the Qt5 tree](#building-the-qt5-tree)
- [Translations](#translations)
- [Bug reports and contributions](#bug-reports-and-contributions)

What gets built
----

| Group | Source | Programs |
|---|---|---|
| Core | [src-qt6/core/](src-qt6/core/) | `start-lumina-desktop`, `lumina-desktop`, `lumina-open`, `lumina-info`, `lumina-pingcursor`, `lumina-checkpass`, the `lthemeengine` Qt platform theme and style plugins, themes, menu scripts |
| Core utilities | [src-qt6/core-utils/](src-qt6/core-utils/) | `lumina-config`, `lumina-xconfig`, `lumina-search`, and the Lumina Control Center (`lcc` plus its `lcc-*` modules, Qt6 tree only) |
| Desktop utilities | [src-qt6/desktop-utils/](src-qt6/desktop-utils/) | `lumina-fm`, `lumina-archiver`, `lumina-fileinfo`, `lumina-mediaplayer`, `lumina-photo`, `lumina-screenshot`, `lumina-sudo`, `lumina-textedit` |
| Icons | [icon-theme/](icon-theme/) | `lumina-icons` and the `material-design-light` / `material-design-dark` icon themes |

Lumina is an X11 desktop. It does not provide its own window manager: the session starts Fluxbox by default (Openbox or another window manager can be selected in the session settings).

Requirements
----

### Build tools

- A C++17 compiler (GCC or Clang)
- CMake 3.22 or newer
- Extra CMake Modules (ECM) 5.88 or newer (the KDE Frameworks 6 version works)
- Make or Ninja
- git, to fetch the source

### Libraries

- **Qt 6.2 or newer**, with these modules:

  | Qt module | CMake components used |
  |---|---|
  | qtbase | Core, Gui, Widgets, Network, Concurrent, DBus, PrintSupport, and the private Gui headers (GuiPrivate, needed by the theme engine) |
  | qtdeclarative | Qml, Quick |
  | qtmultimedia | Multimedia, MultimediaWidgets |
  | qtsvg | Svg, SvgWidgets |
  | qt5compat | Core5Compat |
  | qttools | LinguistTools (`lrelease`, to build the translations) |

- **X11**: libX11, libXi, libXcursor
- **XCB**: libxcb (including its composite, damage, dpms, screensaver, xfixes and xkb extensions; xtest is optional), xcb-util, xcb-util-wm (EWMH and ICCCM), xcb-util-image, xcb-util-keysyms
- **PAM** (libpam), used by `lumina-checkpass`

### Runtime programs

Required:

| Program | Used for |
|---|---|
| `fluxbox` | Default window manager for the session |
| An X server and `xinit` | `start-lumina-desktop` launches an X session with `xinit` when one is not already running |
| `xrandr` | Monitor configuration in `lumina-xconfig` |
| `tar` (bsdtar/libarchive preferred) | Backend for `lumina-archiver` |

Recommended:

| Program | Used for |
|---|---|
| `xscreensaver` | Screensaver and screen locking |
| `picom` (or `compton`, `xcompmgr`) | Compositing: transparency and smoother drawing |
| D-Bus (`dbus-launch`, `dbus-send`) | Session bus; shutdown and restart on several Linux backends |
| `numlockx` | Setting Num Lock at login |
| `xterm` or another graphical terminal | Launching terminal applications |
| `xdg-user-dirs` | Creating the standard user directories at first login |
| An icon theme such as La Capitaine | A fuller icon set than the bundled ones |
| `sudo` | Backend for `lumina-sudo` |

Optional, for OS integration:

| | FreeBSD | Linux |
|---|---|---|
| Screen brightness | `xbrightness` | `xbacklight` |
| Audio volume | `mixer` (base system) | `amixer` (alsa-utils), `pactl`, `pavucontrol` |
| Battery status | `apm`, `sysctl` (base system) | `acpi` |
| Bluetooth (`lcc-bluetooth`) | | `bluetoothctl` (BlueZ) |

Installing dependencies on FreeBSD
----

Build dependencies:

```sh
pkg install git cmake ninja pkgconf kf6-extra-cmake-modules \
    qt6-base qt6-declarative qt6-multimedia qt6-svg qt6-5compat qt6-tools \
    libX11 libXi libXcursor libxcb \
    xcb-util xcb-util-wm xcb-util-image xcb-util-keysyms
```

PAM and a C++17 compiler (Clang) are part of the base system.

Runtime dependencies:

```sh
pkg install xorg fluxbox xscreensaver picom numlockx xterm \
    dbus xdg-user-dirs xbrightness sudo
```

Installing dependencies on Linux
----

Package names differ between distributions and releases. If one of the names below is not found, search your package manager for the library named in [Requirements](#requirements).


### Fedora (My best guess ATM)

```sh
sudo dnf install git gcc-c++ cmake ninja-build extra-cmake-modules \
    qt6-qtbase-devel qt6-qtbase-private-devel qt6-qtdeclarative-devel \
    qt6-qtmultimedia-devel qt6-qtsvg-devel qt6-qt5compat-devel qt6-qttools-devel \
    libX11-devel libXi-devel libXcursor-devel libxcb-devel \
    xcb-util-devel xcb-util-wm-devel xcb-util-image-devel xcb-util-keysyms-devel \
    pam-devel
```

```sh
sudo dnf install xorg-x11-server-Xorg xorg-x11-xinit fluxbox xscreensaver picom \
    numlockx xterm dbus-x11 xdg-user-dirs bsdtar \
    xbacklight alsa-utils pavucontrol acpi
```

### Arch Linux

```sh
sudo pacman -S --needed base-devel git cmake ninja extra-cmake-modules \
    qt6-base qt6-declarative qt6-multimedia qt6-svg qt6-5compat qt6-tools \
    libx11 libxi libxcursor libxcb \
    xcb-util xcb-util-wm xcb-util-image xcb-util-keysyms pam
```

```sh
sudo pacman -S --needed xorg-server xorg-xinit xorg-xrandr fluxbox xscreensaver picom \
    numlockx xterm dbus xdg-user-dirs libarchive \
    xorg-xbacklight alsa-utils pavucontrol acpi
```

Building and installing
----

Fetch the source:

```sh
git clone https://github.com/lumina-desktop/lumina
cd lumina
```

### With the build script

[scripts/build-lumina.sh](scripts/build-lumina.sh) configures, builds and installs the Qt6 tree in one step. It uses `sudo` (or `doas`) for the install step when not run as root.

```sh
./scripts/build-lumina.sh
```

By default it builds a `Debug` configuration in `build-lumina-qt6/` and installs to `/usr` on Linux and `/usr/local` on FreeBSD and the other BSDs. Common options:

| Option | Effect |
|---|---|
| `-p`, `--prefix DIR` | Install prefix (default `/usr` on Linux, `/usr/local` elsewhere) |
| `-t`, `--type TYPE` | `CMAKE_BUILD_TYPE` (default `Debug`; use `Release` for daily use) |
| `-B`, `--build DIR` | Build directory |
| `-j`, `--jobs N` | Parallel jobs (default: CPU count) |
| `-c`, `--clean` | Wipe the build directory first |
| `-b`, `--branch NAME` | Check out this branch before building |
| `--pull` | `git pull --ff-only` before building |
| `--no-install` | Build only |
| `--qt5` | Build the Qt5 tree instead |

Run it with `--help` for the full list. The complete output is saved to `build.log` in the build directory.

### With CMake directly

The Qt6 tree is built by default, and the install prefix defaults to `/usr` on Linux and `/usr/local` on FreeBSD and the other BSDs:

```sh
cmake -S . -B build-lumina-qt6 -DCMAKE_BUILD_TYPE=Release
cmake --build build-lumina-qt6 --parallel
sudo cmake --install build-lumina-qt6
```

Configuration options:

| Option | Default | Meaning |
|---|---|---|
| `WITH_QT6` | `ON` | `ON` builds [src-qt6/](src-qt6/) against Qt6; `OFF` builds [src-qt5/](src-qt5/) against Qt5 |
| `CMAKE_INSTALL_PREFIX` | `/usr` on Linux, `/usr/local` elsewhere | Where Lumina is installed. On Linux, do not use `/usr/local`: display managers and Qt will not find the session entry and other installed files there |
| `L_ETCDIR` | `/etc` on Linux with the `/usr` prefix; `<prefix>/etc` on the BSDs | Directory for Lumina's system-wide configuration files |
| `CMAKE_BUILD_TYPE` | none | `Release`, `Debug`, `RelWithDebInfo` |

The operating system is detected at configure time and selects the matching OS backend (FreeBSD, DragonFly, OpenBSD, NetBSD, or on Linux: Gentoo, Slackware, Void, with a generic Linux backend for everything else).

### Install locations outside the prefix

Two parts of the install do not follow `CMAKE_INSTALL_PREFIX`, so the install step always needs root:

- The `lthemeengine` platform theme and style plugins are installed into Qt's own plugin directory (as reported by `qtpaths --plugin-dir`), because Qt only loads plugins from there.
- The session script is installed as `/etc/X11/Sessions/lumina`.

Starting the desktop
----

- **From a display manager:** log out and choose "Lumina" in the session list. The session entry is installed to `<prefix>/share/xsessions/Lumina-DE.desktop`.
- **From a console:** run `start-lumina-desktop`. It starts X through `xinit` if no X session is running.
- **From `~/.xinitrc`:** add `exec start-lumina-desktop`.

Building the Qt5 tree
----

[src-qt5/](src-qt5/) is the Qt5-only source, kept for platforms that cannot yet use Qt6. Build it by passing `-DWITH_QT6=OFF` to CMake (or `--qt5` to the build script); it needs Qt 5.15 with the X11Extras and ThemeSupport modules in place of Core5Compat. It can also still be built with qmake using [lumina.pro](lumina.pro).

Translations
----

Translations are kept in this repository, in the `i18n/` directory of each program, and are built and installed together with the binaries.

To contribute a translation, edit the relevant `i18n/*.ts` files and open a pull request.

Bug reports and contributions
----

Please open a ticket on the [GitHub issue tracker](https://github.com/lumina-desktop/lumina/issues). Patches and other contributions are welcome as GitHub pull requests.

Lumina is released under the [BSD 3-Clause license](LICENSE) with some code under the MIT.
