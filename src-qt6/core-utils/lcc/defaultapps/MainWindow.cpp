//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

//==========
//    AppDialog Implementation
//==========
AppDialog::AppDialog(QWidget *parent, QString defaultPath) : QDialog(parent) {
    this->setWindowTitle(tr("Select Application"));
    this->setWindowIcon(LXDG::findIcon("system-search",""));
    this->resize(348, 300);
    
    appreset = false;
    
    // Create layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Create search line edit
    lineSearch = new QLineEdit(this);
    lineSearch->setPlaceholderText(tr("Search applications..."));
    layout->addWidget(lineSearch);
    
    // Create list widget
    listApps = new QListWidget(this);
    layout->addWidget(listApps);
    
    // Create button box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults, Qt::Horizontal, this);
    layout->addWidget(buttonBox);
    
    // Populate applications list
    listApps->clear();
    QListWidgetItem *defaultItem = 0;
    
    // Get all desktop files from standard directories
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
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(on_buttonBox_accepted()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(on_buttonBox_rejected()));
    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(on_buttonBox_clicked(QAbstractButton*)));
    connect(listApps, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(on_listApps_itemDoubleClicked(QListWidgetItem*)));
    connect(lineSearch, SIGNAL(textChanged(QString)), this, SLOT(on_lineSearch_textChanged(QString)));
    
    if(parent!=0){
        QWidget *top = parent;
        while(!top->isWindow()){ top = top->parentWidget(); }
        QPoint center = top->geometry().center();
        this->move(center.x()-(this->width()/2), center.y()-(this->height()/2) );
    }
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
    this->close();
}

void AppDialog::on_buttonBox_rejected(){
    this->close();
}

void AppDialog::on_buttonBox_clicked(QAbstractButton *button){
    if(buttonBox->standardButton(button) == QDialogButtonBox::RestoreDefaults){
        appreset = true;
        this->close();
    }
}

void AppDialog::on_listApps_itemDoubleClicked(QListWidgetItem *item){
    appselected = item->data(Qt::UserRole).toString();
    this->close();
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
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    connect(ui->tool_default_filemanager, SIGNAL(clicked()), this, SLOT(changeDefaultFileManager()) );
    connect(ui->tool_default_terminal, SIGNAL(clicked()), this, SLOT(changeDefaultTerminal()) );
    connect(ui->tool_default_webbrowser, SIGNAL(clicked()), this, SLOT(changeDefaultBrowser()) );
    connect(ui->tool_default_email, SIGNAL(clicked()), this, SLOT(changeDefaultEmail()) );
    connect(ui->tool_defaults_clear, SIGNAL(clicked()), this, SLOT(cleardefaultitem()) );
    connect(ui->tool_defaults_set, SIGNAL(clicked()), this, SLOT(setdefaultitem()) );
    connect(ui->tool_defaults_setbin, SIGNAL(clicked()), this, SLOT(setdefaultbinary()) );
    connect(ui->tree_defaults, SIGNAL(itemSelectionChanged()), this, SLOT(checkdefaulticons()) );
    
    updateIcons();
    ui->tabWidget_apps->setCurrentWidget(ui->tab_auto);
    
    // Load settings on startup
    LoadSettings();
}

MainWindow::~MainWindow(){
    delete ui;
}

//================
//    PUBLIC SLOTS
//================
void MainWindow::updateIcons(){
    ui->tool_defaults_clear->setIcon( LXDG::findIcon("edit-clear","") );
    ui->tool_defaults_set->setIcon( LXDG::findIcon("system-run","") );
    ui->tool_defaults_setbin->setIcon( LXDG::findIcon("application-x-executable","") );
    ui->tabWidget_apps->setTabIcon( ui->tabWidget_apps->indexOf(ui->tab_auto), LXDG::findIcon("system-run", "") );
    ui->tabWidget_apps->setTabIcon( ui->tabWidget_apps->indexOf(ui->tab_defaults), LXDG::findIcon("preferences-desktop-filetype-association", "") );
}

//=================
//         PRIVATE
//=================
QString MainWindow::getSysApp(bool allowreset, QString defaultPath){
    AppDialog dlg(this, defaultPath);
    dlg.allowReset(allowreset);
    dlg.exec();
    if(dlg.appreset && allowreset){
        return "reset";
    }else{
        return dlg.appselected;
    }
}

void MainWindow::updateDefaultButton(QToolButton *button, QString app){
    if( !QFile::exists(app) && !LUtils::isValidBinary(app) ){ qDebug() << "Invalid Settings:" << app; app.clear(); } //invalid settings
    if(app.endsWith(".desktop")){
        XDGDesktop file(app);
        if(file.type == XDGDesktop::BAD){
            //Might be a binary - just print out the raw "path"
            button->setText(app.section("/",-1));
            button->setIcon( LXDG::findIcon("application-x-executable","") );
        }else{
            button->setText(file.name);
            button->setIcon(LXDG::findIcon(file.icon,"") );
        }
    }else if(app.isEmpty()){
        button->setText(tr("Click to Set"));
        button->setIcon( LXDG::findIcon("system-help","") );
    }else{
        //Might be a binary - just print out the raw "path"
        button->setText(app.section("/",-1));
        button->setIcon( LXDG::findIcon("application-x-executable","") );
    }
}

void MainWindow::LoadSettings(){
    this->setWindowTitle(tr("Default Applications"));

    //First load the lumina-open specific defaults
    //  - Default File Manager
    defaultFileManager = LXDG::findDefaultAppForMime("inode/directory"); 
    if(defaultFileManager.isEmpty()){ defaultFileManager = "lumina-fm"; }
    updateDefaultButton(ui->tool_default_filemanager, defaultFileManager);

    // - Default Terminal
    defaultTerminal = LXDG::findDefaultAppForMime("application/terminal");
    updateDefaultButton(ui->tool_default_terminal, defaultTerminal);

    // - Default Web Browser
    defaultBrowser = LXDG::findDefaultAppForMime("x-scheme-handler/http");
    updateDefaultButton(ui->tool_default_webbrowser, defaultBrowser);

    // - Default Email Client
    defaultEmail = LXDG::findDefaultAppForMime("application/email");
    updateDefaultButton(ui->tool_default_email, defaultEmail);
    
    //Now load the XDG mime defaults
    ui->tree_defaults->clear();
    QStringList defMimeList = LXDG::listFileMimeDefaults();
    defMimeList.sort(); //sort by group/mime
    //Now fill the tree by group/mime
    QTreeWidgetItem *group = 0;
    QString ccat;
    for(int i=0; i<defMimeList.length(); i++){
        //Get the info from this entry
        QString mime = defMimeList[i].section("::::",0,0);
        QString cat = mime.section("/",0,0);
        QString extlist = defMimeList[i].section("::::",1,1);
        QString def = defMimeList[i].section("::::",2,2);
        QString comment = defMimeList[i].section("::::",3,50);
        //Now check if this is a new category
        if(group == 0 || ccat!=cat){
            //New group
            group = new QTreeWidgetItem(0);
            group->setText(0, cat); //add translations for known/common groups later
            ui->tree_defaults->addTopLevelItem(group);
            ccat = cat;
        }
        //Now create the entry
        QTreeWidgetItem *it = new QTreeWidgetItem();
        it->setWhatsThis(0,mime); // full mimetype
        it->setText(0, QString(tr("%1 (%2)")).arg(mime.section("/",-1), extlist) );
        it->setText(2,comment);
        it->setToolTip(0, comment); it->setToolTip(1,comment);
        //Now load the default (if there is one)
        it->setWhatsThis(1,def); //save for later
        it->setData(1, Qt::UserRole, def);
        if(def.endsWith(".desktop")){
            XDGDesktop file(def);
            if(file.type == XDGDesktop::BAD){
                //Might be a binary - just print out the raw "path"
                it->setText(1, def.section("/",-1));
                it->setIcon(1, LXDG::findIcon("application-x-executable","") );
            }else{
                it->setText(1, file.name);
                it->setIcon(1, LXDG::findIcon(file.icon,"") );
            }
        }else if(!def.isEmpty()){
            //Binary/Other default
            it->setText(1, def.section("/",-1));
            it->setIcon(1, LXDG::findIcon("application-x-executable","") );
        }
        group->addChild(it);
    }
    
    ui->tree_defaults->sortItems(0,Qt::AscendingOrder);
    checkdefaulticons();
}

//=================
//    PRIVATE SLOTS
//=================
void MainWindow::changeDefaultBrowser(){
    //Prompt for the new app
    QString app = getSysApp(true, defaultBrowser);
    if(app.isEmpty()){ return; }//nothing selected
    if(app=="reset"){ app.clear(); }
    //save the new app setting and adjust the button appearance
    defaultBrowser = app;
    LXDG::setDefaultAppForMime("x-scheme-handler/http", app.section("/",-1));
    LXDG::setDefaultAppForMime("x-scheme-handler/https", app.section("/",-1));
    updateDefaultButton(ui->tool_default_webbrowser, app);
}

void MainWindow::changeDefaultEmail(){
    //Prompt for the new app
    QString app = getSysApp(true, defaultEmail);
    if(app.isEmpty()){ return; }//nothing selected
    if(app=="reset"){ app.clear(); }
    //save the new app setting and adjust the button appearance
    defaultEmail = app;
    LXDG::setDefaultAppForMime("application/email",app.section("/",-1));
    updateDefaultButton(ui->tool_default_email, app);
}

void MainWindow::changeDefaultFileManager(){
    //Prompt for the new app
    QString app = getSysApp(true, defaultFileManager);
    if(app.isEmpty()){ return; }//nothing selected
    if(app=="reset"){ app = "lumina-fm"; }
    //save the new app setting and adjust the button appearance
    defaultFileManager = app;
    LXDG::setDefaultAppForMime("inode/directory", app.section("/",-1));
    updateDefaultButton(ui->tool_default_filemanager, app);
}

void MainWindow::changeDefaultTerminal(){
    //Prompt for the new app
    QString app = getSysApp(true, defaultTerminal);
    if(app.isEmpty()){ return; }//nothing selected
    if(app=="reset"){ app = "xterm"; }
    //save the new app setting and adjust the button appearance
    defaultTerminal = app;
    LXDG::setDefaultAppForMime("application/terminal", app.section("/",-1) );
    updateDefaultButton(ui->tool_default_terminal, app);
}

void MainWindow::cleardefaultitem(){
    QTreeWidgetItem *it = ui->tree_defaults->currentItem();
    if(it==0){ return; } //no item selected
    QList<QTreeWidgetItem*> list;
    for(int i=0; i<it->childCount(); i++){
        list << it->child(i);
    }
    if(list.isEmpty()){ list << it; } //just do the current item
    //Now clear the items
    for(int i=0; i<list.length(); i++){
        //Clear it in the back end
        LXDG::setDefaultAppForMime(list[i]->whatsThis(0), "");
        //Now clear it in the UI
        list[i]->setWhatsThis(1,""); //clear the app path
        list[i]->setIcon(1,QIcon()); //clear the icon
        list[i]->setText(1,""); //clear the name
    }
}

void MainWindow::setdefaultitem(){
    QTreeWidgetItem *it = ui->tree_defaults->currentItem();
    QString path;
    if(it==0){ return; } //no item selected
    QList<QTreeWidgetItem*> list;
    for(int i=0; i<it->childCount(); i++){
        list << it->child(i);
    }
    if(list.isEmpty()){
        //just do the current item
        list << it;
        path = it->data(1, Qt::UserRole).toString();
    }
    //Prompt for which application to use
    QString app = getSysApp(false, path); //no "reset"  option
    if(app.isEmpty()){ return; }//nothing selected
    //Now set the items
    for(int i=0; i<list.length(); i++){
        //Set it in the back end
        LXDG::setDefaultAppForMime(list[i]->whatsThis(0), app);
        //Set it in the UI
        XDGDesktop desk(app);
        list[i]->setWhatsThis(1,app); //app path
        list[i]->setIcon(1,LXDG::findIcon(desk.icon,"")); //reset the icon
        list[i]->setText(1,desk.name); //reset the name
        list[i]->setData(1, Qt::UserRole, app);
    }
}

void MainWindow::setdefaultbinary(){
    QTreeWidgetItem *it = ui->tree_defaults->currentItem();
    if(it==0){ return; } //no item selected
    QList<QTreeWidgetItem*> list;
    for(int i=0; i<it->childCount(); i++){
        list << it->child(i);
    }
    if(list.isEmpty()){ list << it; } //just do the current item
    //Prompt for which binary to use
    QFileDialog dlg(this);
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setDirectory( LOS::AppPrefix()+"bin" );
    dlg.setWindowTitle(tr("Select Binary"));
    if( !dlg.exec() || dlg.selectedFiles().isEmpty() ){
        return; //cancelled
    }
    QString path = dlg.selectedFiles().first();
    //Make sure it is executable
    if( !QFileInfo(path).isExecutable()){
        QMessageBox::warning(this, tr("Invalid Binary"), tr("The selected binary is not executable!"));
        return;
    }
    //Now set the items
    for(int i=0; i<list.length(); i++){
        //Set it in the back end
        LXDG::setDefaultAppForMime(list[i]->whatsThis(0), path);
        //Set it in the UI
        list[i]->setWhatsThis(1,path); //app path
        list[i]->setIcon(1,LXDG::findIcon("application-x-executable","")); //clear the icon
        list[i]->setText(1,path.section("/",-1)); //clear the name
    }
}

void MainWindow::checkdefaulticons(){
    QTreeWidgetItem *it = ui->tree_defaults->currentItem();
    ui->tool_defaults_set->setEnabled(it!=0);
    ui->tool_defaults_clear->setEnabled(it!=0);
    ui->tool_defaults_setbin->setEnabled(it!=0);
}
