//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "AppDialog.h"
#include "ui_AppDialog.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QDir>
#include <QDebug>

#include <LuminaXDG.h>

AppDialog::AppDialog(QWidget *parent, QString defaultPath) : QDialog(parent) {
    this->setWindowTitle(tr("Select Application"));
    this->setWindowIcon(LXDG::findIcon("system-search",""));
    this->resize(400, 300);
    
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
