#update the includepath so we can just #include as needed without paths
INCLUDEPATH *= $${PWD}

SOURCES *= $${PWD}/SystemTrayMenu.cpp \
		$${PWD}/VolumeButton.cpp \
		$${PWD}/BatteryButton.cpp \
		$${PWD}/NetworkButton.cpp \
		$${PWD}/UpdatesButton.cpp

HEADERS *= $${PWD}/ClockPlugin.h \
		$${PWD}/CalendarMenu.h \
		$${PWD}/SpacerPlugin.h \
		$${PWD}/StatusTrayPlugin.h \
		$${PWD}/SystemTrayMenu.h \
		$${PWD}/VolumeButton.h \
		$${PWD}/BatteryButton.h \
		$${PWD}/NetworkButton.h \
		$${PWD}/UpdatesButton.h
