SOURCES *= $${PWD}/LXcbEventFilter.cpp

HEADERS *= $${PWD}/LXcbEventFilter.h

#Shortcut event files
SOURCES *= $${PWD}/LShortcutEvents.cpp
HEADERS *= $${PWD}/LShortcutEvents.h

#update the includepath so we can just (#include <LXcbEventFilter.h>) as needed without paths
INCLUDEPATH *= ${PWD}
