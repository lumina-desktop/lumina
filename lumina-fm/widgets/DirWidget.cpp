//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "DirWidget.h"
#include "ui_DirWidget.h"

DirWidget::DirWidget(QString objID, QWidget *parent) : QWidget(parent), ui(new Ui::DirWidget){
  ui->setupUi(this); //load the designer file
  setShowDetails(true);
  UpdateIcons();
  UpdateText();	
  setupConnections();
}

DirWidget::~DirWidget(){
	
}

QString DirWidget::id(){
  return ID;
}

void DirWidget::setShowDetails(bool show){
  showDetails = show;
  ui->listWidget->setVisible(!showDetails);
  ui->treeWidget->setVisible(showDetails);
}

void DirWidget::setShowSidebar(bool show){
  this->stacked_actions->setVisible(show);
}

void DirWidget::setDetails(QList<DETAILTYPES> list){
  listDetails = list;
  LoadDir(CDIR, CLIST); //just refresh the UI
}

// ================
//    PUBLIC SLOTS
// ================
void DirWidget::LoadDir(QString dir, QList<LFileInfo> list){
  if(dir.isEmpty()){ return; } //nothing to do
  CLIST = list; //save for later
  CDIR = dir;
  //Determine if this is an internal ZFS snapshot
  bool loadsnaps = false;
  if( dir.contains(ZSNAPDIR) ){
    //This is a zfs snapshot - only update the saved paths necessary to rotate between snapshots/system
    snaprelpath = dir.section(ZSNAPDIR,1,1000).section("/",1,1000); //the relative path inside the snapshot
    normalbasedir = dir.section(ZSNAPDIR,0,0)+"/"+snaprelpath; //Update the new base directory
    ui->stacked_actions->setCurrentWidget(ui->page_restore);
    //See if this was a manual move to the directory, or an internal move
    QString tmp = dir.section(ZSNAPDIR,0,0);
    if(tmp != snapbasedir.section(ZSNAPDIR,0,0)){
      loadsnaps = true; //different snapshot loaded - need to update internally
    }
  }else{
    //This is a normal directory - prompt for snapshot information
    normalbasedir = dir;
    snapbasedir.clear();
    ui->stacked_actions->setCurrentWidget(ui->page_dir);
    loadsnaps = true;
  }
  if(loadsnaps){
    //kick this off while still loading the dir contents
    ui->group_snaps->setEnabled(false); //to prevent the snap updates to be automatically used
    ui->group_snaps->setVisible(false);
    ui->slider_snap->setRange(1,1);
    emit findSnaps(ID, normalbasedir); 
  }
  //Clear the display widget
  if(showDetails){ 
    ui->treeWidget->clear(); 
    //Need to re-create the header item as well
    QTreeWidgetItem *it = new QTreeWidgetItem();
    for(int t=0; t<listDetails.length(); t++){
      it->
    }	  
    ui->treeWidget->setHeaderItem(it);
  }else{ ui->listWidget->clear(); }
  //Now fill the display widget
  for(int i=0; i<list.length(); i++){
    if(showDetails){
      //Now create all the individual items
      QTreeWidgetItem *it = new QTreeWidgetItem();
	it->setWhatsThis(list[i].fileName());
      for(int t=0; t<listDetails.length(); t++){
        switch(listDetails[t]){
	  case NAME:
	    it->setText(t,list[i].fileName());
	    if(list[i].isImage()){
	      it->setIcon(t, QIcon( QPixmap(list[i].absoluteFilePath()).scaledToHeight(64) ) );
	    }else{
	      it->setIcon(t, LXDG::findIcon(list[i].iconfile(),"unknown") );
	    }
	    break;
	  case SIZE:
	    if(!list[i].isDir()){
	      it->setText(t, LUtils::BytesToDisplaySize(list[i].size()) );
	    }
	    break;
	  case TYPE:
	    it->setText(t, list[i].mimetype());
	  case DATEMOD:
	    it->setText(t, list[i].lastModified().toString(Qt:DefaultLocaleShortDate);
	    break;
	  case DATECREATE:
	    it->setText(t, list[i].created().toString(Qt:DefaultLocaleShortDate);
	    break;
	}
      }
      ui->treeWidget->addTopLevelItem(it);
    }else{
	    
    }
    QApplication::processEvents(); //keep the UI snappy while loading a directory
  }
}

void DirWidget::LoadSnaps(QString basedir, QStringList snaps){
  //Save these value internally for use later
  snapbasedir = basedir;
  snapshots = snaps;
  //Now update the UI as necessary
  
  ui->slider_snap->setRange(1, snaps.length()+1);
  if(CDIR.contains(ZSNAPDIR)){
    //The user was already within a snapshot - figure out which one and set the slider appropriately
    int index = snaps.indexOf( CDIR.section(ZSNAPDIR,1,1).section("/",0,0) );
    if(index < 0){ index = snaps.length()+1; } //unknown - load the system (should never happen)
    ui->slider_snap_setValue(index);
  }else{
    ui->slider_snap->setValue(snaps.length()+1); //last item (normal system)
  }
  QApplication::processEvents(); //let the slider changed signal get thrown away before we re-enable the widget
  ui->group_snaps->setEnabled(!snaps.isEmpty());
  ui->group_snaps->setVisible(!snaps.isEmpty());
  ui->tool_snap_newer->setEnabled(ui->slider_snap->value() < ui->slider_snap->maximum());
  ui->tool_snap_older->setEnabled(ui->slider_snap->value() > ui->slider_snap->minimum());
	
}

//Theme change functions
void DirWidget::UpdateIcons(){
  //ui->tool_addNewFile->setIcon( LXDG::findIcon("document-new",""));
  //ui->tool_addToDir->setIcon( LXDG::findIcon("folder-new","") );

  //Snapshot buttons
  ui->tool_snap_newer->setIcon(LXDG::findIcon("go-next-view","") );
  ui->tool_snap_older->setIcon(LXDG::findIcon("go-previous-view","") );
  //Botton-Action Buttons
  ui->tool_goToImages->setIcon( LXDG::findIcon("fileview-preview","") );
  ui->tool_goToPlayer->setIcon( LXDG::findIcon("applications-multimedia","") );	
  //Side-Action Buttons
  ui->tool_act_run->setIcon( LXDG::findIcon("run-build-file","") );
  ui->tool_act_runwith->setIcon( LXDG::findIcon("run-build-configure","") );
  ui->tool_act_cut->setIcon( LXDG::findIcon("edit-cut","") );
  ui->tool_act_copy->setIcon( LXDG::findIcon("edit-copy","") );
  ui->tool_act_paste->setIcon( LXDG::findIcon("edit-paste","") );
  ui->tool_act_rename->setIcon( LXDG::findIcon("edit-rename","") );
  ui->tool_act_rm->setIcon( LXDG::findIcon("edit-delete","") );
  ui->tool_act_fav->setIcon( LXDG::findIcon("bookmark-toolbar","") );
  //Restore-Action Buttons
  ui->tool_restore->setIcon( LXDG::findIcon("document-revert","") );	
  ui->tool_restore_over->setIcon( LXDG::findIcon("document-revert","") );	
}

void DirWidget::UpdateText(){
  ui->retranslateUi(this);
}


// =================
//       PRIVATE
// =================
void DirWidget::setupConnections(){
  //Tree Widget interaction
  connect(ui->tree_dir_view, SIGNAL(activated(const QModelIndex&)), this, SLOT(ItemRun(const QModelIndex&)) );	
  connect(ui->list_dir_view, SIGNAL(activated(const QModelIndex&)), this, SLOT(ItemRun(const QModelIndex&)) );
  connect(ui->tree_dir_view, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OpenContextMenu(const QPoint&)) );
  connect(ui->list_dir_view, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OpenContextMenu(const QPoint&)) );
  connect(ui->tree_dir_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(ItemSelectionChanged()) );
  connect(ui->list_dir_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(ItemSelectionChanged()) );
}

// =================
//    PRIVATE SLOTS
// =================