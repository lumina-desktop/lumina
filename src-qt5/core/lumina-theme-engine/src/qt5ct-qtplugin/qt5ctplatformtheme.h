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

#ifndef QT5CTPLATFORMTHEME_H
#define QT5CTPLATFORMTHEME_H

#include <qpa/qplatformtheme.h>
#include <QObject>
#include <QFont>
#include <QPalette>
#include <QLoggingCategory>

#if (QT_VERSION < QT_VERSION_CHECK(5, 5, 0))
#ifndef QT_NO_SYSTEMTRAYICON
#define QT_NO_SYSTEMTRAYICON
#endif
#endif

class QPalette;
#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
class QPlatformSystemTrayIcon;
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)) && !defined(QT_NO_DBUS)
class QPlatformMenuBar;
#endif

class Qt5CTPlatformTheme : public QObject, public QPlatformTheme
{
    Q_OBJECT
public:
    Qt5CTPlatformTheme();

    virtual ~Qt5CTPlatformTheme();


    //virtual QPlatformMenuItem* createPlatformMenuItem() const;
    //virtual QPlatformMenu* createPlatformMenu() const;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)) && !defined(QT_NO_DBUS)
    virtual QPlatformMenuBar* createPlatformMenuBar() const;
#endif
    //virtual void showPlatformMenuBar() {}
    //virtual bool usePlatformNativeDialog(DialogType type) const;
    //virtual QPlatformDialogHelper *createPlatformDialogHelper(DialogType type) const;
#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
    virtual QPlatformSystemTrayIcon *createPlatformSystemTrayIcon() const;
#endif
    virtual const QPalette *palette(Palette type = SystemPalette) const;
    virtual const QFont *font(Font type = SystemFont) const;
    virtual QVariant themeHint(ThemeHint hint) const;
    //virtual QPixmap standardPixmap(StandardPixmap sp, const QSizeF &size) const;
    //virtual QPixmap fileIconPixmap(const QFileInfo &fileInfo, const QSizeF &size,
    //                               QPlatformTheme::IconOptions iconOptions = 0) const;

    //virtual QIconEngine *createIconEngine(const QString &iconName) const;
    //virtual QList<QKeySequence> keyBindings(QKeySequence::StandardKey key) const;
    //virtual QString standardButtonText(int button) const;

private slots:
    void applySettings();
#ifdef QT_WIDGETS_LIB
    void createFSWatcher();
    void updateSettings();
#endif

private:
    void readSettings();
#ifdef QT_WIDGETS_LIB
    bool hasWidgets();
#endif
    QString loadStyleSheets(const QStringList &paths);
    QPalette loadColorScheme(const QString &filePath);
    QString m_style, m_iconTheme, m_userStyleSheet, m_prevStyleSheet;
    QPalette *m_customPalette = nullptr;
    QFont m_generalFont, m_fixedFont;
    int m_doubleClickInterval;
    int m_cursorFlashTime;
    int m_uiEffects;
    int m_buttonBoxLayout;
    bool m_update = false;
    bool m_usePalette = true;
    int m_toolButtonStyle = Qt::ToolButtonFollowStyle;
    int m_wheelScrollLines = 3;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)) && !defined(QT_NO_DBUS)
    mutable bool m_dbusGlobalMenuAvailable = false;
    mutable bool m_checkDBusGlobalMenu = true;
#endif
#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
    mutable bool m_dbusTrayAvailable = false;
    mutable bool m_checkDBusTray = true;
#endif

};

Q_DECLARE_LOGGING_CATEGORY(lqt5ct)

#endif // QT5CTPLATFORMTHEME_H
