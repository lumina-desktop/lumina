//===========================================
//  Lumina-Desktop source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// Dialog front-end for the "dd" utility to burn an IMG file to a USB stick
//===========================================
#ifndef _LUMINA_ARCHIVER_IMAGE_DIALOG_H
#define _LUMINA_ARCHIVER_IMAGE_DIALOG_H
#include <QDialog>
#include <QTimer>
#include <QProcess>
#include <QCloseEvent>
#include <QDebug>
#include <QDateTime>

 namespace Ui{
	class imgDialog;
};

class imgDialog : public QDialog{
	Q_OBJECT
public:
	imgDialog(QWidget *parent = 0);
	~imgDialog();

	void loadIMG(QString filepath);
	
private:
	Ui::imgDialog *ui;
	QProcess *ddProc;
	QTimer *procTimer;
	bool BSD_signal;
	QString lastmsg;
	double unitdiv;
	QDateTime startTime;
private slots:
	void start_process();
	void cancel();
	void loadDeviceList();

	void getProcStatus();
	void procInfoAvailable();
	void procFinished();

protected:
	void closeEvent(QCloseEvent *ev){
	  if(ddProc==0 || ddProc->state()==QProcess::NotRunning){
	    //Nothing special going on
	    QDialog::closeEvent(ev); //normal close procedure
	  }else{
	    //Process running - run the cancel routine first
	    ev->ignore();
	    QTimer::singleShot(0, this, SLOT(cancel()) );
	  }
	};

};

#endif
