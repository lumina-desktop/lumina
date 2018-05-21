//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_PLAIN_TEXT_EDITOR_COLOR_DIALOG_H
#define _LUMINA_PLAIN_TEXT_EDITOR_COLOR_DIALOG_H

#include <QDialog>
#include <QSettings>
#include <QWidget>
#include <QCloseEvent>

namespace Ui{
	class ColorDialog;
};

class ColorDialog : public QDialog{
	Q_OBJECT
private:
	Ui::ColorDialog *ui;
	QSettings *settings;

public:
	ColorDialog(QSettings *set, QWidget *parent);
	~ColorDialog(){}

	void LoadColors();

public slots:
	void updateIcons();

private slots:
	void saveColors();
	void changeColor();

signals:
	void colorsChanged();

protected:
	void closeEvent(QCloseEvent*){
	  //Make sure this window never actually closes until the app is finished
	  this->hide();
	}
};

#endif
