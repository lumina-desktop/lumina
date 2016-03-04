# =============================================
#  Subroutine for project files to detect which OS is compiling the project
#  Generic variables it sets for internal use: OS, LINUX_DISTRO (if OS=="Linux")
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
#	L_SESSDIR:	Directory to place *.desktop file for starting the Lumina session
#	LRELEASE:	binary path to the Qt lrelease utility (usually auto-set)
# =============================================
# Note: Make sure the OS variable matches the name of a libLumina/LuminaOS-<OS>.cpp file
# =============================================
isEmpty(OS){
  message("Build OS Info: $${QMAKE_HOST.os}, $${QMAKE_HOST.arch}, $${QMAKE_HOST.version_string}")
  
  #Load the initial library/includefile search locations (more can be added in the OS-specific sections below)
  LIBS = -L$${PWD}/libLumina -L$$[QT_INSTALL_LIBS]
  INCLUDEPATH = $${PWD}/libLumina $$[QT_INSTALL_HEADERS] $$[QT_INSTALL_PREFIX]
  QMAKE_LIBDIR =  $${PWD}/libLumina $$[QT_INSTALL_LIBS] $$LIBPREFIX/qt5 $$LIBPREFIX
  
  #Setup the default values for build settings (if not explicitly set previously)
  isEmpty(PREFIX){ PREFIX=/usr/local }
  isEmpty(LIBPREFIX){ LIBPREFIX=$${PREFIX}/lib }
  
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
    
    exists(/bin/lsb_release){
      LINUX_DISTRO = $$system(lsb_release -si)
    } else:exists(/usr/bin/lsb_release){
      LINUX_DISTRO = $$system(lsb_release -si)
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
  isEmpty(L_LIBDIR){ L_LIBDIR = $${PREFIX}/lib }
  isEmpty(L_ETCDIR){ L_ETCDIR = $${PREFIX}/etc }
  isEmpty(L_SHAREDIR){ L_SHAREDIR = $${PREFIX}/share }
  isEmpty(L_INCLUDEDIR){ L_INCLUDEDIR = $${PREFIX}/include }
  isEmpty(L_SESSDIR){ L_SESSDIR = $${L_SHAREDIR}/xsessions }
  isEmpty(LRELEASE){ LRELEASE = $$[QT_INSTALL_BINS]/lrelease }
  
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
  }
}
