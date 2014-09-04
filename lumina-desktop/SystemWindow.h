#ifndef _LUMINA_DESKTOP_SYSTEM_WINDOW_H
#define _LUMINA_DESKTOP_SYSTEM_WINDOW_H

#include <QDialog>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <QList>

#include "ui_SystemWindow.h"
#include "Globals.h"

#include <LuminaXDG.h>
#include <LuminaX11.h>
#include <LuminaOS.h>



namespace Ui{
	class SystemWindow;
};

class SystemWindow : public QDialog{
	Q_OBJECT
public:
	SystemWindow();
	~SystemWindow();

private:
	Ui::SystemWindow *ui;

	void closeAllWindows();
	
private slots:
	void sysLogout(){
	  closeAllWindows();
	  QCoreApplication::exit(0);
	}
	
	void sysRestart(){
	  closeAllWindows();
	  LOS::systemRestart();
	  QCoreApplication::exit(0);		
	}
	
	void sysShutdown(){
	  closeAllWindows();
	  LOS::systemShutdown();
	  QCoreApplication::exit(0);
	}
	
	void sysCancel(){
	  this->close();
	}
};

#endif