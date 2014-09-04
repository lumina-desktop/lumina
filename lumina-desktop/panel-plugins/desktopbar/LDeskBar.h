//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This plugin displays the contents of the user's home directory 
//    as organized within a couple buttons on the panel (apps, dirs, files)
//===========================================
#ifndef _LUMINA_DESKTOP_DESKBAR_H
#define _LUMINA_DESKTOP_DESKBAR_H

// Qt includes
#include <QWidget>
#include <QString>
#include <QAction>
#include <QMenu>
#include <QProcess>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QHBoxLayout>
#include <QIcon>
#include <QToolButton>
#include <QDebug>

// libLumina includes
#include <LuminaXDG.h>

// local includes
//#include "../LTBWidget.h"
#include "../LPPlugin.h"

class LDeskBarPlugin : public LPPlugin{
	Q_OBJECT
public:
	LDeskBarPlugin(QWidget* parent=0, QString id = "desktopbar", bool horizontal=true);
	~LDeskBarPlugin();
	
private:
	//QHBoxLayout *layout;
	QString desktopPath;
	QFileSystemWatcher *watcher;
	//Special toolbuttons and menus
	QToolButton *appB, *fileB, *dirB;
	QMenu *appM, *dirM, *audioM, *videoM, *pictureM, *fileM, *otherM, *docM;
	QStringList audioFilter, videoFilter, pictureFilter, docsFilter;
	QFileInfoList totals;
	QList<QToolButton*> APPLIST;
	
	void initializeDesktop();
	//bool readDesktopFile(QString path, QString &name, QString &iconpath);
	
	QAction* newAction(QString filepath, QString name, QString iconpath);
	QAction* newAction(QString filepath, QString name, QIcon icon);

	void updateMenu(QMenu* menu, QFileInfoList files, bool trim = true);

	
private slots:
	void ActionTriggered(QAction* act);
	void desktopChanged();
	
public slots:
	void OrientationChange(){
	  QSize sz;
	  if(this->layout()->direction()==QBoxLayout::LeftToRight){
	    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
	    sz = QSize(this->height(), this->height());
	  }else{
	    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	    sz = QSize(this->width(), this->width());
	  }
	  appB->setIconSize(sz);
	  fileB->setIconSize(sz);
	  dirB->setIconSize(sz);
	  for(int i=0; i<APPLIST.length(); i++){
	    APPLIST[i]->setIconSize(sz);
	  }
	  this->layout()->update();
	}
};


#endif

