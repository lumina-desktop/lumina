//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the dialog for showing currently running file operations
//===========================================
#ifndef _LUMINA_FILE_MANAGER_FILE_OP_DIALOG_H
#define _LUMINA_FILE_MANAGER_FILE_OP_DIALOG_H

// Qt includes
#include <QDialog>
#include <QMessageBox>
#include <QStringList>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <QFile>

// libLumina includes
#include <LuminaXDG.h>
#include <LuminaUtils.h>

namespace Ui{
	class FODialog;
};

class FODialog : public QDialog{
	Q_OBJECT
public:
	FODialog(QWidget *parent = 0);
	~FODialog();

	bool noerrors;

	void setOverwrite(bool);
	void RemoveFiles(QStringList paths);
	void CopyFiles(QStringList oldPaths, QStringList newPaths); 	  //same permissions as old files
	void RestoreFiles(QStringList oldPaths, QStringList newPaths); //user/group rw permissions
	void MoveFiles(QStringList oldPaths, QStringList newPaths);     //no change in permissions

private:
	Ui::FODialog *ui;
	bool isRM, isCP, isRESTORE, isMV;
	bool stopped;
	int overwrite; // [-1= auto, 0= no overwrite, 1= overwrite]
	QStringList ofiles, nfiles; //original/new files

	QStringList subfiles(QString dirpath); //recursive function for fetching all "child" files/dirs (dirs last)
	QString newFileName(QString path);
	QStringList removeItem(QString path);
	QStringList copyItem(QString oldpath, QString newpath);

private slots:
	void slotStartOperations();
	void on_push_stop_clicked();
};

#endif
