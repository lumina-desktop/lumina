SOURCES *= $${PWD}/RootWindow.cpp

HEADERS *= $${PWD}/RootWindow.h

#update the includepath so we can just #include as needed without paths
INCLUDEPATH *= $${PWD}

include(../../../src-cpp/Desktop.pri)
