#Show the QML files to lupdate for translation purposes - not for the actual build

QT *= quick qml

lupdate_only{
  SOURCES *= $${PWD}/RootDesktop.qml
}

RESOURCES *= $${PWD}/src-qml.qrc
