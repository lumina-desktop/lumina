//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the dialog for editing a theme stylesheet
//===========================================
#ifndef _LUMINA_CONFIG_THEME_EDIT_DIALOG_H
#define _LUMINA_CONFIG_THEME_EDIT_DIALOG_H

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
#include <QMenu>

#include <LuminaXDG.h>
#include <LuminaThemes.h>

#include "LPlugins.h"


namespace Ui{
	class ThemeDialog;
};

class ThemeDialog : public QDialog{
	Q_OBJECT
private:
	Ui::ThemeDialog *ui;
	QString filepath;
	QMenu *colormenu;

	void loadTheme();
	void saveTheme();

public:
	ThemeDialog(QWidget *parent, LPlugins* plugs, QString themeFilePath);
	~ThemeDialog(){}

	QString themename, themepath;
	
private slots:
	void themeChanged();
	void on_push_save_clicked();
	void on_push_cancel_clicked();
	void on_push_apply_clicked();
	void menuTriggered(QAction*);

};

#endif