# =============================================
#  Subroutine for project files to detect which OS is compiling the project
#  Generic variables it sets for internal use: OS, LINUX_DISTRO (if OS=="Linux")
# =============================================
#  Main Build Variables: 
# 	PREFIX: 		Base install directory (${PREFIX}/[bin/share/etc/include] will be used)
#	LIBPREFIX: 	Base install directory for libraries (usually ${PREFIX}/lib)
#	QTLIBDIR: 		Directory where the Qt libraries are currently installed 
#
# =============================================
# Note: Make sure the OS variable matches the name of a libLumina/LuminaOS-<OS>.cpp file
# =============================================
!defined(OS){
  message("Build OS Info: $${QMAKE_HOST.os}, $${QMAKE_HOST.arch}, $${QMAKE_HOST.version_string}")
  OS=$$QMAKE_HOST.os
  
  #Setup the default values for build settings (if not explicitly set previously)
  !defined(PREFIX){ PREFIX=/usr/local/ }
  !defined(LIBPREFIX){ LIBPREFIX=$${PREFIX}/lib }
  !defined(QTLIBDIR){ QTLIBDIR=$${LIBPREFIX}/qt5 }
  
  #Now go through and setup any known OS build settings
  #  which are different from the defaults
  equals(OS, "NetBSD"){
    PREFIX=/usr/local
    LIBPREFIX=/usr/local/lib
    QTLIBDIR=/usr/local/lib/qt5  
  }else:contains(OS, "Linux"){
    OS=Linux
    exists(/bin/lsb_release){
      LINUX_DISTRO = $$system(lsb_release -si)
    } else:exists(/usr/bin/lsb_release){
      LINUX_DISTRO = $$system(lsb_release -si)
    }
    
    #Now switch through known Linux distro templates
    equals(LINUX_DISTRO, "Fedora"){
      PREFIX=/usr/local
      LIBPREFIX=/lib64
      QTLIBDIR=/lib/qt5
      
    }
    
  }
  MSG="Build Settings Loaded: $${OS}"
  equals(OS,"Linux"){ MSG+="-$${LINUX_DISTRO}" }
  message( $$MSG )
  #now remove the temporary MSG variable
  
}