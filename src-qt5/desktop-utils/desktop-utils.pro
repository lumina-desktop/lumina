
include("../OS-detect.pri")

TEMPLATE = subdirs
CONFIG += recursive

SUBDIRS+= lumina-fm \
	lumina-screenshot \
	lumina-fileinfo \
	lumina-textedit \
	lumina-archiver \
	lumina-mediaplayer \
	lumina-photo
