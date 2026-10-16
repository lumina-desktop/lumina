//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QApplication>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // Set window properties
    this->setWindowTitle(tr("Lumina Control Center"));
    this->setWindowIcon(LXDG::findIcon("preferences-system", ""));
    
    // Setup keyboard shortcut for search
    findShort = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), this, SLOT(showFind()));
    
    // Configure tree widget
    ui->treeWidget->setMouseTracking(true);
    ui->treeWidget->setSortingEnabled(false);
    
    // Connect signals
    connect(ui->treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(itemTriggered(QTreeWidgetItem*, int)));
    connect(ui->treeWidget, SIGNAL(itemPressed(QTreeWidgetItem*,int)), this, SLOT(itemTriggered(QTreeWidgetItem*, int)));
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(searchChanged(QString)));
    
    // Connect menu actions
    connect(ui->actionRefresh, SIGNAL(triggered()), this, SLOT(LoadApplications()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    
    // Load applications
    LoadApplications();
    
    // Set focus to search box
    ui->lineEdit->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotSingleInstance()
{
    this->show();
    this->raise();
    this->activateWindow();
}

void MainWindow::LoadApplications()
{
    INFO.clear();
    
    // Get LCC applications
    QList<APPINFO> lccApps = GetLCCApplications();
    INFO.append(lccApps);
    
    // Get system applications with Settings category
    QList<APPINFO> systemApps = GetSystemApplications();
    INFO.append(systemApps);
    
    // Sort applications by name
    QStringList names;
    for(int i=0; i<INFO.length(); i++){ 
        names << INFO[i].name; 
    }
    names.sort(Qt::CaseInsensitive);
    
    QList<APPINFO> sorted;
    for(int i=0; i<names.length(); i++){
        for(int j=0; j<INFO.length(); j++){
            if(INFO[j].name == names[i]){ 
                sorted << INFO.takeAt(j); 
                break; 
            }
        }
    }
    INFO = sorted;
    
    // Update the display
    UpdateItems(ui->lineEdit->text());
    
    // Show status
    ui->statusbar->showMessage(tr("Loaded %1 configuration tools").arg(INFO.length()), 3000);
}

QList<APPINFO> MainWindow::GetLCCApplications()
{
    QList<APPINFO> list;
    
    // Define LCC application information
    QMap<QString, QStringList> lccInfo;
    lccInfo["lcc-wallpaper"] = QStringList() << tr("Wallpaper") << tr("Wallpaper Settings") << "preferences-desktop-wallpaper" << tr("Change background image(s)") << "appearance" << "background,wallpaper,color,image";
    lccInfo["lcc-autostart"] = QStringList() << tr("Autostart") << tr("Startup Settings") << "preferences-system-session-services" << tr("Automatically start applications or services") << "session" << "apps,autostart,services,xdg,startup,session";
    lccInfo["lcc-defaultapps"] = QStringList() << tr("Applications") << tr("Mimetype Settings") << "preferences-desktop-default-applications" << tr("Change default applications") << "session" << "apps,default,services,xdg,session";
    lccInfo["lcc-keyboard"] = QStringList() << tr("Keyboard Shortcuts") << tr("Keyboard Shortcuts") << "preferences-desktop-keyboard" << tr("Change keyboard shortcuts") << "session" << "apps,fluxbox,keys,keyboard,session,launch";
    lccInfo["lcc-window-manager"] = QStringList() << tr("Window Manager") << tr("Window Settings") << "preferences-system-windows-actions" << tr("Change window settings and appearances") << "appearance" << "window,frame,border,workspace,theme,fluxbox,session";
    lccInfo["lcc-desktop"] = QStringList() << tr("Desktop") << tr("Desktop Plugins") << "preferences-desktop-icons" << tr("Change what icons or tools are embedded on the desktop") << "interface" << "desktop,plugins,embed,icons,utilities";
    lccInfo["lcc-panels"] = QStringList() << tr("Panels") << tr("Panels and Plugins") << "configure-toolbars" << tr("Change any floating panels and what they show") << "interface" << "desktop,toolbar,panel,floating,plugins";
    lccInfo["lcc-menu"] = QStringList() << tr("Menu") << tr("Menu Plugins") << "format-list-unordered" << tr("Change what options are shown on the desktop context menu") << "interface" << "desktop,menu,plugins,shortcuts";
    lccInfo["lcc-locale"] = QStringList() << tr("Localization") << tr("Locale Settings") << "preferences-desktop-locale" << tr("Change the default locale settings for this user") << "user" << "user,locale,language,translations";
    lccInfo["lcc-session"] = QStringList() << tr("General Options") << tr("User Settings") << "configure" << tr("Change basic user settings such as time/date formats") << "user" << "user,settings,time,date,icon,reset,numlock,clock";
    lccInfo["lcc-compositor"] = QStringList() << tr("Window Effects") << tr("Window Effects") << "window-duplicate" << tr("Adjust transparency levels and window effects") << "appearance" << "background,interface,color,transparency,windows,compositing";
    lccInfo["lcc-sound-theme"] = QStringList() << tr("Sound Themeing") << tr("Theme") << "media-playlist-audio" << tr("Change basic sound settings") << "session" << "session,settings,sound,theme";
    lccInfo["lcc-mouse"] = QStringList() << tr("Mouse Settings") << tr("Mouse Settings") << "preferences-desktop-mouse" << tr("Adjust mouse devices") << "user" << "user,speed,accel,mouse";
    lccInfo["lcc-bluetooth"] = QStringList() << tr("Bluetooth Settings") << tr("Bluetooth Settings") << "preferences-desktop-bluetooth" << tr("Setup Bluetooth devices") << "user" << "user,bluetooth,audio";
    
    // Get the base path for LCC executables
    QString lccPath = QApplication::applicationDirPath() + "/../core-utils/lcc";
    
    // Check for each LCC application
    for(auto it = lccInfo.begin(); it != lccInfo.end(); ++it) {
        QString appName = it.key();
        QString dirName = appName;
        dirName.remove("lcc-");
        
        // Try to find the executable in the build directory
        QString execPath = lccPath + "/" + dirName + "/" + appName;
        if(QFile::exists(execPath)) {
            QStringList info = it.value();
            list << CreateAppInfo(execPath, info[0], info[1], info[2], info[3], info[4], info[5].split(","), true);
        } else {
            // Try system PATH
            QStringList info = it.value();
            list << CreateAppInfo(appName, info[0], info[1], info[2], info[3], info[4], info[5].split(","), true);
        }
    }
    
    return list;
}

QList<APPINFO> MainWindow::GetSystemApplications()
{
    QList<APPINFO> list;
    
    // Scan for desktop files with Settings category
    QStringList dirs;
    dirs << "/usr/share/applications" << "/usr/local/share/applications" << QDir::homePath()+"/.local/share/applications";
    
    QStringList desktopFiles;
    for(const QString &dir : dirs) {
        if(QDir(dir).exists()) {
            QDir appDir(dir);
            QStringList files = appDir.entryList(QStringList() << "*.desktop", QDir::Files);
            for(const QString &file : files) {
                desktopFiles << appDir.absoluteFilePath(file);
            }
        }
    }
    
    for(const QString &filePath : desktopFiles) {
        XDGDesktop desk(filePath);
        if(desk.isValid() && !desk.isHidden) {
            // Check if it has Settings category and is not an LCC app
            if(desk.catList.contains("Settings") && !desk.filePath.contains("lcc-") && !desk.filePath.endsWith("lumina-config.desktop")) {
                QString category = "system";
                if(desk.catList.contains("DesktopSettings")) category = "interface";
                else if(desk.catList.contains("HardwareSettings")) category = "user";
                
                list << CreateAppInfo(desk.filePath, desk.name, desk.genericName.isEmpty() ? desk.name : desk.genericName, desk.icon, desk.comment, category, desk.keyList, false);
            }
        }
    }
    
    return list;
}

APPINFO MainWindow::CreateAppInfo(QString id, QString name, QString title, QString icon, QString comment, QString category, QStringList tags, bool isLCC)
{
    APPINFO app;
    app.id = id;
    app.name = name;
    app.title = title;
    app.icon = icon;
    app.comment = comment;
    app.category = category;
    app.search_tags = tags;
    app.isLCC = isLCC;
    return app;
}

void MainWindow::UpdateItems(QString search)
{
    ui->treeWidget->clear();
    ui->treeWidget->setColumnCount(2);
    
    // Create categories
    QTreeWidgetItem *interface = new QTreeWidgetItem();
    interface->setIcon(0, LXDG::findIcon("preferences-desktop",""));
    interface->setText(0, tr("Interface Configuration"));
    interface->setFirstColumnSpanned(true);
    
    QTreeWidgetItem *appearance = new QTreeWidgetItem();
    appearance->setIcon(0, LXDG::findIcon("preferences-desktop-color",""));
    appearance->setText(0, tr("Appearance"));
    appearance->setFirstColumnSpanned(true);
    
    QTreeWidgetItem *session = new QTreeWidgetItem();
    session->setIcon(0, LXDG::findIcon("preferences-system-session-services",""));
    session->setText(0, tr("Desktop Defaults"));
    session->setFirstColumnSpanned(true);
    
    QTreeWidgetItem *user = new QTreeWidgetItem();
    user->setIcon(0, LXDG::findIcon("preferences-desktop-user",""));
    user->setText(0, tr("User Settings"));
    user->setFirstColumnSpanned(true);
    
    QTreeWidgetItem *system = new QTreeWidgetItem();
    system->setIcon(0, LXDG::findIcon("preferences-system",""));
    system->setText(0, tr("System Settings"));
    system->setFirstColumnSpanned(true);
    
    // Filter applications based on search
    QStringList searchList = search.split(" ");
    for(int i=0; i<INFO.length(); i++){
        if(!search.isEmpty()) {
            // Check if this item matches the search
            QStringList info;
            info << INFO[i].name.split(" ") << INFO[i].title.split(" ") << INFO[i].comment.split(" ") << INFO[i].search_tags;
            info.removeDuplicates();
            
            bool matches = true;
            for(int s=0; s<searchList.length() && matches; s++){
                matches = !info.filter(searchList[s], Qt::CaseInsensitive).isEmpty();
            }
            if(!matches) continue;
        }
        
        // Determine which category to add to
        QTreeWidgetItem *category = system; // default
        if(INFO[i].category == "interface") category = interface;
        else if(INFO[i].category == "appearance") category = appearance;
        else if(INFO[i].category == "session") category = session;
        else if(INFO[i].category == "user") category = user;
        
        // Find if we can add to existing item (2 columns)
        int col = 0;
        QTreeWidgetItem *lastIt = 0;
        if(category->childCount() > 0) {
            QTreeWidgetItem *last = category->child(category->childCount()-1);
            if(last->text(1).isEmpty()) {
                lastIt = last;
                col = 1;
            }
        }
        
        if(lastIt == 0) {
            lastIt = new QTreeWidgetItem();
            col = 0;
        }
        
        lastIt->setIcon(col, LXDG::findIcon(INFO[i].icon, ""));
        lastIt->setText(col, INFO[i].name);
        lastIt->setStatusTip(col, INFO[i].comment);
        lastIt->setToolTip(col, INFO[i].comment);
        lastIt->setWhatsThis(col, INFO[i].id);
        
        if(col == 0) {
            category->addChild(lastIt);
        }
    }
    
    // Add categories to tree widget if they have children
    if(interface->childCount() > 0) { ui->treeWidget->addTopLevelItem(interface); interface->setExpanded(true); }
    if(appearance->childCount() > 0) { ui->treeWidget->addTopLevelItem(appearance); appearance->setExpanded(true); }
    if(session->childCount() > 0) { ui->treeWidget->addTopLevelItem(session); session->setExpanded(true); }
    if(user->childCount() > 0) { ui->treeWidget->addTopLevelItem(user); user->setExpanded(true); }
    if(system->childCount() > 0) { ui->treeWidget->addTopLevelItem(system); system->setExpanded(true); }
    
    ui->treeWidget->sortItems(0, Qt::AscendingOrder);
    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->resizeColumnToContents(1);
}

void MainWindow::showFind()
{
    ui->lineEdit->setFocus();
    ui->lineEdit->selectAll();
}

void MainWindow::itemTriggered(QTreeWidgetItem *it, int col)
{
    if(it->childCount() > 0) {
        // Category item - expand/collapse
        it->setExpanded(!it->isExpanded());
        it->setSelected(false);
    } else if(!it->whatsThis(col).isEmpty()) {
        QString id = it->whatsThis(col);
        
        if(id.endsWith(".desktop")) {
            // Desktop file - launch with lumina-open
            QProcess::startDetached("lumina-open", QStringList() << id);
            ui->statusbar->showMessage(tr("Launching %1...").arg(it->text(col)), 2000);
        } else {
            // Executable - launch directly
            QProcess::startDetached(id, QStringList());
            ui->statusbar->showMessage(tr("Launching %1...").arg(it->text(col)), 2000);
        }
    } else {
        it->setSelected(false);
    }
}

void MainWindow::searchChanged(QString txt)
{
    UpdateItems(txt.simplified());
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("About Lumina Control Center"),
        tr("<h3>Lumina Control Center</h3>"
           "<p>The Lumina Control Center provides easy access to all system configuration utilities.</p>"
           "<p>Copyright (c) 2024, jt(q5sys)<br>"
           "Available under the 3-clause BSD license</p>"
           "<p>Part of the Lumina Control Center (LCC) suite.</p>"));
}
