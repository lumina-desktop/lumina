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

#include <QString>
#include <QStringList>
#include <QLocalServer>
#include <QLockFile>
#include <QApplication>

#include <LUtils.h>

//NOTE: This application type will automatically load the proper translation file(s)
//  if the application name is set properly
class LSingleApplication : public QApplication{
  Q_OBJECT
public:
	LSingleApplication(int &argc, char **argv, QString appname);
	~LSingleApplication();

	static QString getLockfileName(QString appname);
	static void removeLocks(QString appname);

	bool isPrimaryProcess();

	QStringList inputlist; //in case the app wants access to modified inputs (relative path fixes and such)

private:
	bool isActive, isBypass;
	QLockFile *lockfile;
	QLocalServer *lserver;
	QString cfile;
	QTranslator *cTrans; //current translation

	void PerformLockChecks();

private slots:
	void newInputsAvailable(); //internally used to detect a message from an alternate instance

signals:
	void InputsAvailable(QStringList);

};

#endif
