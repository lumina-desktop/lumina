#ifndef LTHEMEENGINEPLATFORMTHEME_H
#define LTHEMEENGINEPLATFORMTHEME_H

#include <qpa/qplatformtheme.h>
#include <QObject>
#include <QFont>
#include <QPalette>
#include <QLoggingCategory>
#include <QFileSystemWatcher>

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

class lthemeenginePlatformTheme : public QObject, public QPlatformTheme{
    Q_OBJECT
public:
    lthemeenginePlatformTheme();

    virtual ~lthemeenginePlatformTheme();


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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)) && !defined(QT_NO_DBUS)
    mutable bool m_dbusGlobalMenuAvailable = false;
    mutable bool m_checkDBusGlobalMenu = true;
#endif
#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
    mutable bool m_dbusTrayAvailable = false;
    mutable bool m_checkDBusTray = true;
#endif

	void syncMouseCursorTheme(QString indexfile);
};

Q_DECLARE_LOGGING_CATEGORY(llthemeengine)

#endif // LTHEMEENGINEPLATFORMTHEME_H
