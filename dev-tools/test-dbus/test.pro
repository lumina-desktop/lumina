# Simple Qt program to test whether UI files can be loaded when there is no DBUS session running

QT  = core gui widgets
#Just for consistency - remove the dbus module from this app build (in case "gui" or "widgets" pulled it in)
QT -= dbus

TARGET = test
target.path = $${PWD}

SOURCES = main.cpp 

HEADERS = test.h

FORMS = test.ui