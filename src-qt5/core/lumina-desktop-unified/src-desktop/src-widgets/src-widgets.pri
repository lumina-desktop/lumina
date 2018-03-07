#update the includepath so we can just #include as needed without paths
INCLUDEPATH *= $${PWD}

SOURCES *= $${PWD}/RootDesktop.cpp \
		$${PWD}/ContextMenu.cpp

HEADERS *= $${PWD}/RootDesktop.h \
		$${PWD}/ContextMenu.h
