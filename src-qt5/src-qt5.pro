
include("OS-detect.pri")

TEMPLATE = subdirs
CONFIG += recursive

SUBDIRS+= core \
	 core-utils \
	 desktop-utils

#Make sure to list core as a requirement for the others (for parallellized builds)
core-utils.depends = core
desktop-utils.depends = core
