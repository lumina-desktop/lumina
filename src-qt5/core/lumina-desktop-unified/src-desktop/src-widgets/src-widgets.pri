#update the includepath so we can just #include as needed without paths
INCLUDEPATH *= $${PWD}

SOURCES *= $${PWD}/RootDesktop.cpp \
		$${PWD}/ContextMenu.cpp \
		$${PWD}/NativeWindow.cpp

HEADERS *= $${PWD}/RootDesktop.h \
		$${PWD}/ContextMenu.h \
		$${PWD}/NativeWindow.h
