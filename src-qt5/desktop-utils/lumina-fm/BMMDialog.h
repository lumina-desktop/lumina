//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the dialog for managing bookmarks (BookMark Manager)
//===========================================
#ifndef _LUMINA_FILE_MANAGER_BOOKMARK_MANAGER_DIALOG_H
#define _LUMINA_FILE_MANAGER_BOOKMARK_MANAGER_DIALOG_H

// Qt includes
#include <QDialog>
#include <QInputDialog>
#include <QStringList>
#include <QSettings>
#include <QTimer>
#include <QMessageBox>

// libLumina includes
#include <LuminaXDG.h>

namespace Ui{
	class BMMDialog;
};

class BMMDialog : public QDialog{
	Q_OBJECT
public:
	BMMDialog(QWidget *parent = 0);
	~BMMDialog();

	void loadSettings(QSettings *);

private:
	Ui::BMMDialog *ui;
	QSettings *settings;

private slots:
	void RemoveItem();
	void RenameItem();
};

#endif
