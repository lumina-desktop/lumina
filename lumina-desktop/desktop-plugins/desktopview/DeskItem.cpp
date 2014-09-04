//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "DeskItem.h"

DeskItem::DeskItem(QWidget *parent, QString itempath, int ssize) : QToolButton(parent){
  this->setFixedSize(ssize, ssize);
  this->setWhatsThis(itempath);
  this->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  this->setAutoRaise(true);
  int txtheight = this->fontMetrics().height() *2;
  this->setIconSize( QSize(ssize-txtheight, ssize-txtheight));
  connect(this, SIGNAL(clicked()), this, SLOT(RunItem()) );
  updateItem();
}

DeskItem::~DeskItem(){
	
}

void DeskItem::updateItem(){
  QFileInfo info(this->whatsThis());
  QIcon ico;
  QString txt;
  if(info.isDir()){
    ico = LXDG::findIcon("folder","");
    txt = info.fileName();
  }else if(info.suffix()=="desktop"){
    bool ok = false;
    XDGDesktop dsk = LXDG::loadDesktopFile(this->whatsThis(), ok);
    if(ok){
      ico = LXDG::findIcon( dsk.icon );
      txt = dsk.name;
    }else{
      ico = LXDG::findIcon("","");
      txt = info.fileName();
    }
  }else{
    ico = LXDG::findIcon("application-x-zerosize","");
    txt = info.fileName();
  }
  this->setIcon(ico);
  //Trim the text size to fit
  txt = this->fontMetrics().elidedText(txt, Qt::ElideRight ,this->width() - 4);
  this->setText(txt);
	
}