//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is the general class for a single-instance application
//===========================================
//EXAMPLE USAGE in main.cpp:
//
// LSingleApplication app(argc, argv);
// if( !app.isPrimaryProcess() ){
//    return 0;
//  }
//  QMainWindow w; //or whatever the main window class is
//  connect(app, SIGNAL(InputsAvailable(QStringList)), w, SLOT(<some slot>)); //for interactive apps - optional
//  app.exec();
//===========================================
#ifndef _LUMINA_LIBRARY_SINGLE_APPLICATION_H
#define _LUMINA_LIBRARY_SINGLE_APPLICATION_H

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QLocalServer>
#include <QLockFile>


class LSingleApplication : public QApplication{
  Q_OBJECT
public:
	LSingleApplication(int &argc, char **argv);
	~LSingleApplication();

	bool isPrimaryProcess();

private:
	bool isActive;
	QLockFile *lockfile;
	QLocalServer *lserver;
	QString cfile;
	QStringList inputlist;

	void PerformLockChecks();

private slots:
	void newInputsAvailable(); //internally used to detect a message from an alternate instance

signals:
	void InputsAvailable(QStringList);

};

#endif
