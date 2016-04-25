//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the dialog for editing a color scheme
//===========================================
#ifndef _LUMINA_FILE_MANAGER_COLOR_SELECT_DIALOG_H
#define _LUMINA_FILE_MANAGER_COLOR_SELECT_DIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QTreeWidgetItem>
#include <QBrush>
#include <QColor>
#include <QMessageBox>
#include <QInputDialog>
#include <QColorDialog>
#include <QDebug>

#include <LuminaXDG.h>
#include <LuminaThemes.h>

#include "LPlugins.h"


namespace Ui{
	class ColorDialog;
};

class ColorDialog : public QDialog{
	Q_OBJECT
private:
	Ui::ColorDialog *ui;
	QString filepath;

	void loadColors();
	void saveColors();
	QColor StringToColor(QString);
	void updateItem(QTreeWidgetItem *it, QString value);

public:
	ColorDialog(QWidget *parent, LPlugins* plugs, QString colorFilePath);
	~ColorDialog(){}

	QString colorname, colorpath;
	
private slots:
	void on_push_save_clicked();
	void on_push_cancel_clicked();
	void on_tool_getcolor_clicked();
	void on_tool_editcolor_clicked();

};

#endif