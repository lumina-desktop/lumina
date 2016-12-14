//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class is a simple container for a QtQuick plugin
//===========================================
#ifndef _LUMINA_DESKTOP_DESKTOP_PLUGIN_QUICK_H
#define _LUMINA_DESKTOP_DESKTOP_PLUGIN_QUICK_H

#include <QQuickWidget>
#include <QVBoxLayout>
#include "../LDPlugin.h"

#include <LUtils.h>

class QuickDPlugin : public LDPlugin{
	Q_OBJECT
public:
	QuickDPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID){
	  this->setLayout( new QVBoxLayout());
	    this->layout()->setContentsMargins(0,0,0,0);
	  container = new QQuickWidget(this);
	    container->setResizeMode(QQuickWidget::SizeRootObjectToView);
	    connect(container, SIGNAL(statusChanged(QQuickWidget::Status)), this, SLOT(statusChange(QQuickWidget::Status)) );
	  this->layout()->addWidget(container);
	    container->setSource(QUrl::fromLocalFile( LUtils::findQuickPluginFile(ID.section("---",0,0)) ));
	  QApplication::processEvents(); //to check for errors right away
	  //this->setInitialSize(container->initialSize().width(), container->initialSize().height());
	}
	
	~QuickDPlugin(){}
	
	virtual QSize defaultPluginSize(){
	  // The returned QSize is in grid points (typically 100 or 200 pixels square)
	  return QSize(2,2);
	}
private:
	QQuickWidget *container;

private slots:
	void statusChange(QQuickWidget::Status status){
	  if(status == QQuickWidget::Error){
	    qDebug() << "Quick Widget Error:" << this->ID();
	    container->setSource(QUrl()); //clear out the script - experienced an error
	  }
	}

};
#endif
