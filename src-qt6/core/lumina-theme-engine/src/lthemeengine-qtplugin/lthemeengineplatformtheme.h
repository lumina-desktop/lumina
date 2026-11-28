#ifndef LTHEMEENGINEPLATFORMTHEME_H
#define LTHEMEENGINEPLATFORMTHEME_H

#include <qpa/qplatformtheme.h>
//QGenericUnixTheme's header was renamed in Qt 6.10 (qgenericunixthemes_p.h -> qgenericunixtheme_p.h); in Qt5 it
// lives in the ThemeSupport module
#if __has_include(<private/qgenericunixtheme_p.h>)
#include <private/qgenericunixtheme_p.h>
#elif __has_include(<private/qgenericunixthemes_p.h>)
#include <private/qgenericunixthemes_p.h>
#else
#include <QtThemeSupport/private/qgenericunixthemes_p.h>
#endif
#include <QObject>
#include <QFont>
#include <QPalette>
#include <QLoggingCategory>
#include <QFileSystemWatcher>

class lthemeenginePlatformTheme : public QObject, public QGenericUnixTheme{
    Q_OBJECT
public:
    lthemeenginePlatformTheme();

    virtual ~lthemeenginePlatformTheme();

    // createPlatformMenuBar() and createPlatformSystemTrayIcon() are
    // inherited from QGenericUnixTheme (D-Bus global menu and tray).
    virtual const QPalette *palette(Palette type = SystemPalette) const override;
    virtual const QFont *font(Font type = SystemFont) const override;
    virtual QVariant themeHint(ThemeHint hint) const override;

private slots:
    void applySettings();
#ifdef QT_WIDGETS_LIB
    void createFSWatcher();
    void updateSettings();
#endif
    void fileChanged(QString);

private:
    void readSettings();
#ifdef QT_WIDGETS_LIB
    bool hasWidgets();
#endif
    QString loadStyleSheets(const QStringList &paths);
    QPalette loadColorScheme(QString filePath);
    QString m_style, m_iconTheme, m_userStyleSheet, m_oldStyleSheet, m_cursorTheme;
    QPalette *m_customPalette = nullptr;
    QPalette *outgoingpalette = nullptr;
    QFont m_generalFont, m_fixedFont;
    int m_doubleClickInterval;
    int m_cursorFlashTime;
    int m_uiEffects;
    int m_buttonBoxLayout;
    bool m_update = false;
    bool m_usePalette = true;
    int m_toolButtonStyle = Qt::ToolButtonFollowStyle;
    int m_wheelScrollLines = 3;
    QFileSystemWatcher *watcher;
	void syncMouseCursorTheme(QString indexfile);
};

Q_DECLARE_LOGGING_CATEGORY(llthemeengine)

#endif // LTHEMEENGINEPLATFORMTHEME_H
