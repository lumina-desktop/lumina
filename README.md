lumina
======

The official source tree for the Lumina Desktop Environment.

Description
=====

Lumina-DE is a lightweight, BSD licensed desktop environment designed specifically for use on FreeBSD (although it should be easily portable to other Unix-like OS's as well).

Latest Stable Versions
----
Qt4 - 0.7.2 (Beta): Use the qt4/0.7.2 branch of the source tree to fetch this version.

Qt5 - 0.8.0 (Beta): Use the qt5/0.8.0 branch of the source tree to fetch this version.

The master branch is where all development is performed and tested before the next release version.

Translations
----
All the translation files for Lumina are available through the PC-BSD pootle translations systems, which are then auto-committed to the pcbsd/lumina-i18n repo (Qt5+ only). This provides an easy interface through which to contribute translations while also ensuring that the translation files are kept in sync with the Lumina source files, and it is highly recommended that translators use this interface.

Translation Website URL: http://translate.pcbsd.org/

How to file bug reports or feature requests
----
Please submit any bug reports or feature requests through the PC-BSD bug tracker, as this ensures that your submissions will be addressed in a timely manner (developers on GitHub may also create a ticket through the GitHub issues tracker, although it is not as feature-full as the PC-BSD bug tracker). If you want to send in patches or other source contributions, please send in a GitHub pull request so that it can get reviewed/committed to the main repo as quickly as possible.

Bug Tracker: https://bugs.pcbsd.org/projects/pcbsd

How to build from source
----
1) Checkout the source repo to your local box (GitHub gives a few methods for this)

2) In a terminal, change to the lumina repo directory (base dir, not one of the sub-projects)
 - (Linux Note) If there is a customized OS template for your particular distro (not the general "Linux" template), you will need to change into the libLumina subdirectory and run the "make-linux-distro.sh" script to modify the project file for the Lumina library before running "qmake" in step 3. Be sure to change back to the base Lumina directory before moving on to step 3.
Example usage: "./make-linux-distro.sh MYDISTRO" (where there is a LuminaOS-MYDISTRO.cpp template available).

3) Run "qmake" on your local system to turn all the Qt project files (*.pro) into Makefiles
 - NOTE: If any of the Qt project files is modified (either by you or from updating the Lumina source tree), you will need to be sure to repeat this step to place those changes into your local Makefiles (otherwise you may experience strange build failures).
 - NOTE 2: The Qt5 version of qmake is located in /usr/local/lib/qt5/bin/qmake on FreeBSD, other OS's may have slightly different locations or names for qmake (such as qmake-qt5 for example)
 - NOTE 3: You can set a customized install PREFIX, LIBPREFIX, and QT5LIBDIR variables via the qmake command. PREFIX determines the base directory to install Lumina ("/usr/local" by default), LIBPREFIX determines the location to install/use libraries ("PREFIX/lib" by default), and QT5LIBDIR determines the location of the Qt5 libraries ("LIBPREFIX/qt5" by default). The syntax to set these custom values when running qmake is usually: "qmake PREFIX=/some/prefix LIBPREFIX=/some/lib/prefix QT5LIBDIR=/main/qt5/library/dir"

4) Run "make" to compile all the Lumina projects (can be done as user)

5) Run "make install" to install the Lumina desktop on your local system (requires admin/root privileges usually)

6-optional) Run "make clean" to clean up all the temporary build files in the Lumina source tree.
