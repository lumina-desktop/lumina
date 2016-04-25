#ifndef _LUMINA_DESKTOP_SYSTEM_WINDOW_H
#define _LUMINA_DESKTOP_SYSTEM_WINDOW_H

#include <QDialog>

#include "ui_SystemWindow.h"




namespace Ui{
	class SystemWindow;
};

class SystemWindow : public QDialog{
	Q_OBJECT
public:
	SystemWindow();
	~SystemWindow();

public slots:
	void updateWindow();

private:
	Ui::SystemWindow *ui;

	//void closeAllWindows();
	
private slots:
	void sysLogout();
	
	void sysRestart();
	
	void sysShutdown();
	
	void sysSuspend();

	void sysCancel(){
	  this->close();
	}
	
	void sysLock();
};

#endif
