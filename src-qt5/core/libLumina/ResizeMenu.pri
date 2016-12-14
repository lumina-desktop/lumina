#Subproject file for bundling the ResizeMenu class into an application
SOURCES *= $${PWD}/ResizeMenu.cpp
HEADERS *= $${PWD}/ResizeMenu.h
#Add this dir to the include path
# This allows the application to simply use "#include <ResizeMenu.h>" to use it
INCLUDEPATH *= $${PWD}
