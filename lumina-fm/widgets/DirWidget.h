//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_FM_DIRECTORY_BROWSER_WIDGET_H
#define _LUMINA_FM_DIRECTORY_BROWSER_WIDGET_H

#include <QList>
#include <QWidget>
#include <QObject>

#include "../DirData.h"

#define ZSNAPDIR QString("/.zfs/snapshot/")

namespace Ui{
	class DirWidget;
};

class DirWidget : public QWidget{
	Q_OBJECT
public:
	enum DETAILTYPES{ NAME, SIZE, TYPE, DATEMOD, DATECREATE};
	DirWidget(QString objID, QWidget *parent = 0); //needs a unique ID (to distinguish from other DirWidgets)
	~DirWidget();

	QString id();
	void setShowDetails(bool show);
	void setShowSidebar(bool show);
	void setDetails(QList<DETAILTYPES> list);
	
public slots:
	void LoadDir(QString dir, QList<LFileInfo> list);
	void LoadSnaps(QString basedir, QStringList snaps);

	//Theme change functions
	void UpdateIcons();
	void UpdateText();
	
private:
	Ui::DirWidget *ui;
	QString ID, CDIR; //unique ID assigned by the parent and the current dir path
	QList<LFileInfo> CLIST; //current item list (snap or not)
	QString normalbasedir, snapbasedir, snaprelpath; //for maintaining direcoty context while moving between snapshots
	QStringList snapshots;
	bool showDetails; //which widget to use for showing items
	QList<DETAILTYPES> listDetails;

	void setupConnections();
	
private slots:
	//UI BUTTONS
	// -- Left Action Buttons
	void on_tool_act_copy_clicked();
	void on_tool_act_cut_clicked();
	void on_tool_act_fav_clicked();
	void on_tool_act_paste_clicked();
	void on_tool_act_rename_clicked();
	void on_tool_act_rm_clicked();
	void on_tool_act_run_clicked();
	void on_tool_act_runwith_clicked();
        // -- Left Restore Buttons
	void on_tool_restore_clicked();
	void on_tool_restore_over_clicked();
	// -- Bottom Action Buttons
	void on_tool_goToImages_clicked();
	void on_tool_goToPlayer_clicked();
	// -- Top Snapshot Buttons
	void on_tool_snap_newer_clicked();
	void on_tool_snap_older_clicked();
	void on_slider_snap_valueChanged(int);
signals:
	void LoadDirectory(QString, QString); //ID, dirpath
	void findSnaps(QString, QString); //ID, dirpath
};
#endif