/*
# lumina-freedesktop.org.ScreenSaver
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <LuminaSingleApplication.h>
#include "screensaver.h"

#define SERVICE "org.freedesktop.ScreenSaver"

int main(int argc, char *argv[])
{
    LSingleApplication a(argc, argv, "");
    if(!a.isPrimaryProcess()) { return 0; }

    QCoreApplication::setApplicationName("freedesktop");
    QCoreApplication::setOrganizationDomain("org");

    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning("Cannot connect to D-Bus.");
        return 1;
    }

    if (!QDBusConnection::sessionBus().registerService(SERVICE)) {
        qWarning() << QDBusConnection::sessionBus().lastError().message();
        return 1;
    }

    ScreenSaver ss;
    QDBusConnection::sessionBus().registerObject("/ScreenSaver", &ss, QDBusConnection::ExportAllContents);

    return a.exec();
}
