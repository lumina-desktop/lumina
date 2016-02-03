# =============================================
#  Subroutine for project files to detect which OS is compiling the project
#  Generic variables it sets for internal use: OS, LINUX_DISTRO (if OS=="Linux")
# =============================================
#  Main Build Variables (generally for finding existing files): 
# 	PREFIX: 		Base install directory (${PREFIX}/[bin/share/etc/include] will be used)
#	LIBPREFIX: 	Base install directory for libraries (usually ${PREFIX}/lib) 
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
!defined(OS){
  message("Build OS Info: $${QMAKE_HOST.os}, $${QMAKE_HOST.arch}, $${QMAKE_HOST.version_string}")
  
  #Load the initial library/includefile search locations (more can be added in the OS-specific sections below)
  LIBS = -L$${PWD}/libLumina -L$$[QT_INSTALL_LIBS]
  INCLUDEPATH = $${PWD}/libLumina $$[QT_INSTALL_HEADERS] $$[QT_INSTALL_PREFIX]
  
  #Setup the default values for build settings (if not explicitly set previously)
  !defined(PREFIX){ PREFIX=/usr/local }
  !defined(LIBPREFIX){ LIBPREFIX=$${PREFIX}/lib }
  
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
    PREFIX=/usr/local
    LIBPREFIX=/usr/local/lib
    #Use the defaults for everything else
    
  }else : linux-*{
    L_SESSDIR=/usr/share/xsessions
    OS=Linux
    LIBS += -L/usr/local/lib -L/usr/lib -L/lib
    PREFIX = /usr
    
    exists(/bin/lsb_release){
      LINUX_DISTRO = $$system(lsb_release -si)
    } else:exists(/usr/bin/lsb_release){
      LINUX_DISTRO = $$system(lsb_release -si)
    }
    
    #Now switch through known Linux distro templates
    equals(LINUX_DISTRO, "Fedora"){
      PREFIX=/usr/local
      equals($${QMAKE_HOST.arch},"amd64"){ L_LIBDIR=/lib64 }
      else{ L_LIBDIR=/lib }
    }
    
  }else{ 
    OS="Unknown"; 
  }
  
  MSG="Build Settings Loaded: $${OS}"
  equals(OS,"Linux"){ MSG+="-$${LINUX_DISTRO}" }
  message( $$MSG )
  
  # Setup the dirs needed to find/load libraries
  QMAKE_LIBDIR =  $${PWD}/libLumina $$[QT_INSTALL_LIBS] $$LIBPREFIX/qt5 $$LIBPREFIX
  INCLUDEPATH +=$${PREFIX}/include
  
  # If the detailed install variables are not set - create them from the general vars
  !defined(L_BINDIR){ L_BINDIR = $${PREFIX}/bin }
  !defined(L_LIBDIR){ L_LIBDIR = $${PREFIX}/lib }
  !defined(L_ETCDIR){ L_ETCDIR = $${PREFIX}/etc }
  !defined(L_SHAREDIR){ L_SHAREDIR = $${PREFIX}/share }
  !defined(L_INCLUDEDIR){ L_INCLUDEDIR = $${PREFIX}/include }
  !defined(L_SESSDIR){ L_SESSDIR = $${L_SHAREDIR}/xsessions }
  !defined(LRELEASE){ LRELEASE = $$[QT_INSTALL_BINS]/lrelease }
  
  !exists(LRELEASE){ NO_I18N=true } #translations unavailable
  #Now convert any of these path variables into defines for C++ usage
  DEFINES += PREFIX="QString\\\(\\\"$${PREFIX}\\\"\\\)"
  DEFINES += L_ETCDIR="QString\\\(\\\"$${L_ETCDIR}\\\"\\\)"
  DEFINES += L_SHAREDIR="QString\\\(\\\"$${L_SHAREDIR}\\\"\\\)"
}
