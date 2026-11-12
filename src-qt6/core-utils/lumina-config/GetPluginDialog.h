//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_GET_PLUGIN_DIALOG_H
#define _LUMINA_CONFIG_GET_PLUGIN_DIALOG_H

#include <QDialog>

#include "LPlugins.h"

namespace Ui{
	class GetPluginDialog;
};

class GetPluginDialog : public QDialog{
	Q_OBJECT
public:
	GetPluginDialog(QWidget* parent = 0);
	~GetPluginDialog();
	
	void LoadPlugins(QString type, LPlugins *DB);

	bool selected; //this is set to true if a plugin was selected by the user
	QString plugID; //this is set to the ID of the selected plugin

private:
	Ui::GetPluginDialog *ui;

private slots:
	void pluginchanged();
	void accept();
	
};

#endif