isEmpty(PREFIX) {
 PREFIX = /usr/local
}

system(make-global-h.sh $$PREFIX)

TEMPLATE = subdirs

SUBDIRS+= libLumina \
	 lumina-desktop \
	 lumina-open \
	 lumina-config \
	 lumina-fm \
	 lumina-screenshot

