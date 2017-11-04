//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// Note: Some of this class (particularly parts of the UI), were initially created by:
//    William (william-os4y on GitHub: https://github.com/william-os4y)
//    March -> April,  2015
// This utility was re-written by Ken Moore on August 31, 2015
//    Primarily to align the utility with the LWinInfo & XDGDesktop classes
//===========================================
#ifndef _LUMINA_FILE_INFO_MAIN_UI_H
#define _LUMINA_FILE_INFO_MAIN_UI_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <LVideoSurface.h>
#include <LVideoLabel.h>
#include <QElapsedTimer>
#include <QFuture>

#include <LuminaXDG.h>

namespace Ui{
  class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

	void LoadFile(QString path, QString type=""); //type=[APP, LINK]

public slots:
	void UpdateIcons();
	void ReloadAppIcon();

private:
	Ui::MainUI *ui;
	LFileInfo *INFO;
	LVideoSurface *surface;
	QMediaPlayer *player;
	bool flag;
	QElapsedTimer timer;
	QFuture<void> sizeThread;

	bool canwrite;
	bool terminate_thread; //flag for terminating the GetDirSize task
	void stopDirSize();

	void GetDirSize(const QString dirname) const; //function to get folder size

	void SyncFileInfo();

	void syncXdgStruct(XDGDesktop*);

	bool saveFile(QString path);
	QString findOpenDirFile(bool isdir = false);

signals:
	void folder_size_changed(quint64 size, quint64 files, quint64 folders, bool finished) const; //Signal for updating the folder size asynchronously

private slots:
	void SetupNewFile();
	//Initialization functions
	void SetupConnections();

	//UI Buttons
	void closeApplication();
	void save_clicked();
	void save_as_local_clicked();
	void save_as_register_clicked();
	void open_dir_clicked();
	void open_file_clicked();
	void getXdgCommand(QString prev = "");
	void on_tool_xdg_getDir_clicked();
	void on_push_xdg_getIcon_clicked();

	//XDG Value Changed
	bool checkXDGValidity();
	void xdgvaluechanged();

	//Folder size
	void refresh_folder_size(quint64 size, quint64 files, quint64 folders, bool finished); //Slot for updating the folder size asynchronously
};

#endif
