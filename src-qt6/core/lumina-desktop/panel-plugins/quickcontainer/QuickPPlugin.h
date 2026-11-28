//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class is a simple container for a QtQuick plugin
//===========================================
#ifndef _LUMINA_DESKTOP_PANEL_PLUGIN_QUICK_H
#define _LUMINA_DESKTOP_PANEL_PLUGIN_QUICK_H

#include <QQuickWidget>
#include <QVBoxLayout>
#include "../LPPlugin.h"

#include <LUtils.h>
#include <QDebug>

class QuickPPlugin : public LPPlugin{
	Q_OBJECT
public:
	QuickPPlugin(QWidget* parent, QString ID, bool horizontal) : LPPlugin(parent, ID){
	  container = new QQuickWidget(this);
	    container->setResizeMode(QQuickWidget::SizeRootObjectToView);
	  this->layout()->addWidget(container);
	  horizontal = true; //just to silence compiler warning
	    container->setSource(QUrl::fromLocalFile( LUtils::findQuickPluginFile(ID.section("---",0,0)) ));
	}
	
	~QuickPPlugin(){}
	
private:
	QQuickWidget *container;

private slots:
	void statusChange(QQuickWidget::Status status){
	  if(status == QQuickWidget::Error){
	    qDebug() << "Quick Widget Error:" << this->type();
	  }
	}

};
#endif
