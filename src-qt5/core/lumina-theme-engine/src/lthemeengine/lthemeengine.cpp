/*
 * Copyright (c) 2014-2017, Ilya Kotov <forkotov02@hotmail.ru>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QDir>
#include <QLocale>
#include "lthemeengine.h"

#ifndef LTHEMEENGINE_DATADIR
#define LTHEMEENGINE_DATADIR "/usr/share"
#endif


QString lthemeengine::configPath()
{
    return QDir::homePath() + "/.config/lthemeengine/";
}

QString lthemeengine::configFile()
{
    return configPath() + "lthemeengine.conf";
}

QStringList lthemeengine::iconPaths()
{
    QString xdgDataDirs = qgetenv("XDG_DATA_DIRS");
    QString xdgDataHome = qgetenv("XDG_DATA_HOME");

    QStringList paths;
    paths << QDir::homePath() + "/.icons/";

    if(xdgDataDirs.isEmpty())
    {
        paths << "/usr/share/icons";
        paths << "/usr/local/share/icons";
    }
    else
    {
        foreach (QString p, xdgDataDirs.split(":"))
            paths << QDir(p + "/icons/").absolutePath();
    }

    if(xdgDataHome.isEmpty())
        xdgDataHome = QDir::homePath() + "/.local/share";

    paths << "/usr/share/pixmaps";
    paths << xdgDataHome + "/icons";
    paths.removeDuplicates();

    //remove invalid
    foreach (QString p, paths)
    {
        if(!QDir(p).exists())
            paths.removeAll(p);
    }
    return paths;
}

QString lthemeengine::userStyleSheetPath()
{
    return configPath() + "qss/";
}

QString lthemeengine::sharedStyleSheetPath()
{
    return LTHEMEENGINE_DATADIR"/lthemeengine/qss/";
}

QString lthemeengine::userColorSchemePath()
{
    return configPath() + "colors/";
}

QString lthemeengine::sharedColorSchemePath()
{
    return LTHEMEENGINE_DATADIR"/lthemeengine/colors/";
}

QString lthemeengine::systemLanguageID()
{
#ifdef Q_OS_UNIX
    QByteArray v = qgetenv ("LC_ALL");
    if (v.isEmpty())
        v = qgetenv ("LC_MESSAGES");
    if (v.isEmpty())
        v = qgetenv ("LANG");
    if (!v.isEmpty())
        return QLocale (v).name();
#endif
    return  QLocale::system().name();
}
