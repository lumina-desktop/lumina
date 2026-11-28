//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_WIDGET_UI_H
#define _LUMINA_CONFIG_PAGE_WIDGET_UI_H
#include "../globals.h"
//===============================
// NOTES FOR CREATING SUBPAGES
//===============================
// 1) Subclass this PageWidget for any client page
// 2) Init any internal widgets/classes in the constructor
// 3) Make sure you handle the [Save/Load]Settings functions (LoadSettings() will be called automatially after widget creation)
// 4) Make sure to emit the signals as needed for interactivity with the main container
//===============================

//Structure of all information needed for a page
struct PAGEINFO{
  QString name, title,  icon, comment, category, id;
  QStringList req_systems, search_tags;
};

//Main widget class needed to show a configuration page
class PageWidget : public QWidget{
	Q_OBJECT
public:

	//Main constructor/destructor (create/destroy any interface items)
	PageWidget(QWidget *parent) : QWidget(parent){
	  //this->setFocusPolicy(Qt::NoFocus);
	}
	~PageWidget(){}
		
	virtual bool needsScreenSelector(){ return false; } //change this to true for pages which load/set options on a per-screen basis
	virtual void setPreviousPage(QString){ } //re-implement this if the page needs knowledge of what the previous page was

signals:
	//emit this when the page has changes which are waiting to be saved
	void HasPendingChanges(bool);
	//emit this when the page title changes (will updates main UI as needed)
	void ChangePageTitle(QString);
	//emit this when we need to change to another client/page (if needed - generally only used for the base/group pages)
	void ChangePage(QString); //ID of new page to open

public slots:
	//User requested to save any pending changes
	virtual void SaveSettings(){} 
	virtual void LoadSettings(int){} //INPUT: Screen number (0+)
	virtual void updateIcons(){}

	//Simplification function for widget connections
	virtual void settingChanged(){
    emit HasPendingChanges(true);
	}
};

#endif
