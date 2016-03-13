//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is basically a replacement for QProcess, where all process/terminal outputs
//   are redirected and not just the standard input/output channels. This allows it
//   to be used for terminal-like apps (shells) which directly modify the terminal output
//   rather than stick to input/output channels for communication.
//===========================================
//  IMPLEMENTATION NOTE
//======================
// The process requires/uses ANSI control codes (\x1B[<something>) for special operations
//    such as moving the cursor, erasing characters, etc..
//  It is recommended that you pair this class with the graphical "TerminalWidget.h" class
//    or some other ANSI-compatible display widget.
//===========================================
#ifndef _LUMINA_DESKTOP_UTILITIES_TERMINAL_TTY_PROCESS_WIDGET_H
#define _LUMINA_DESKTOP_UTILITIES_TERMINAL_TTY_PROCESS_WIDGET_H

#include <QDebug>
#include <QSocketNotifier>
#include <QKeyEvent>

//Standard C library functions for PTY access/setup
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

class TTYProcess : public QObject{
	Q_OBJECT
public:
	TTYProcess(QObject *parent = 0);
	~TTYProcess();

	bool startTTY(QString prog, QStringList args = QStringList());
	void closeTTY();

	//Primary read/write functions
	void writeTTY(QByteArray output);
	void writeQtKey(int key); //simplification function for handling special keys like arrows and such ( QKeyEvent()->key() )
	QByteArray readTTY();

	//Setup the terminal size (characters and pixels)
	void setTerminalSize(QSize chars, QSize pixels);

	//Status update checks
	bool isOpen();
	
	//Functions for handling ANSI escape codes (typically not used by hand)
	QByteArray CleanANSI(QByteArray, bool &incomplete);

private:
	pid_t childProc;
	int ttyfd;
	QSocketNotifier *sn;
	QByteArray fragBA; //fragment ByteArray
	
	//====================================
	// C Library function for setting up the PTY
	// Inputs:
	//    int &fd:	(output) file descriptor for the master PTY (positive integer if valid)
	//    char *prog:	program to run
	//    char **args:	program arguments
	// Returns:
	//    -1 for errors, child process PID (positive integer) if successful
	//====================================
	static pid_t LaunchProcess(int& fd, char *prog, char **child_args); 
	
private slots:
	void checkStatus(int);

signals:
	void readyRead();
	void processClosed();
};

#endif
