Lumina
======

This is the official source tree for the Lumina Desktop Environment.

For additional information about the project, please browse the official website for documentation, screenshots, and more: www.lumina-desktop.org


Description
=====

Lumina-DE is a lightweight, BSD licensed desktop environment designed specifically for use on FreeBSD (although it should be easily portable to other Unix-like OS's as well).

Latest Stable Versions
----
Qt4 - 0.7.2 (Beta): Use the qt4/0.7.2 branch of the source tree to fetch this version.

Qt5 - 0.8.8 (Beta): Use the qt5/0.8.8 branch of the source tree to fetch this version.

The master branch is where all development is performed and tested before the next release version is branched off.

NOTE: Starting with version 0.8.4, you may also find packaged versions of the source code for the individual releases of Lumina here on GitHub. Please go to the "Releases" page here on GitHub for additional information.

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
* To update an exising checkout of the Lumina repo, run "git pull" while within the source directory. 

2) In a terminal, change to the lumina repo directory (base dir, not one of the sub-projects)

3) Run "qmake" on your local system to turn all the Qt project files (*.pro) into Makefiles
 * NOTE 1: The Qt5 version of qmake is located in /usr/local/lib/qt5/bin/qmake on FreeBSD, other OS's may have slightly different locations or names for qmake (such as qmake-qt5 for example)
 * NOTE 2: Check the terminal output of the qmake command to ensure that your OS build settings were loaded properly. Here is an example:

> Project MESSAGE: Build OS Info: FreeBSD, amd64, FreeBSD 10.2-RELEASE-p11 #0: Thu Jan 14 15:48:17 UTC 2016 root@amd64-builder.pcbsd.org:/usr/obj/usr/src/sys/GENERIC

> Project MESSAGE: Build Settings Loaded: FreeBSD

 * Build Note: Compile-time options may be set at this time using qmake. The syntax for this is typically: `qmake CONFIG+="PREFIX=/some/prefix LIBPREFIX=/some/lib/prefix DESTDIR=/some/temporary/packaging/dir"`. This is typically not needed unless you are setting up Lumina for automated build/packaging. If the automatically-detected build settings for your particular OS are invalid or need adjustment, please update the "OS-detect.pri" file as needed and send in your changes.
  * PREFIX: Determines the base directory used to install/run Lumina ("/usr/local" by default)
  * LIBPREFIX: Determines the location to install the Lumina library ("PREFIX/lib" by default)
  * DESTDIR: An optional directory where the compiled files will be placed temporary (such as for packaging/distributing via some other system).
  * NO_I18N: (Recommended) Do not generate/install the translation files. If you wish to install translation files, please use the pcbsd/lumina-i18n repo for the full set (this repo only contains partial translations/templates).


4) Run "make" to compile all the Lumina projects (can be done as user)

5) Run "make install" to install the Lumina desktop on your local system (requires admin/root privileges usually)

6-optional) Run "make distclean" to clean up all the build files in the Lumina source tree. (go back to step 3 later)

