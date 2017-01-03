Lumina
======

[![Join the chat at https://gitter.im/trueos/lumina](https://badges.gitter.im/trueos/lumina.svg)](https://gitter.im/trueos/lumina?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

This is the official source tree for the Lumina Desktop Environment.

For additional information about the project, please browse the official website for documentation, screenshots, and more: www.lumina-desktop.org


Description
=====

The Lumina Desktop Environment is a lightweight system interface that is designed for use on any Unix-like operating system. It takes a plugin-based approach, allowing the entire interface to be assembled/arranged by each individual user as desired, with a system-wide default layout which can be setup by the system administrator. This allows every system (or user session) to be designed to maximize the individual user's productivity.

Latest Versions
----
Click [HERE](https://github.com/trueos/lumina/releases) to see all the available releases for the Lumina Desktop.

Click [HERE]() to see all the available branches of the Lumina Desktop source tree (includes development versions: *.*.(X > 0)).
The master branch is where all development is performed and tested before the next version is tagged. Additionally, every version (development **and** release) of the project gets branched off of master at the time the version is tagged, so older versions may be fetched/patched/used as desired.

Occasionally there may be patches committed for particular versions which are not included in the static "release" archives. On the release page these can be identified by a note stating the release is a number of commits behind it's corresponding branch. In this situation, it may be preferable to fetch/use the corresponding branch of the source tree rather than the static archive if the patch(es) which were committed impact your particular operating system.

Translations
----
**Version 1.0.0 and older**

All the translation files for Lumina are available through the TrueOS pootle translations systems, which are then auto-committed to the pcbsd/lumina-i18n repo (Qt5+ only). This provides an easy interface through which to contribute translations while also ensuring that the translation files are kept in sync with the Lumina source files, and it is highly recommended that translators use this interface.

Translation Website URL: http://translate.pcbsd.org/

**Version 1.1.0+**

All the translations for Lumina are contained within this repository directly (the i18n/ subdirectories within each project source tree). To include these translation files, the `WITH_I18N` build flag must be set at compilation time, in which case the translation files will automatically get synced/built alongside the corresponding binaries.

To submit translations, please use the online [TrueOS Translation System](http://weblate.trueos.org/projects/lumina/). Anonymous users may submit translation suggestions, but a website account will be needed to created before you can submit/approve translations directly to the project. See the documentation/information on the site for full details.

How to file bug reports or feature requests
----
Please create a ticket through the GitHub issues tracker on this repository. Similarly, if you want to send in patches or other source contributions, please send in a GitHub pull request so that it can get reviewed/committed to the main repo as quickly as possible.


How to build from source
----
1) Checkout the source repo to your local box (GitHub gives a few methods for this)

Examples:
* To checkout the master branch of the repo, run `git clone https://github.com/trueos/lumina` to create a "lumina" directory with a local copy of the source tree.
* To update an exising checkout of the Lumina repo, run "git pull" while within your local copy of the source tree. 

2) In a terminal, change to the lumina repo directory (base dir, not one of the sub-projects)

3) Run "qmake" on your local system to turn all the Qt project files (*.pro) into Makefiles
 * NOTE 1: The Qt5 version of qmake is located in /usr/local/lib/qt5/bin/qmake on FreeBSD, other OS's may have slightly different locations or names for qmake (such as qmake-qt5 for example)
 * NOTE 2: Check the terminal output of the qmake command to ensure that your OS build settings were loaded properly. Here is an example:

> Project MESSAGE: Build OS Info: FreeBSD, amd64, FreeBSD 10.2-RELEASE-p11 #0: Thu Jan 14 15:48:17 UTC 2016 root@amd64-builder.pcbsd.org:/usr/obj/usr/src/sys/GENERIC

> Project MESSAGE: Build Settings Loaded: FreeBSD

 * Build Note: Compile-time options may be set at this time using qmake. This is typically not needed unless you are setting up Lumina for automated build/packaging. If the automatically-detected build settings for your particular OS are invalid or need adjustment, please update the "OS-detect.pri" file as needed and send in your changes so that it can be corrected for future builds.
  * PREFIX: Determines the base directory used to install/run Lumina ("/usr/local" by default)
	Example: `qmake PREFIX=/usr/local`
  * LIBPREFIX: Determines the location to install the Lumina library ("PREFIX/lib" by default)
	Example: `qmake LIBPREFIX=/usr/local/lib`
  * DESTDIR: An optional directory where the compiled files will be placed temporary (such as for packaging/distributing via some other system).
	Example: `qmake DESTDIR=/my/build/dir`
  * DEFAULT_SETTINGS: An optional flag to install the default settings/wallpaper for some other operating system (Note: Make sure the OS name is capitalized appropriately!)
	Example: `qmake DEFAULT_SETTINGS=TrueOS`
  * WITH_I18N: Generate/install the translation files. (Note: For versions prior to 1.1.0, do *not* use this flag! The localization files for versions up through 1.0.0 are available in the lumina-i18n repository instead).
	Example: `qmake CONFIG+=WITH_I18N`
  * debug: (not recommended for release builds) Do not strip all the debugging information out of the binaries (useful for performing backtraces on builds that crash or for other development tests)
	Example: `qmake CONFIG+=debug`

4) Run "make" to compile all the Lumina projects (can be done as user)

5) Run "make install" to install the Lumina desktop on your local system (requires admin/root privileges usually)

6-optional) Run "make distclean" to clean up all the build files in the Lumina source tree. (go back to step 3 later)
