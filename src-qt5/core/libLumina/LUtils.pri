#since this is the most common of the include files - make sure it only gets added once
!contains( HEADERS, $${PWD}/LUtils.h ){ 

include("$${PWD}/../../OS-detect.pri")

QT *= concurrent network

#Setup any special defines (qmake -> C++)
GIT_VERSION=$$system(git describe --always)
!isEmpty(GIT_VERSION){
  DEFINES += GIT_VERSION='"\\\"$${GIT_VERSION}\\\""'
}
#Note: Saving the build date will break reproducible builds (time stamp always different)
# Disable this by default, but leave it possible to re-enable this as needed by user
#DEFINES += BUILD_DATE='"\\\"$$system(date)\\\""'

#LuminaOS files
HEADERS *= $${PWD}/LuminaOS.h \
	$${PWD}/OSInterface.h

# LuminaOS support functions (or fall back to generic one)
exists($${PWD}/LuminaOS-$${LINUX_DISTRO}.cpp){
  SOURCES *= $${PWD}/LuminaOS-$${LINUX_DISTRO}.cpp
}else:exists($${PWD}/LuminaOS-$${OS}.cpp){
  SOURCES *= $${PWD}/LuminaOS-$${OS}.cpp
}else{
  SOURCES *= $${PWD}/LuminaOS-template.cpp
}
exists($${PWD}/OSInterface-$${LINUX_DISTRO}.cpp){
  SOURCES *= $${PWD}/OSInterface-$${LINUX_DISTRO}.cpp
}else:exists($${PWD}/OSInterface-$${OS}.cpp){
  SOURCES *= $${PWD}/OSInterface-$${OS}.cpp
}else{
  SOURCES *= $${PWD}/OSInterface-template.cpp
}

#LUtils Files
SOURCES *= $${PWD}/LUtils.cpp
HEADERS *= $${PWD}/LUtils.h

INCLUDEPATH *= ${PWD}

}
