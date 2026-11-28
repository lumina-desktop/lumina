//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_LCC_PANELS_SCRIPT_DIALOG_H
#define _LUMINA_LCC_PANELS_SCRIPT_DIALOG_H

#include <QDialog>
#include <QString>

namespace Ui{
	class ScriptDialog;
};

class ScriptDialog : public QDialog{
	Q_OBJECT
public:
	ScriptDialog(QWidget* parent = 0);
	~ScriptDialog();
	
	bool isValid();
	QString name();
	QString command();
	QString icon();

private:
	Ui::ScriptDialog *ui;
	
};

#endif
