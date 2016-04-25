
include("../OS-detect.pri")

TEMPLATE = subdirs
CONFIG += recursive

SUBDIRS+= libLumina \
	 lumina-desktop \
	 lumina-open \
	 lumina-info
#	lumina-wm-INCOMPLETE \
#	lumina-checkpass


#Make sure to list libLumina as a requirement for the others (for parallellized builds)
lumina-desktop.depends = libLumina
lumina-open.depends = libLumina
lumina-info.depends = libLumina
