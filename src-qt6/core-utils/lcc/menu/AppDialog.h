//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_LCC_MENU_APP_DIALOG_H
#define _LUMINA_LCC_MENU_APP_DIALOG_H

#include <QDialog>
#include <QString>
#include <QWidget>

// Forward declarations
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QDialogButtonBox;
class QAbstractButton;

namespace Ui{
	class AppDialog;
};

class AppDialog : public QDialog{
	Q_OBJECT
public:
	AppDialog(QWidget* parent = 0, QString defaultPath = "");
	~AppDialog();
	
	void allowReset(bool allow);

	bool appreset;
	QString appselected;

private:
	Ui::AppDialog *ui;
	QLineEdit *lineSearch;
	QListWidget *listApps;
	QDialogButtonBox *buttonBox;

private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
	void on_buttonBox_clicked(QAbstractButton *button);
	void on_listApps_itemDoubleClicked(QListWidgetItem *item);
	void on_lineSearch_textChanged(const QString &term);
	
};

#endif
