SOURCES *= $${PWD}/LXCBEventFilter.cpp

HEADERS *= $${PWD}/LXCBEventFilter.h

#update the includepath so we can just (#include <LXCBEventFilter.h>) as needed without paths
INCLUDEPATH *= ${PWD}
