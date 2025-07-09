//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_LCC_DEFAULTAPPS_MAINWINDOW_H
#define _LUMINA_LCC_DEFAULTAPPS_MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QList>
#include <QWidget>
#include <QTreeWidgetItem>
#include <QToolButton>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QPoint>
#include <QMenu>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QAbstractButton>

//Now the Lumina Library classes
#include <LuminaXDG.h>
#include <LUtils.h>
#include <LDesktopUtils.h>
#include <LuminaOS.h>

namespace Ui{
	class MainWindow;
};

//App selection dialog class
class AppDialog : public QDialog{
	Q_OBJECT
private:
	QListWidget *listApps;
	QLineEdit *lineSearch;
	QDialogButtonBox *buttonBox;

public:
	AppDialog(QWidget *parent = 0, QString defaultPath = "");
	~AppDialog(){}

	void allowReset(bool allow);

	QString appselected; //selected application
	bool appreset; //Did the user select to reset to defaults?
		
private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
	void on_buttonBox_clicked(QAbstractButton *button);
	void on_listApps_itemDoubleClicked(QListWidgetItem *item);
	void on_lineSearch_textChanged(const QString &term);
};

class MainWindow : public QMainWindow{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void updateIcons();

private:
	Ui::MainWindow *ui;
	QString defaultBrowser;
	QString defaultEmail;
	QString defaultFileManager;
	QString defaultTerminal;

	QString getSysApp(bool allowreset, QString defaultPath = "");
	void updateDefaultButton(QToolButton *button, QString app);
	void LoadSettings();

private slots:
	//Simple defaults tab
	void changeDefaultBrowser();
	void changeDefaultEmail();
	void changeDefaultFileManager();
	void changeDefaultTerminal();
	//Advanced defaults tab
	void cleardefaultitem();
	void setdefaultitem();
	void setdefaultbinary();
	void checkdefaulticons();
};

#endif
