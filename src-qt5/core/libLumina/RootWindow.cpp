//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RootWindow.h"

// === PUBLIC ===
RootWindow::RootWindow(){

}

RootWindow::~RootWindow(){

}

void RootWindow::start(){

}

// === PRIVATE ===
void RootWindow::updateScreenPixmap(ScreenInfo *info){
  QPixmap pix(info->area.size());
  if(info->scale == RootWindow::SolidColor){
    QColor color;
    if(info->file.startsWith("rgb("))){
      QStringList colors = bgFile.section(")",0,0).section("(",1,1).split(",");
      color = QColor(colors[0].toInt(), colors[1].toInt(), colors[2].toInt());
    }else{
      color = QColor(info->file);
    }
    pix.fill(color);
  }else{
    QPixmap raw(info->file); //load the image from file
    //Now apply the proper aspect ratio as needed
    if(info->scale == RootWindow::Stretch || info->scale == RootWindow::Full || info->scale == RootWindow::Fit){
       Qt::AspectRatioMode armode = Qt::KeepAspectRatio; 
       if(info->scale == RootWindow::Stretch ){  armode = Qt::IgnoreAspectRatio; }
      else if(info->scale == RootWindow::Full ){ armode = Qt::KeepAspectRatioByExpanding; }
      if(raw.height()!=info->area.height() && raw.width() !=info->area.width()){
        raw = raw.scaled(info->area, armode);
      }
    }
    //Now calculate offset and draw width/height
    QRect drawRect(0,0, raw.width(), raw.height());
    if(info->scale == RootWindow::Full ){ 
      drawRect.moveTo( (info->area.width() - raw.width())/2, (info->area.height() - raw.height())/2 );
    }else if(info->scale == RootWindow::Fit ){
      drawRect.moveTo( (info->area.width() - raw.width())/2, (info->area.height() - raw.height())/2 );    
    }else if(info->scale == RootWindow::Center ){
      drawRect.moveTo( (info->area.width() - raw.width())/2, (info->area.height() - raw.height())/2 );    
    }else if(info->scale == RootWindow::Tile ){
      //Draw the entire area - no offset
      drawRect.setHeight(info->area.height());
      drawRect.setWidth(info->area.width()); 
    }else if(info->scale == RootWindow::BottomLeft ){
      drawRect.moveTo( 0 , info->area.height() - raw.height() );
    }else if(info->scale == RootWindow::BottomRight ){
      drawRect.moveTo( (info->area.width() - raw.width()), (info->area.height() - raw.height()) );
    }else if(info->scale == RootWindow::BottomCenter ){
      drawRect.moveTo( (info->area.width() - raw.width())/2, info->area.height() - raw.height()) );
    }else if(info->scale == RootWindow::TopLeft ){
      drawRect.moveTo( 0, 0 );
    }else if(info->scale == RootWindow::TopRight ){
      drawRect.moveTo( (info->area.width() - raw.width()), 0);
    }else if(info->scale == RootWindow::TopCenter ){
      drawRect.moveTo( (info->area.width() - raw.width())/2, 0);
    }else if(info->scale == RootWindow::CenterLeft ){
      drawRect.moveTo( 0, (info->area.height() - raw.height())/2 );
    }else if(info->scale == RootWindow::CenterRight ){
      drawRect.moveTo( (info->area.width() - raw.width()), (info->area.height() - raw.height())/2 );
    }

  QPainter P(&pix);
    P.setBrush(raw);
    P.setBrushOrigin(dx,dy);
    P.drawRect(dx,dy,
} //end SolidColor Check

  info.wallpaper = pix;
}

// === PUBLIC SLOTS ===
void RootWindow::ResizeRoot(){
  QList<QScreen*> scrns = QApplication::screens();
  //Update all the screen locations and ID's in the WALLPAPERS list
  QRect fullScreen;
  QStringList validids;
  //Update the size of the rootWindow itself
  for(int i=0; i<scrns.length(); i++){
    fullscreen = fullscreen.united(scrns[i]->geometry());
    validids << scrns[i]->name();
    for(int j=0; j<WALLPAPERS.length(); j++){
      if(WALLPAPERS[j].id == scrn[i]->name()){
        QSize oldsize = WALLPAPERS[j].area.size();
        WALLPAPERS[j].area = scrn[i]->geometry();
        if(oldsize != WALLPAPERS[j].area.size()){ updateScreenPixmap(&WALLPAPERS[j]); }
        break;
      }
    }
  }
  //Now clean up any invalid screen info in the WALLPAPERS List
  QStringList invalid;
  for(int i=0; i<WALLPAPERS.length(); i++){
    if(valid.contains(WALLPAPERS[i].id)){
      valid.removeAll(WALLPAPERS[i].id); //Already handled - remove it from the list
    }else{
      invalid << WALLPAPERS[i].id;
      WALLPAPERS.takeAt(i);
      i--;
    }
  }
  //Trigger a repaint and send out any signals
  this->update();
  emit RootResized();
  if(!validids.isEmpty()){ emit NewScreens(valid); }
  if(!invalid.isEmpty()){ emit RemoveScreens(invalid); }
}

void RootWindow::ChangeWallpaper(QString id, RootWindow::ScaleType scale, QString file){
  bool found = false;
  for(int i=0; i<WALLPAPERS.length() && !found; i++){
    if(WALLPAPERS[i].id == id){ 
      WALLPAPERS[i].scale = scale;
      WALLPAPERS[i].file = file;
      updateScreenPixmap(&WALLPAPERS[i]);
      found = true;
    }
  }
  if(!found){
    //Need to create a new screeninfo structure
    QList<QScreen*> scrns = QApplication::screens();
    for(int i=0; i<srcns.length(); i++){
      if(scrns[i].name()==id){
        screeninfo info;
          info.id = id;
	  info.file = file;
          info.scale = scale;
          info.area = srcns[i]->geometry();
         updateScreenPixmap(&info);
        WALLPAPERS << info;
        break;
      }
    }
  } //end check for a new id

}

// === PRIVATE SLOTS ===

// === PROTECTED ===
void RootWindow::paintEvent(QPaintEvent *ev){
  
  if(!wallpaper.isNull()){
    QPainter painter(this);
    painter.setBrush(wallpaper);
    painter.drawRect(ev->rect().adjusted(-1,-1,2,2));
  }else{
    QWidget::paintEvent(ev);
  }
}
