
include("../OS-detect.pri")

TEMPLATE = subdirs
CONFIG += recursive

SUBDIRS+= lumina-fm \
	lumina-screenshot \
	lumina-fileinfo \
	lumina-textedit \
	lumina-calculator \
	lumina-archiver \
#	lumina-notify \
	lumina-mediaplayer \
	lumina-xdg-entry
#	lumina-terminal
