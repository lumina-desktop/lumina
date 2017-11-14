QT *= gui widgets qml quick

SOURCES *= $${PWD}/RootWindow.cpp

HEADERS *= $${PWD}/RootWindow.h

#update the includepath so we can just #include as needed without paths
INCLUDEPATH *= $${PWD}

include($${PWD}/src-cpp/src-cpp.pri)
include($${PWD}/src-qml/src-qml.pri)
