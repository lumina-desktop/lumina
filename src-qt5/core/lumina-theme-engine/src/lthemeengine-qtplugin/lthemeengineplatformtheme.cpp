#include <QVariant>
#include <QSettings>
#include <QGuiApplication>
#include <QScreen>
#include <QFont>
#include <QPalette>
#include <QTimer>
#include <QIcon>
#include <QRegExp>
#include <QWindow>

#ifdef QT_WIDGETS_LIB
#include <QStyle>
#include <QStyleFactory>
#include <QApplication>
#include <QWidget>
#endif

#include <QFile>
#include <QFileSystemWatcher>
#include <QDir>
#include <QTextStream>

#include <stdlib.h>

#include <lthemeengine/lthemeengine.h>
#include "lthemeengineplatformtheme.h"
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)) && !defined(QT_NO_DBUS)
#include <private/qdbusmenubar_p.h>
#endif
#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
#include <QDBusArgument>
#include <private/qdbustrayicon_p.h>
#endif

#include <QX11Info>
#include <QCursor>
//Need access to the private QCursor header so we can refresh the mouse cursor cache
//#include <private/qcursor_p.h> //Does not work - looks like we need to use X11 stuff instead
#include <X11/Xcursor/Xcursor.h>

Q_LOGGING_CATEGORY(llthemeengine, "lthemeengine")

//QT_QPA_PLATFORMTHEME=lthemeengine

lthemeenginePlatformTheme::lthemeenginePlatformTheme(){
  if(QGuiApplication::desktopSettingsAware()){
    readSettings();
#ifdef QT_WIDGETS_LIB
    QMetaObject::invokeMethod(this, "createFSWatcher", Qt::QueuedConnection);
#endif
    QMetaObject::invokeMethod(this, "applySettings", Qt::QueuedConnection);

    QGuiApplication::setFont(m_generalFont);
    }
  //qCDebug(llthemeengine) << "using lthemeengine plugin";
#ifdef QT_WIDGETS_LIB
  if(!QStyleFactory::keys().contains("lthemeengine-style"))
    qCCritical(llthemeengine) << "unable to find lthemeengine proxy style";
#endif
}

lthemeenginePlatformTheme::~lthemeenginePlatformTheme(){
  if(m_customPalette)
    delete m_customPalette;
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)) && !defined(QT_NO_DBUS)
QPlatformMenuBar *lthemeenginePlatformTheme::createPlatformMenuBar() const{
  if(m_checkDBusGlobalMenu){
    QDBusConnection conn = QDBusConnection::sessionBus();
    m_dbusGlobalMenuAvailable = conn.interface()->isServiceRegistered("com.canonical.AppMenu.Registrar");
    //qCDebug(llthemeengine) << "D-Bus global menu:" << (m_dbusGlobalMenuAvailable ? "yes" : "no");
    }
  return (m_dbusGlobalMenuAvailable ? new QDBusMenuBar() : nullptr);
}
#endif

#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
QPlatformSystemTrayIcon *lthemeenginePlatformTheme::createPlatformSystemTrayIcon() const{
  if(m_checkDBusTray){
    QDBusMenuConnection conn;
    m_dbusTrayAvailable = conn.isStatusNotifierHostRegistered();
    m_checkDBusTray = false;
    //qCDebug(llthemeengine) << "D-Bus system tray:" << (m_dbusTrayAvailable ? "yes" : "no");
    }
  return (m_dbusTrayAvailable ? new QDBusTrayIcon() : nullptr);
}
#endif

const QPalette *lthemeenginePlatformTheme::palette(QPlatformTheme::Palette type) const{
  Q_UNUSED(type);
  return (m_usePalette ? m_customPalette : nullptr);
}

const QFont *lthemeenginePlatformTheme::font(QPlatformTheme::Font type) const{
  if(type == QPlatformTheme::FixedFont){ return &m_fixedFont; }
  return &m_generalFont;
}

QVariant lthemeenginePlatformTheme::themeHint(QPlatformTheme::ThemeHint hint) const{
  switch (hint){
    case QPlatformTheme::CursorFlashTime: return m_cursorFlashTime;
    case MouseDoubleClickInterval: return m_doubleClickInterval;
    case QPlatformTheme::ToolButtonStyle: return m_toolButtonStyle;
    case QPlatformTheme::SystemIconThemeName: return m_iconTheme;
    case QPlatformTheme::StyleNames: return QStringList() << "lthemeengine-style";
    case QPlatformTheme::IconThemeSearchPaths: return lthemeengine::iconPaths();
    case DialogButtonBoxLayout: return m_buttonBoxLayout;
    case QPlatformTheme::UiEffects: return m_uiEffects;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    case QPlatformTheme::WheelScrollLines: return m_wheelScrollLines;
#endif
    default: return QPlatformTheme::themeHint(hint);
    }
}

void lthemeenginePlatformTheme::applySettings(){
  if(!QGuiApplication::desktopSettingsAware()){ return; }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
 if(!m_update){
   //do not override application palette
   if(QCoreApplication::testAttribute(Qt::AA_SetPalette)){
     m_usePalette = false;
     qCDebug(llthemeengine) << "palette support is disabled";
     }
   }
#endif
#ifdef QT_WIDGETS_LIB
  if(hasWidgets()){
    qApp->setFont(m_generalFont);
    //Qt 5.6 or higher should be use themeHint function on application startup.
    //So, there is no need to call this function first time.
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    if(m_update)
      qApp->setWheelScrollLines(m_wheelScrollLines);
#else
      qApp->setWheelScrollLines(m_wheelScrollLines);
#endif
      if(m_update && qApp->style()->objectName() == "lthemeengine-style") /* ignore application style */ { qApp->setStyle("lthemeengine-style"); } //recreate style object
      if(m_update && m_usePalette){
        if(m_customPalette){ qApp->setPalette(*m_customPalette); }
        else{ qApp->setPalette(qApp->style()->standardPalette()); }
        }
        //do not override application style if one is already set by the app itself
      QString orig = qApp->styleSheet();
      if(orig.startsWith(m_oldStyleSheet)){ orig = orig.remove(m_oldStyleSheet); }
      qApp->setStyleSheet(m_userStyleSheet+orig); //make sure the app style has higher priority than ours
      m_oldStyleSheet = m_userStyleSheet;

    }
#endif
  QGuiApplication::setFont(m_generalFont); //apply font
  bool ithemechange = m_iconTheme != QIcon::themeName();
  QIcon::setThemeName(m_iconTheme); //apply icons
  //See if we need to reload the application icon from the new theme
  if(ithemechange){
    QString appIcon = qApp->windowIcon().name();
    if(!appIcon.isEmpty() && QIcon::hasThemeIcon(appIcon)){ qApp->setWindowIcon(QIcon::fromTheme(appIcon)); }
    QWindowList wins = qApp->topLevelWindows();
    for(int i=0; i<wins.length(); i++){
     QString winIcon = wins[i]->icon().name();
      if(!winIcon.isEmpty() && QIcon::hasThemeIcon(winIcon)){ wins[i]->setIcon(QIcon::fromTheme(winIcon)); }
    }
  }
  bool cthemechange = m_cursorTheme != QString(getenv("X_CURSOR_THEME"));
  setenv("X_CURSOR_THEME", m_cursorTheme.toLocal8Bit().data(), 1);
  //qDebug() << "Icon Theme Change:" << m_iconTheme << QIcon::themeSearchPaths();
  if(m_customPalette && m_usePalette){ QGuiApplication::setPalette(*m_customPalette); } //apply palette
#ifdef QT_WIDGETS_LIB
  if(hasWidgets()){
    QEvent et(QEvent::ThemeChange);
    QEvent ec(QEvent::CursorChange);
    foreach (QWidget *w, qApp->allWidgets()){
      if(ithemechange){ QApplication::sendEvent(w, &et); }
      if(cthemechange){ QApplication::sendEvent(w, &ec); }
      }
    }
#endif
  if(!m_update){ m_update = true; }

  //Mouse Cursor syncronization
  /*QString mthemefile = QDir::homePath()+"/.icons/default/index.theme";
  if(!watcher->files().contains(mthemefile) && QFile::exists(mthemefile)){
    watcher->addPath(mthemefile); //X11 mouse cursor theme file
    //qDebug() << "Add Mouse Cursor File to Watcher";
    syncMouseCursorTheme(mthemefile);
  }*/
  //Now cleanup any old palette as needed
  if(outgoingpalette != 0){
    QCoreApplication::processEvents(); //make sure everything switches to the new palette first
    delete outgoingpalette;
  }
}
#ifdef QT_WIDGETS_LIB

void lthemeenginePlatformTheme::createFSWatcher(){
  watcher = new QFileSystemWatcher(this);
  watcher->addPath(lthemeengine::configPath()); //theme engine settings directory
  watcher->addPath(QDir::homePath()+"/.icons/default/index.theme"); //X11 mouse cursor theme file
  QTimer *timer = new QTimer(this);
  timer->setSingleShot(true);
  timer->setInterval(500);
  connect(watcher, SIGNAL(directoryChanged(QString)), timer, SLOT(start()));
  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)) );
  connect(timer, SIGNAL(timeout()), SLOT(updateSettings()));
}

void lthemeenginePlatformTheme::updateSettings(){
  //qCDebug(llthemeengine) << "updating settings..";
  readSettings();
  applySettings();
}
#endif

void lthemeenginePlatformTheme::fileChanged(QString path){
  if(path.endsWith("default/index.theme")){
    //qDebug() << "Mouse Cursor File Changed";
    syncMouseCursorTheme(path);
  }
}

void lthemeenginePlatformTheme::readSettings(){
  outgoingpalette = m_customPalette;
  if(m_customPalette){
    m_customPalette = 0;
  }
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  settings.beginGroup("Appearance");
  m_style = settings.value("style", "Fusion").toString();
  if(settings.value("custom_palette", false).toBool()){
    QString schemePath = settings.value("color_scheme_path","airy").toString();
    m_customPalette = new QPalette(loadColorScheme(schemePath));
  }
  m_cursorTheme = settings.value("cursor_theme","").toString();
  m_iconTheme = settings.value("icon_theme", "material-design-light").toString();
  settings.endGroup();
  settings.beginGroup("Fonts");
  m_generalFont = settings.value("general", QPlatformTheme::font(QPlatformTheme::SystemFont)).value<QFont>();
  m_fixedFont = settings.value("fixed", QPlatformTheme::font(QPlatformTheme::FixedFont)).value<QFont>();
  settings.endGroup();
  settings.beginGroup("Interface");
  m_doubleClickInterval = QPlatformTheme::themeHint(QPlatformTheme::MouseDoubleClickInterval).toInt();
  m_doubleClickInterval = settings.value("double_click_interval", m_doubleClickInterval).toInt();
  m_cursorFlashTime = QPlatformTheme::themeHint(QPlatformTheme::CursorFlashTime).toInt();
  m_cursorFlashTime = settings.value("cursor_flash_time", m_cursorFlashTime).toInt();
  m_buttonBoxLayout = QPlatformTheme::themeHint(QPlatformTheme::DialogButtonBoxLayout).toInt();
  m_buttonBoxLayout = settings.value("buttonbox_layout", m_buttonBoxLayout).toInt();
  QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, !settings.value("menus_have_icons", true).toBool());
  m_toolButtonStyle = settings.value("toolbutton_style", Qt::ToolButtonFollowStyle).toInt();
  m_wheelScrollLines = settings.value("wheel_scroll_lines", 3).toInt();
  //load effects
  m_uiEffects = QPlatformTheme::themeHint(QPlatformTheme::UiEffects).toInt();
    if(settings.childKeys().contains("gui_effects")){
      QStringList effectList = settings.value("gui_effects").toStringList();
      m_uiEffects = 0;
      if(effectList.contains("General")){ m_uiEffects |= QPlatformTheme::GeneralUiEffect; }
      if(effectList.contains("AnimateMenu")){ m_uiEffects |= QPlatformTheme::AnimateMenuUiEffect; }
      if(effectList.contains("FadeMenu")){ m_uiEffects |= QPlatformTheme::FadeMenuUiEffect; }
      if(effectList.contains("AnimateCombo")){ m_uiEffects |= QPlatformTheme::AnimateComboUiEffect; }
      if(effectList.contains("AnimateTooltip")){ m_uiEffects |= QPlatformTheme::AnimateTooltipUiEffect; }
      if(effectList.contains("FadeTooltip")){ m_uiEffects |= QPlatformTheme::FadeTooltipUiEffect; }
      if(effectList.contains("AnimateToolBox")){ m_uiEffects |= QPlatformTheme::AnimateToolBoxUiEffect; }
      }
    //load style sheets
#ifdef QT_WIDGETS_LIB
    QStringList qssPaths;
    if(qApp->applicationFilePath().section("/",-1).startsWith("lumina-desktop") ){ qssPaths << settings.value("desktop_stylesheets").toStringList(); }
    qssPaths << settings.value("stylesheets").toStringList();
    //qDebug() << "Loaded Stylesheets:" << qApp->applicationName() << qssPaths;
    m_userStyleSheet = loadStyleSheets(qssPaths);
#endif
    settings.endGroup();
}

#ifdef QT_WIDGETS_LIB
bool lthemeenginePlatformTheme::hasWidgets(){
  return qobject_cast<QApplication *> (qApp) != nullptr;
}
#endif

QString lthemeenginePlatformTheme::loadStyleSheets(const QStringList &paths){
  //qDebug() << "Loading Stylesheets:" << paths;
  QString content;
  foreach (QString path, paths){
    if(!QFile::exists(path)){ continue; }
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    content.append(file.readAll());
    }
  QRegExp regExp("//.*(\\n|$)");
  regExp.setMinimal(true);
  content.remove(regExp);
  return content;
}

QPalette lthemeenginePlatformTheme::loadColorScheme(QString filePath){
  if(!filePath.contains("/") && !filePath.endsWith(".conf") && !filePath.isEmpty()){
    //relative theme name, auto-complete it
    QStringList dirs;
    dirs << getenv("XDG_CONFIG_HOME");
    dirs << QString(getenv("XDG_CONFIG_DIRS")).split(":");
    dirs << QString(getenv("XDG_DATA_DIRS")).split(":");
    QString relpath = "/lthemeengine/colors/%1.conf";
    relpath = relpath.arg(filePath);
    for(int i=0; i<dirs.length(); i++){
      if(QFile::exists(dirs[i]+relpath)){ filePath = dirs[i]+relpath; break; }
    }
  }

  QPalette customPalette;
  QSettings settings(filePath, QSettings::IniFormat);
  settings.beginGroup("ColorScheme");
  QStringList activeColors = settings.value("active_colors").toStringList();
  QStringList inactiveColors = settings.value("inactive_colors").toStringList();
  QStringList disabledColors = settings.value("disabled_colors").toStringList();
  settings.endGroup();
  if(activeColors.count() <= QPalette::NColorRoles && inactiveColors.count() <= QPalette::NColorRoles && disabledColors.count() <= QPalette::NColorRoles){
    for (int i = 0; i < QPalette::NColorRoles && i<activeColors.count(); i++){
      QPalette::ColorRole role = QPalette::ColorRole(i);
      customPalette.setColor(QPalette::Active, role, QColor(activeColors.at(i)));
      customPalette.setColor(QPalette::Inactive, role, QColor(inactiveColors.at(i)));
      customPalette.setColor(QPalette::Disabled, role, QColor(disabledColors.at(i)));
      }
    }
  else{ customPalette = *QPlatformTheme::palette(SystemPalette); } //load fallback palette
  return customPalette;
}

void lthemeenginePlatformTheme::syncMouseCursorTheme(QString indexfile){
  //Read the index file and pull out the theme name
  QFile file(indexfile);
  QString newtheme;
  if(file.open(QIODevice::ReadOnly)){
    QTextStream stream(&file);
    QString tmp;
    while(!stream.atEnd()){
      tmp = stream.readLine().simplified();
      if(tmp.startsWith("Inherits=")){ newtheme = tmp.section("=",1,-1).simplified(); break; }
    }
    file.close();
  }
  if(newtheme.isEmpty()){ return; } //nothing to do
  QString curtheme = QString(XcursorGetTheme(QX11Info::display()) ); //currently-used theme
  //qDebug() << "Sync Mouse Cursur Theme:" << curtheme << newtheme;
  if(curtheme!=newtheme){
    qDebug() << " - Setting new cursor theme:" << newtheme;
    XcursorSetTheme(QX11Info::display(), newtheme.toLocal8Bit().data()); //save the new theme name
  }else{
    return;
  }
  //qDebug() << "Qt Stats:";
  //qDebug() << " TopLevelWindows:" << QGuiApplication::topLevelWindows().length();
  //qDebug() << " AllWindows:" << QGuiApplication::allWindows().length();
  //qDebug() << " AllWidgets:" << QApplication::allWidgets().length();

  //XcursorSetThemeCore( QX11Info::display(), XcursorGetThemeCore(QX11Info::display()) ); //reset the theme core
  //Load the cursors from the new theme
  int defsize = XcursorGetDefaultSize(QX11Info::display());
  //qDebug() << "Default cursor size:" << defsize;
  XcursorImages *imgs = XcursorLibraryLoadImages("left_ptr", NULL, defsize);
  //qDebug() << "imgs:" << imgs << imgs->nimage;
  XcursorCursors *curs = XcursorImagesLoadCursors(QX11Info::display(), imgs);
  if(curs==0){ return; } //not found
  //qDebug() << "Got Cursors:" << curs->ncursor;
  //Now re-set the cursors for the current top-level X windows
  QWindowList wins = QGuiApplication::allWindows(); //QGuiApplication::topLevelWindows();
  //qDebug() << "Got Windows:" << wins.length();
  for(int i=0; i<curs->ncursor; i++){
    for(int w=0; w<wins.length(); w++){
      XDefineCursor(curs->dpy, wins[w]->winId(), curs->cursors[i]);
    }
  }
  XcursorCursorsDestroy(curs); //finished with this temporary structure

  /*QWidgetList wlist = QApplication::allWidgets();
  qDebug() << "Widget List:" << wlist.length();
  for(int i=0; i<wlist.length(); i++){
    QCursor cur(wlist[i]->cursor().shape());
    wlist[i]->cursor().swap( cur );
  }*/
}
