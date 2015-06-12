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

#include <LuminaUtils.h>

class QuickDPlugin : public LDPlugin{
	Q_OBJECT
public:
	QuickDPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID){
	  this->setLayout( new QVBoxLayout());
	    this->layout()->setContentsMargins(0,0,0,0);
	  container = new QQuickWidget(this);
	    container->setResizeMode(QQuickWidget::SizeRootObjectToView);
	  this->layout()->addWidget(container);
	    container->setSource(QUrl::fromLocalFile( LUtils::findQuickPluginFile(ID.section("---",0,0)) ));
	}
	
	~QuickDPlugin(){}
	
private:
	QQuickWidget *container;

private slots:
	void statusChange(QQuickWidget::Status status){
	  if(status == QQuickWidget::Error){
	    qDebug() << "Quick Widget Error:" << this->ID();
	  }
	}

};
#endif
