//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainWindow.h"
#include "ui_MainWindow.h"

//==========
//    LPlugins Implementation
//==========
LPlugins::LPlugins(){
    LoadDesktopPlugins();
}

LPlugins::~LPlugins(){
}

QStringList LPlugins::desktopPlugins(){
    return DESKTOP.keys();
}

LPI LPlugins::desktopPluginInfo(QString plug){
    return DESKTOP.value(plug, LPI());
}

void LPlugins::LoadDesktopPlugins(){
    DESKTOP.clear();
    
    // Calendar plugin
    LPI info;
    info.ID = "calendar";
    info.name = "Calendar";
    info.description = "Display a calendar on the desktop";
    info.icon = "view-calendar";
    DESKTOP.insert(info.ID, info);
    
    // Application Launcher plugin
    info = LPI();
    info.ID = "applauncher";
    info.name = "Application Launcher";
    info.description = "Launch applications from the desktop";
    info.icon = "application-x-executable";
    DESKTOP.insert(info.ID, info);
    
    // Desktop View plugin
    info = LPI();
    info.ID = "desktopview";
    info.name = "Desktop Icons View";
    info.description = "Show desktop folder contents";
    info.icon = "user-desktop";
    DESKTOP.insert(info.ID, info);
    
    // System Monitor plugin
    info = LPI();
    info.ID = "systemmonitor";
    info.name = "System Monitor";
    info.description = "Display system resource usage";
    info.icon = "utilities-system-monitor";
    DESKTOP.insert(info.ID, info);
    
    // Note Pad plugin
    info = LPI();
    info.ID = "notepad";
    info.name = "Note Pad";
    info.description = "Simple text note widget";
    info.icon = "accessories-text-editor";
    DESKTOP.insert(info.ID, info);
    
    // System Tray plugin
    info = LPI();
    info.ID = "systemtray";
    info.name = "System Tray";
    info.description = "System notification area";
    info.icon = "preferences-desktop-notification";
    DESKTOP.insert(info.ID, info);
}

//==========
//    GetPluginDialog Implementation
//==========
GetPluginDialog::GetPluginDialog(QWidget* parent) : QDialog(parent){
    selected = false;
    
    setWindowTitle("Select Plugin");
    setModal(true);
    resize(400, 300);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    listWidget = new QListWidget(this);
    layout->addWidget(listWidget);
    
    descLabel = new QLabel("Select a plugin from the list above", this);
    descLabel->setWordWrap(true);
    descLabel->setMinimumHeight(60);
    layout->addWidget(descLabel);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);
    
    connect(listWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(pluginchanged()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

GetPluginDialog::~GetPluginDialog(){
}

void GetPluginDialog::LoadPlugins(QString type, LPlugins *DB){
    listWidget->clear();
    if(type == "desktop"){
        QStringList plugs = DB->desktopPlugins();
        for(int i=0; i<plugs.length(); i++){
            LPI info = DB->desktopPluginInfo(plugs[i]);
            QListWidgetItem *item = new QListWidgetItem();
            item->setText(info.name);
            item->setData(Qt::UserRole, info.ID);
            item->setToolTip(info.description);
            item->setIcon(LXDG::findIcon(info.icon, ""));
            listWidget->addItem(item);
        }
    }
    if(listWidget->count() > 0){
        listWidget->setCurrentRow(0);
    }
}

void GetPluginDialog::pluginchanged(){
    QListWidgetItem *item = listWidget->currentItem();
    if(item != 0){
        descLabel->setText(item->toolTip());
        plugID = item->data(Qt::UserRole).toString();
    }
}

void GetPluginDialog::accept(){
    QListWidgetItem *item = listWidget->currentItem();
    if(item != 0){
        selected = true;
        plugID = item->data(Qt::UserRole).toString();
    }
    QDialog::accept();
}

//==========
//    AppDialog Implementation
//==========
AppDialog::AppDialog(QWidget *parent, QString defaultPath) : QDialog(parent){
    appreset = false;
    appselected = "";
    
    setWindowTitle("Select Application");
    setModal(true);
    resize(500, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Search box
    lineSearch = new QLineEdit(this);
    lineSearch->setPlaceholderText("Search applications...");
    layout->addWidget(lineSearch);
    
    // Application list
    listApps = new QListWidget(this);
    layout->addWidget(listApps);
    
    // Button box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);
    
    // Load applications using local desktop file scanning
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
    
    QListWidgetItem *defaultItem = 0;
    for(const QString &filePath : desktopFiles) {
        XDGDesktop desk(filePath);
        if(desk.isValid() && !desk.isHidden) {
            QListWidgetItem *app = new QListWidgetItem(LXDG::findIcon(desk.icon,"application-x-executable"), desk.name);
            app->setData(Qt::UserRole, desk.filePath);
            listApps->addItem(app);
            if(desk.filePath == defaultPath){
                defaultItem = app;
            }
        }
    }
    
    if(listApps->count()){
        listApps->setCurrentItem(defaultItem != 0 ? defaultItem : listApps->item(0));
    }
    
    // Connect signals
    connect(lineSearch, SIGNAL(textChanged(const QString&)), this, SLOT(on_lineSearch_textChanged(const QString&)));
    connect(listApps, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(on_listApps_itemDoubleClicked(QListWidgetItem*)));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(on_buttonBox_accepted()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(on_buttonBox_rejected()));
    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(on_buttonBox_clicked(QAbstractButton*)));
}

AppDialog::~AppDialog(){
}

void AppDialog::allowReset(bool allow){
    if(allow){
        buttonBox->setStandardButtons(QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    }else{
        buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    }
}

void AppDialog::on_buttonBox_accepted(){
    QListWidgetItem *item = listApps->currentItem();
    if(item != 0){
        appselected = item->data(Qt::UserRole).toString();
    }
    accept();
}

void AppDialog::on_buttonBox_rejected(){
    reject();
}

void AppDialog::on_buttonBox_clicked(QAbstractButton *button){
    if(buttonBox->standardButton(button) == QDialogButtonBox::RestoreDefaults){
        appreset = true;
        accept();
    }
}

void AppDialog::on_listApps_itemDoubleClicked(QListWidgetItem *item){
    appselected = item->data(Qt::UserRole).toString();
    accept();
}

void AppDialog::on_lineSearch_textChanged(const QString &term){
    QListWidgetItem *first_visible = 0;
    for(int i = 0; i < listApps->count(); i++){
        QListWidgetItem *item = listApps->item(i);
        bool visible = item->text().contains(term, Qt::CaseInsensitive);
        item->setHidden(!visible);
        if(visible && first_visible == 0){
            first_visible = item;
        }
    }
    //Select the first app
    listApps->setCurrentItem(first_visible);
    if(first_visible != 0){
        listApps->scrollToItem(first_visible);
    }
}

//==========
//    MainWindow Implementation
//==========
MainWindow::MainWindow(int screen, QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    cscreen = screen;
    loading = false;
    PINFO = new LPlugins();
    
    // Setup window
    this->setWindowTitle("Desktop Settings");
    this->setWindowIcon(LXDG::findIcon("preferences-desktop", ""));
    
    // Setup menus
    setupMenus();
    
    // Connect signals
    connect(ui->tool_desktop_addplugin, SIGNAL(clicked()), this, SLOT(deskplugadded()));
    connect(ui->tool_desktop_rmplugin, SIGNAL(clicked()), this, SLOT(deskplugremoved()));
    connect(ui->check_desktop_autolaunchers, SIGNAL(clicked()), this, SLOT(settingChanged()));
    connect(ui->check_media_icons, SIGNAL(clicked()), this, SLOT(settingChanged()));
    
    // Load settings
    updateIcons();
    LoadSettings();
}

MainWindow::~MainWindow(){
    delete PINFO;
}

void MainWindow::setupMenus(){
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    QAction *saveAction = fileMenu->addAction("&Save Settings");
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setIcon(LXDG::findIcon("document-save", ""));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(SaveSettings()));
    
    fileMenu->addSeparator();
    
    QAction *quitAction = fileMenu->addAction("&Quit");
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setIcon(LXDG::findIcon("application-exit", ""));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    
    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    
    QAction *aboutAction = helpMenu->addAction("&About");
    aboutAction->setIcon(LXDG::findIcon("help-about", ""));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));
}

void MainWindow::SaveSettings(){
    if(loading) return;
    
    QSettings settings("lumina-desktop","desktopsettings");
    QString screenID = QApplication::screens().at(cscreen)->name();
    QString DPrefix = "desktop-"+screenID+"/";

    settings.setValue(DPrefix+"generateDesktopIcons", ui->check_desktop_autolaunchers->isChecked());
    settings.setValue(DPrefix+"generateMediaIcons", ui->check_media_icons->isChecked());
    
    QStringList plugs;
    for(int i=0; i<ui->list_desktop_plugins->count(); i++){
        plugs << ui->list_desktop_plugins->item(i)->whatsThis();
    }
    settings.setValue(DPrefix+"pluginlist", plugs);
    
    statusBar()->showMessage("Settings saved successfully", 2000);
    
    // Reload settings after a moment to get any changes from the desktop
    QTimer::singleShot(1000, this, SLOT(LoadSettings()));
}

void MainWindow::LoadSettings(){
    loading = true;
    
    QSettings settings("lumina-desktop","desktopsettings");
    QString screenID = QApplication::screens().at(cscreen)->name();
    QString DPrefix = "desktop-"+screenID+"/";
    
    ui->check_desktop_autolaunchers->setChecked(settings.value(DPrefix+"generateDesktopIcons",false).toBool());
    ui->check_media_icons->setChecked(settings.value(DPrefix+"generateMediaIcons",true).toBool());
    
    QStringList dplugs = settings.value(DPrefix+"pluginlist",QStringList()).toStringList();
    ui->list_desktop_plugins->clear();
    
    for(int i=0; i<dplugs.length(); i++){
        QListWidgetItem* it = new QListWidgetItem();
        it->setWhatsThis(dplugs[i]); //save the full thing instantly
        
        //Now load the rest of the info about the plugin
        QString num;
        QString pluginID = dplugs[i];
        if(dplugs[i].contains("---")){ 
            num = dplugs[i].section("---",1,1).section(".",1,1).simplified(); //Skip the screen number
            if(num=="1"){ num.clear(); } //don't bother showing the number
            pluginID = dplugs[i].section("---",0,0);
        }
        
        if(pluginID.startsWith("applauncher::")){
            XDGDesktop app(pluginID.section("::",1,50));
            if(app.type == XDGDesktop::BAD){ continue; } //invalid for some reason
            //Now fill the item with the necessary info
            it->setText(app.name);
            it->setIcon(LXDG::findIcon(app.icon,""));
            it->setToolTip(app.comment);
        }else{
            //Load the info for this plugin
            LPI info = PINFO->desktopPluginInfo(pluginID);
            if( info.ID.isEmpty() ){ continue; } //invalid plugin for some reason
            it->setText(info.name);
            it->setToolTip(info.description);
            it->setIcon( LXDG::findIcon(info.icon,"") );
        }
        if(!num.isEmpty()){ it->setText( it->text()+" ("+num+")"); } //append the number
        ui->list_desktop_plugins->addItem(it);
    }
    
    loading = false;
}

void MainWindow::updateIcons(){
    ui->tool_desktop_addplugin->setIcon( LXDG::findIcon("list-add","") );
    ui->tool_desktop_rmplugin->setIcon( LXDG::findIcon("list-remove","") );
}

QString MainWindow::getSysApp(bool allowreset){
    AppDialog dlg(this);
    dlg.allowReset(allowreset);
    dlg.exec();
    if(dlg.appreset && allowreset){
        return "reset";
    }else{
        return dlg.appselected;
    }
}

void MainWindow::deskplugadded(){
    GetPluginDialog dlg(this);
    dlg.LoadPlugins("desktop", PINFO);
    dlg.exec();
    if( !dlg.selected ){ return; } //cancelled
    
    QString newplug = dlg.plugID;
    QListWidgetItem *it = new QListWidgetItem();
    
    if(newplug=="applauncher"){
        //Prompt for the application to add
        QString app = getSysApp();
        if(app.isEmpty()){ return; } //cancelled
        newplug.append("::"+app);
        XDGDesktop desk(app);
        //Now fill the item with the necessary info
        it->setWhatsThis(newplug);
        it->setText(desk.name);
        it->setIcon(LXDG::findIcon(desk.icon,"") );
        it->setToolTip(desk.comment);
    }else{
        //Load the info for this plugin
        LPI info = PINFO->desktopPluginInfo(newplug);
        if( info.ID.isEmpty() ){ return; } //invalid plugin for some reason (should never happen)
        it->setWhatsThis(newplug);
        it->setText(info.name);
        it->setToolTip(info.description);
        it->setIcon( LXDG::findIcon(info.icon,"") );
    }
    
    ui->list_desktop_plugins->addItem(it);
    ui->list_desktop_plugins->scrollToItem(it);
    settingChanged();
}

void MainWindow::deskplugremoved(){
    QList<QListWidgetItem*> sel = ui->list_desktop_plugins->selectedItems();
    if(sel.isEmpty()){ return; } //nothing to do
    for(int i=0; i<sel.length(); i++){
        delete sel[i];
    }
    settingChanged();
}

void MainWindow::settingChanged(){
    if(!loading){
        SaveSettings();
    }
}

void MainWindow::showAbout(){
    QMessageBox::about(this, "About Desktop Settings",
        "Desktop Settings\n\n"
        "Configure desktop plugins and embedded utilities for the Lumina Desktop Environment.\n\n"
        "Copyright (c) 2024, jt(q5sys)\n"
        "Available under the 3-clause BSD license");
}
