QT *= gui widgets

#update the includepath so we can just #include as needed without paths
INCLUDEPATH *= $${PWD}

SOURCES *= $${PWD}/Desktopmanager.cpp \
		$${PWD}/QMLImageProvider.cpp

HEADERS *= $${PWD}/RootWindow.h \
		$${PWD}/DesktopManager.h

#include the base objects
include($${PWD}/src-cpp/src-cpp.pri)

#Now do the QML/Widgets interface switch
isEmpty(USE_QML){
  #Widgets-based interface
  DEFINES += USE_WIDGETS="true"

  SOURCES *= $${PWD}/RootWindow-Widgets.cpp

  include($${PWD}/src-widgets/src-widgets.pri)

}else{
  #QML-based interface
  QT *= qml quick
  DEFINES += USE_QML="true"
  SOURCES *= $${PWD}/RootWindow-QML.cpp
  HEADERS *= $${PWD}/QMLImageProvider.h

  include($${PWD}/src-qml/src-qml.pri)
}
