//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the dialog for cloning a git repository
//===========================================
#ifndef _LUMINA_FM_GIT_DIALOG_H
#define _LUMINA_FM_GIT_DIALOG_H

#include <QWizard>
#include <QString>

#include "gitCompat.h"

namespace Ui{
	class GitWizard;
};

class GitWizard : public QWizard{
	Q_OBJECT
public:
	GitWizard(QWidget *parent = 0);
	~GitWizard();

	//Input values;
	void setWorkingDir(QString path);

private:
	Ui::GitWizard *ui;
	QString inDir;

	QString assembleURL();
	void showDownload(GitProcess *P);

private slots:
	//Page Change slots
	void pageChanged(int newpage); //called when the "next" button is clicked
	void finished(int); //called when the "finish" button is clicked
	
	//Page validation slots
	void validateRepo(); //for page_repo
	void validateType(); //for page_type
};

#endif
