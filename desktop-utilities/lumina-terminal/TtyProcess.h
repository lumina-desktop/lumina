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
#ifndef _LUMINA_DESKTOP_UTILITIES_TERMINAL_TTY_PROCESS_WIDGET_H
#define _LUMINA_DESKTOP_UTILITIES_TERMINAL_TTY_PROCESS_WIDGET_H

#include <QDebug>
#include <QFile>

//Standard C library functions for PTY access/setup
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <signal.h>


class TTYProcess : public QFile{
	Q_OBJECT
public:
	TTYProcess(QObject *parent = 0);
	~TTYProcess();

	bool startTTY(QString prog, QStringList args = QStringList());

private:
	pid_t childProc;

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
	

};

#endif
