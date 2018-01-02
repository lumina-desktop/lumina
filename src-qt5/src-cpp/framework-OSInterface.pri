QT *= core network

HEADERS *= $${PWD}/framework-OSInterface.h
SOURCES *= $${PWD}/framework-OSInterface_private.cpp

_os=template
SOURCES *= $${PWD}/framework-OSInterface-$${_os}.cpp

INCLUDEPATH *= $${PWD}
