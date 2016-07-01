//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the dialog for selecting an installed application
//===========================================
#ifndef _LUMINA_FILE_MANAGER_SCRIPT_SELECT_DIALOG_H
#define _LUMINA_FILE_MANAGER_SCRIPT_SELECT_DIALOG_H

#include "globals.h"

namespace Ui{
	class ScriptDialog;
};

class ScriptDialog : public QDialog{
	Q_OBJECT
public:
	ScriptDialog(QWidget *parent = 0);
	~ScriptDialog();

	//Main interaction functions
	bool isValid();
	QString icon();
	QString name();
	QString command();

private:
	Ui::ScriptDialog *ui;
	bool ok;

private slots:
	void on_pushApply_clicked();
	void on_pushCancel_clicked();
	void on_tool_getexec_clicked();
	void on_tool_geticon_clicked();
	void checkItems(bool firstrun = false);
};
#endif
