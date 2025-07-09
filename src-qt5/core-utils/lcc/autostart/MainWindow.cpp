//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainWindow.h"
#include "ui_MainWindow.h"

//==========
//    AppDialog Implementation
//==========
AppDialog::AppDialog(QWidget *parent, QString defaultPath) : QDialog(parent) {
    this->setWindowTitle(tr("Select Application"));
    this->setWindowIcon(LXDG::findIcon("system-search",""));
    this->resize(400, 300);
    
    appreset = false;
    appsList = new XDGDesktopList(this);
    
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
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
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

AppDialog::~AppDialog(){}

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
    loading = false;
    
    this->setWindowTitle(tr("Startup Services"));
    this->setWindowIcon(LXDG::findIcon("preferences-system-startup",""));
    
    setupUI();
    updateIcons();
    LoadSettings();
    
    // Connect signals
    connect(ui->tool_session_addapp, SIGNAL(clicked()), this, SLOT(addsessionstartapp()));
    connect(ui->tool_session_addbin, SIGNAL(clicked()), this, SLOT(addsessionstartbin()));
    connect(ui->tool_session_addfile, SIGNAL(clicked()), this, SLOT(addsessionstartfile()));
    connect(ui->list_session_start, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(settingChanged()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::setupUI(){
    ui->list_session_start->setMouseTracking(true);
    
    // Add context menu for removing items
    ui->list_session_start->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->list_session_start, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(rmsessionstartitem()));
}

//================
//    PUBLIC SLOTS
//================
void MainWindow::SaveSettings(){
    //qDebug() << "Load AutoStart Files for saving";
    QList<XDGDesktop*> STARTAPPS = LXDG::findAutoStartFiles(true); //also want invalid/disabled items
    //qDebug() << " - done";
    //bool newstartapps = false;
    for(int i=0; i<ui->list_session_start->count(); i++){
        QString file = ui->list_session_start->item(i)->whatsThis();
        bool enabled = ui->list_session_start->item(i)->checkState()==Qt::Checked;
        bool found = false;
        for(int i=0; i<STARTAPPS.length(); i++){
            if(STARTAPPS[i]->filePath==file){
                found = true;
                if(enabled != !STARTAPPS[i]->isHidden){
                    //value is different
                    qDebug() << "Setting Autostart:" << enabled << STARTAPPS[i]->filePath;
                    STARTAPPS[i]->setAutoStarted(enabled);
                }
                break;
            }
        }
        if(!found && enabled){
            //New file/binary/app
            qDebug() << "Adding new AutoStart File:" << file;
            LXDG::setAutoStarted(enabled, file);
            //newstartapps = true;
        }
    } //end loop over GUI items
    //Now cleanup all the STARTAPPS data
    for(int i=STARTAPPS.length()-1; i>=0; i--){ STARTAPPS[i]->deleteLater(); }
}

void MainWindow::LoadSettings(){
    loading = true;
    //qDebug() << "Load AutoStart Files";
    QList<XDGDesktop*> STARTAPPS = LXDG::findAutoStartFiles(true); //also want invalid/disabled items
    //qDebug() << " - done:" << STARTAPPS.length();
    //qDebug() << "StartApps:";
    ui->list_session_start->clear();
    for(int i=0; i<STARTAPPS.length(); i++){
        //qDebug() << STARTAPPS[i]->filePath +" -> " +STARTAPPS[i]->name << STARTAPPS[i]->isHidden;
        if( !STARTAPPS[i]->isValid() || !QFile::exists(STARTAPPS[i]->filePath) ){ continue; }
        QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(STARTAPPS[i]->icon,"application-x-executable"), STARTAPPS[i]->name );
        it->setWhatsThis(STARTAPPS[i]->filePath); //keep the file location
        it->setToolTip(STARTAPPS[i]->comment);
        if(STARTAPPS[i]->isHidden){ it->setCheckState( Qt::Unchecked); }
        else{it->setCheckState( Qt::Checked); }
        ui->list_session_start->addItem(it);
    }
    //Now cleanup all the STARTAPPS data
    for(int i=STARTAPPS.length()-1; i>=0; i--){ STARTAPPS[i]->deleteLater(); }
    loading = false;
}

void MainWindow::updateIcons(){
    ui->tool_session_addapp->setIcon( LXDG::findIcon("system-run","") );
    ui->tool_session_addbin->setIcon( LXDG::findIcon("system-search","") );
    ui->tool_session_addfile->setIcon( LXDG::findIcon("quickopen-file","") );
}

//=================
//         PRIVATE
//=================
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

//=================
//    PRIVATE SLOTS
//=================
void MainWindow::rmsessionstartitem(){
    if(ui->list_session_start->currentRow() < 0){ return; } //no item selected
    delete ui->list_session_start->takeItem(ui->list_session_start->currentRow());
    settingChanged();
}

void MainWindow::addsessionstartapp(){
    //Prompt for the application to start
    QString app = getSysApp(false); //no reset
    if(app.isEmpty()){ return; } //cancelled
    XDGDesktop desk(app);
    QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(desk.icon,""), desk.name );
    it->setWhatsThis(desk.filePath);
    it->setToolTip(desk.comment);
    it->setCheckState(Qt::Checked);
    
    ui->list_session_start->addItem(it);
    ui->list_session_start->setCurrentItem(it);
    settingChanged();
}

void MainWindow::addsessionstartbin(){
    QString chkpath = LOS::AppPrefix() + "bin";
    if(!QFile::exists(chkpath)){ chkpath = QDir::homePath(); }
    QString bin = QFileDialog::getOpenFileName(this, tr("Select Binary"), chkpath, tr("Application Binaries (*)") );
    if( bin.isEmpty() || !QFile::exists(bin) ){ return; } //cancelled
    if( !QFileInfo(bin).isExecutable() ){
        QMessageBox::warning(this, tr("Invalid Binary"), tr("The selected file is not executable!"));
        return;
    }
    QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon("application-x-executable",""), bin.section("/",-1) );
    it->setWhatsThis(bin); //command to be saved/run
    it->setToolTip(bin);
    it->setCheckState(Qt::Checked);
    ui->list_session_start->addItem(it);
    ui->list_session_start->setCurrentItem(it);
    settingChanged();
}

void MainWindow::addsessionstartfile(){
    QString chkpath = QDir::homePath();
    QString bin = QFileDialog::getOpenFileName(this, tr("Select File"), chkpath, tr("All Files (*)") );
    if( bin.isEmpty() || !QFile::exists(bin) ){ return; } //cancelled
    QListWidgetItem *it = new QListWidgetItem( LXDG::findMimeIcon(bin), bin.section("/",-1) );
    it->setWhatsThis(bin); //file to be saved/run
    it->setToolTip(bin);
    it->setCheckState(Qt::Checked);
    ui->list_session_start->addItem(it);
    ui->list_session_start->setCurrentItem(it);
    settingChanged();
}

void MainWindow::settingChanged(){
    if(!loading){
        SaveSettings();
    }
}

void MainWindow::showAbout(){
    QMessageBox::about(this, tr("About LCC Autostart"), 
        tr("Lumina Control Center - Autostart Configuration\n\n"
           "Configure which applications and services start automatically when you log in.\n\n"
           "Copyright (c) 2024, jt(q5sys)\n"
           "Available under the 3-clause BSD license"));
}
