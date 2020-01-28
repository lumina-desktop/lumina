# =============================================
#  Subroutine for project files to detect which OS is compiling the project
#  Generic variables it sets for internal use: OS, LINUX_DISTRO (if OS=="Linux")
#  **DO NOT** forcibly define the OS and LINUX_DISTRO qmake variables from the command line!!
# =============================================
#  Main Build Variables (generally for finding existing files):
# 	PREFIX: 		Base install directory (${PREFIX}/[bin/share/etc/include] will be used)
#	LIBPREFIX: 	Base install directory for libraries (usually ${PREFIX}/lib)
#  Automated build variables (for pkg builders and such)
#	DESTDIR:		Prepended to the install location of all files (such as a temporary working directory)
#				- Note that the Lumina will probably not run properly from this dir (not final install dir)
#  Detailed Build Variables for installing files:
#	L_BINDIR:		Directory to install binaries
#	L_LIBDIR:		Directory to install Libraries
#	L_ETCDIR:		Directory to install/use global configs
#	L_SHAREDIR:	Directory to install the general Lumina files
#	L_INCLUDEDIR:	Directory to install include files
#	L_SESSDIR:		Directory to place *.desktop file for starting the Lumina session
#	L_MANDIR:		Directory to place man files
#	LRELEASE:		binary path to the Qt lrelease utility (usually auto-set)
# =============================================
# Note: Make sure the OS variable matches the name of a libLumina/LuminaOS-<OS>.cpp file
# =============================================
isEmpty(OS){
  message("Build OS Info: $${QMAKE_HOST.os}, $${QMAKE_HOST.arch}, $${QMAKE_HOST.version_string}")
  message(" - Detected Qt Version: $${QT_VERSION}")
  #Load the initial library/includefile search locations (more can be added in the OS-specific sections below)
  LIBS = -L$${PWD}/core/libLumina -L$$[QT_INSTALL_LIBS]
  INCLUDEPATH = $${PWD}/core/libLumina $$[QT_INSTALL_HEADERS] $$[QT_INSTALL_PREFIX]
  QMAKE_LIBDIR =  $${PWD}/core/libLumina $$[QT_INSTALL_LIBS] $$LIBPREFIX/qt5 $$LIBPREFIX

  #Setup the default values for build settings (if not explicitly set previously)
  isEmpty(PREFIX){ PREFIX=/usr/local }
  isEmpty(LIBPREFIX){ LIBPREFIX=$${PREFIX}/lib }
  #message("General Settings: PREFIX=$${PREFIX}, LIBPREFIX=$${LIBPREFIX}")
  #Now go through and setup any known OS build settings
  #  which are different from the defaults
  equals(QMAKE_HOST.os, "DragonFly"){
    #Note: DragonFly BSD gets detected as FreeBSD with compiler flags ?
    OS = DragonFly
    LIBS += -L/usr/local/lib -L/usr/lib

  }else : freebsd-*{
    OS = FreeBSD
    LIBS += -L/usr/local/lib -L/usr/lib
     #Use the defaults for everything else

  }else : openbsd-*{
    OS = OpenBSD
    LIBS += -L/usr/local/lib -L/usr/lib
    #Use the defaults for everything else

  }else : netbsd-*{
    OS = NetBSD
    LIBS += -L/usr/local/lib -L/usr/lib
    #Use the defaults for everything else

  }else : linux-*{
    L_SESSDIR=/usr/share/xsessions
    OS=Linux
    LIBS += -L/usr/local/lib -L/usr/lib -L/lib

    #Detect *which* distro this is
    isEmpty(LINUX_DISTRO){
      exists(/bin/lsb_release){
        LINUX_DISTRO = $$system(lsb_release -si)
      } else:exists(/usr/bin/lsb_release){
        LINUX_DISTRO = $$system(lsb_release -si)
      } else:exists(/etc/gentoo-release){
        LINUX_DISTRO = Gentoo
      } else:exists(/etc/slackware-version){
	LINUX_DISTRO = Slackware
      }
   }
    #Apply any special rules for particular distros
    equals(LINUX_DISTRO,"Fedora"){
      isEmpty(L_ETCDIR){ L_ETCDIR=/etc }
    }
    equals(LINUX_DISTRO,"Gentoo"){
      isEmpty(L_BINDIR){ L_BINDIR = $${PREFIX}/bin }
      isEmpty(L_ETCDIR){ L_ETCDIR = $${PREFIX}/../etc }
      isEmpty(L_MANDIR){ L_MANDIR = $${PREFIX}/share/man }
    }
    equals(LINUX_DISTRO,"VoidLinux"){
      isEmpty(L_BINDIR){ L_BINDIR = $${PREFIX}/bin }
      isEmpty(L_ETCDIR){ L_ETCDIR = $${PREFIX}/../etc }
      isEmpty(L_MANDIR){ L_MANDIR = $${PREFIX}/share/man }
    }
  }else{
    OS="Unknown";
  }

  MSG="Build Settings Loaded: $${OS}"
  equals(OS,"Linux"){ MSG+="-$${LINUX_DISTRO}" }
  message( $$MSG )

  # Setup the dirs needed to find/load libraries
  INCLUDEPATH +=$${PREFIX}/include 

  # If the detailed install variables are not set - create them from the general vars
  isEmpty(L_BINDIR){ L_BINDIR = $${PREFIX}/bin }
  isEmpty(L_LIBDIR){ L_LIBDIR = $${LIBPREFIX} }
  isEmpty(L_ETCDIR){ L_ETCDIR = $${PREFIX}/etc }
  isEmpty(L_SHAREDIR){ L_SHAREDIR = $${PREFIX}/share }
  isEmpty(L_INCLUDEDIR){ L_INCLUDEDIR = $${PREFIX}/include }
  isEmpty(L_SESSDIR){ L_SESSDIR = $${L_SHAREDIR}/xsessions }
  isEmpty(L_MANDIR){ L_MANDIR = $${PREFIX}/man }
  isEmpty(LRELEASE){ LRELEASE = $$[QT_INSTALL_BINS]/lrelease }
  isEmpty(MAN_ZIP){ MAN_ZIP = "gzip -c" }
  !exists(LRELEASE){ NO_I18N=true } #translations unavailable

  #Now convert any of these install path variables into defines for C++ usage
  DEFINES += PREFIX="QString\\\(\\\"$${PREFIX}\\\"\\\)"
  DEFINES += L_ETCDIR="QString\\\(\\\"$${L_ETCDIR}\\\"\\\)"
  DEFINES += L_SHAREDIR="QString\\\(\\\"$${L_SHAREDIR}\\\"\\\)"

  #If this is being installed to a temporary directory, change the paths where things get placed
  !isEmpty(DESTDIR){
    L_BINDIR = $$DESTDIR$${L_BINDIR}
    L_LIBDIR = $$DESTDIR$${L_LIBDIR}
    L_ETCDIR = $$DESTDIR$${L_ETCDIR}
    L_SHAREDIR = $$DESTDIR$${L_SHAREDIR}
    L_INCLUDEDIR = $$DESTDIR$${L_INCLUDEDIR}
    L_SESSDIR = $$DESTDIR$${L_SESSDIR}
    L_MANDIR = $$DESTDIR$${L_MANDIR}
  }

  #Some conf to redirect intermediate stuff in separate dirs
  UI_DIR=./.build/ui/
  MOC_DIR=./.build/moc/
  OBJECTS_DIR=./.build/obj
  RCC_DIR=./.build/rcc
  QMAKE_DISTCLEAN += -r ./.build

  #Setup the default place for installing icons (use scalable dir for variable-size icons)
  icons.path = $${L_SHAREDIR}/icons/hicolor/scalable/apps

  #some other compile time flags
  CONFIG *= c++11
}
