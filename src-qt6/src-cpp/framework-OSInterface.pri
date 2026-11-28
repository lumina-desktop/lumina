QT *= core network quick concurrent

#include(../core/libLumina/LUtils.pri)
#include(../core/libLumina/LuminaXDG.pri)

HEADERS *= $${PWD}/framework-OSInterface.h
SOURCES *= $${PWD}/framework-OSInterface_private.cpp

#Load the proper OS *.cpp file
exists($${PWD}/framework-OSInterface-$${LINUX_DISTRO}.cpp){
  SOURCES *= $${PWD}/framework-OSInterface-$${LINUX_DISTRO}.cpp
}else:exists($${PWD}/framework-OSInterface-$${OS}.cpp){
  SOURCES *= $${PWD}/framework-OSInterface-$${OS}.cpp
}else{
  SOURCES *= $${PWD}/framework-OSInterface-template.cpp
}

INCLUDEPATH *= $${PWD}
