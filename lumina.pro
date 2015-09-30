TEMPLATE = subdirs
CONFIG += recursive

SUBDIRS+= libLumina \
	 lumina-desktop \
	 lumina-open \
	 lumina-config \
	 lumina-fm \
	 lumina-screenshot \
	 lumina-search \
	 lumina-info \
	 lumina-xconfig \
	 lumina-fileinfo

#Make sure to list libLumina as a requirement for the others (for parallellized builds)
lumina-desktop.depends = libLumina
lumina-open.depends = libLumina
lumina-config.depends = libLumina
lumina-fm.depends = libLumina
lumina-screenshot.depends = libLumina
lumina-search.depends = libLumina
lumina-info.depends = libLumina
lumina-xconfig.depends = libLumina
lumina-fileinfo.depends = libLumina
