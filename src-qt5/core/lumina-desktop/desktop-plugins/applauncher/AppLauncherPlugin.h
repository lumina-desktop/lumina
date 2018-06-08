//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class is a quick sample desktop plugin
//===========================================
#ifndef _LUMINA_DESKTOP_DESKTOP_PLUGIN_APPLICATION_LAUNCHER_H
#define _LUMINA_DESKTOP_DESKTOP_PLUGIN_APPLICATION_LAUNCHER_H

#include <QToolButton>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QProcess>
#include <QFile>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QMenu>
#include <QCursor>
#include <QDrag>
#include <QMimeData>
#include <QtConcurrent>

#include "../LDPlugin.h"

#include <LuminaXDG.h>

class AppLauncherPlugin : public LDPlugin{
	Q_OBJECT
public:
	AppLauncherPlugin(QWidget* parent, QString ID);
	~AppLauncherPlugin(){}

	void Cleanup(); //special function for final cleanup

private:
	QToolButton *button;
	QFileSystemWatcher *watcher;
	//QMenu *menu;
	QInputDialog *inputDLG;
	QString iconID;
	int icosize;
	QPoint dragstartpos;

private slots:
	void loadButton();
	void buttonClicked(bool openwith = false);
	void iconLoaded(QString);

	//void openContextMenu();

	//void increaseIconSize();
	//void decreaseIconSize();
	//void deleteFile();
	void startDragNDrop();

	void actionTriggered(QAction *act);
	void openWith();
	void fileProperties();
	void fileDelete();
	void fileCut();
	void fileCopy();
	void fileRename();
	void renameFinished(int result);

	void fileDrop(bool copy, QList<QUrl> urls);

public slots:
	void LocaleChange(){
	  loadButton(); //force reload
	}

protected:
	void resizeEvent(QResizeEvent *ev){
	  LDPlugin::resizeEvent(ev);
	  QTimer::singleShot(100, this, SLOT(loadButton()) );
	}
	void changeEvent(QEvent *ev){
	  LDPlugin::changeEvent(ev);
	  QEvent::Type tmp = ev->type();
	  if(tmp == QEvent::StyleChange || tmp==QEvent::ThemeChange || tmp==QEvent::LanguageChange || tmp==QEvent::LocaleChange){ loadButton(); }
	}

	void mousePressEvent(QMouseEvent *ev){
	  if(ev->button()==Qt::LeftButton){
	    dragstartpos = ev->pos();
	  }
	}

	void mouseMoveEvent(QMouseEvent *ev){
	  if( (ev->buttons() & Qt::LeftButton) ){
	    if((ev->pos() - dragstartpos).manhattanLength() > (this->width()/4) ){
	      emit StartMoving(this->ID());
	    }else if(false){ //
	      startDragNDrop();
	    }
	  }else{
	    LDPlugin::mouseMoveEvent(ev);
	  }
	}

	void dragEnterEvent(QDragEnterEvent *ev){
	  if(ev->mimeData()->hasUrls() && this->acceptDrops()){ ev->acceptProposedAction(); }
	}

	void dropEvent(QDropEvent *ev){
	  QList<QUrl> urls = ev->mimeData()->urls();
	  bool ok = !urls.isEmpty() && this->acceptDrops();
	  if(ok){
	    if(ev->proposedAction() == Qt::MoveAction){
	      ev->setDropAction(Qt::MoveAction);
	      QtConcurrent::run(this, &AppLauncherPlugin::fileDrop, false, urls);
	    }else if(ev->proposedAction() == Qt::CopyAction){
	      ev->setDropAction(Qt::CopyAction);
	      QtConcurrent::run(this, &AppLauncherPlugin::fileDrop, true, urls);
	    }else{ ok = false; }
	  }
	  if(ok){ ev->acceptProposedAction(); }
	}
};
#endif
