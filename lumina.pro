#=======================================================
#  This is just a simple redirect into the source tree for builds
#=======================================================
TEMPLATE = subdirs

#Load the OS detect pri file here so that it will print output as needed
include(src-qt5/OS-detect.pri)

SUBDIRS+= src-qt5
