QT *= core network quick

include(../core/libLumina/LUtils.pri)
include(../core/libLumina/LuminaXDG.pri)
HEADERS *= $${PWD}/framework-OSInterface.h
SOURCES *= $${PWD}/framework-OSInterface_private.cpp

_os=template
SOURCES *= $${PWD}/framework-OSInterface-$${_os}.cpp

INCLUDEPATH *= $${PWD}
