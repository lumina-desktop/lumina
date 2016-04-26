//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QObject>
#include <QProcess>

class LSession : public QObject{
	Q_OBJECT
private:
	QList<QProcess*> PROCS;
	bool stopping;

private slots:
	void stopall();

	void procFinished();

	void startProcess(QString ID, QString command);

public:
	LSession(){
	stopping = false;
	}
	~LSession(){ }

	void start();
	
};
