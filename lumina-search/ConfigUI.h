//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_SEARCH_CONFIGURATION_DIALOG_H
#define _LUMINA_SEARCH_CONFIGURATION_DIALOG_H

#include <QDialog>

namespace Ui{
	class ConfigUI;
};

class ConfigUI : public QDialog{
	Q_OBJECT
public:
	ConfigUI(QWidget *parent = 0);
	~ConfigUI();

	void loadInitialValues(QString startdir, QStringList skipdirs);

	QString newStartDir;
	QStringList newSkipDirs;

private:
	Ui::ConfigUI *ui;

private slots:
	void on_tool_getStartDir_clicked();
	void on_tool_adddirs_clicked();
	void on_tool_rmdir_clicked();
	void on_list_excludes_itemSelectionChanged();
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
};
	
#endif
