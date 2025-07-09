
include("../OS-detect.pri")

TEMPLATE = subdirs
CONFIG += recursive

SUBDIRS+= lcc \
	 lumina-search \
	 lumina-xconfig
