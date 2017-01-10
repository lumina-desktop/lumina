SOURCES *= $${PWD}/LXcbEventFilter.cpp

HEADERS *= $${PWD}/LXcbEventFilter.h

#update the includepath so we can just (#include <LXcbEventFilter.h>) as needed without paths
INCLUDEPATH *= ${PWD}
