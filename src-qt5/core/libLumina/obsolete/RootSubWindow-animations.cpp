//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RootSubWindow.h"
#include <QDebug>

QStringList RootSubWindow::validAnimations(NativeWindow::Property prop){
  QStringList valid;
  if(prop == NativeWindow::Visible){
    valid << "zoom" << "wipe-center-vertical" << "wipe-center-horizontal" << "shade-top" << "shade-right" << "shade-left" << "shade-bottom";
  }else if(prop == NativeWindow::Size){
    //Note: this is used for pretty much all geometry changes to the window where it is visible both before/after animation
    valid << "direct";
  }
  return valid;
}

void RootSubWindow::loadAnimation(QString name, NativeWindow::Property prop, QVariant nval){
  if(anim->state()==QAbstractAnimation::Running){ return; } //already running
  animResetProp.clear();
  //Special case - random animation each time
  if(name=="random"){
    QStringList valid = validAnimations(prop);
    if(!valid.isEmpty()){ name = valid.at(qrand()%valid.length()); }
  }
  //Now setup the animation
  if(prop == NativeWindow::Visible){
    //NOTE: Assigns values for "invisible->visible" animation: will reverse it afterwards as needed
    anim->setPropertyName("geometry");
    QRect geom = this->geometry();
    if(name == "zoom"){
      //Zoom to/from the center point
      anim->setStartValue( QRect(geom.center(), QSize(0,0)) );
      anim->setEndValue(geom);
    }else if(name == "wipe-center-vertical"){
      anim->setStartValue( QRect( geom.center().x(), geom.y(), 0, geom.height()) );
      anim->setEndValue( geom );
    }else if(name == "wipe-center-horizontal"){
      anim->setStartValue( QRect( geom.x(), geom.center().y(), geom.width(), 0) );
      anim->setEndValue( geom );
    }else if(name == "shade-top"){
      anim->setStartValue( QRect( geom.x(), geom.y(), geom.width(), 0) );
      anim->setEndValue( geom );
    }else if(name == "shade-bottom"){
      anim->setStartValue( QRect( geom.x(), geom.y()+geom.height(), geom.width(), 0) );
      anim->setEndValue( geom );
    }else if(name == "shade-left"){
      anim->setStartValue( QRect( geom.x(), geom.y(), 0, geom.height()) );
      anim->setEndValue( geom );
    }else if(name == "shade-right"){
      anim->setStartValue( QRect( geom.x()+geom.width(), geom.y(), 0, geom.height()) );
      anim->setEndValue( geom );
    }else{
      //Invalid/None animation
      if(nval.toBool()){ this->show(); }
      else{ this->hide(); }
      return;
    }
    if(nval.toBool()){
      this->setGeometry( anim->startValue().toRect() ); //ensure the window is the initial geom before it becomes visible
      //QTimer::singleShot( anim->duration()+5, this, SLOT(activate()) );
    }else{
      QVariant tmp = anim->startValue();
        anim->setStartValue(anim->endValue());
        anim->setEndValue(tmp);
      animResetProp = anim->startValue();
      QTimer::singleShot(anim->duration(), this, SLOT(hide()) );
    }
    WinWidget->pause();
    anim->start();
    this->show();
  } //end of Visibility animation
  else if(prop == NativeWindow::Size){
    //This is pretty much all geometry animations where the window is visible->visible
    anim->setPropertyName("geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(nval.toRect());
    /*if(name==""){
      // TO-DO modify the path from beginning->end somehow
    }*/
    // Now start the animation
    WinWidget->pause();
    anim->start();
    this->show();
  }
}

void RootSubWindow::animFinished(){
  if(closing){ this->close(); return;}
  else if(anim->propertyName()=="geometry"){
    if(!animResetProp.isNull()){
      /*qDebug() << "Animation Finished, Reset Geometry:" << animResetProp.toRect();
      qDebug() << " - Starting Value:" << anim->startValue().toRect();
      qDebug() << " - Ending Value:" << anim->endValue().toRect();*/
      this->setGeometry( animResetProp.toRect() );
      //Also ensure that the proper geometry is saved to the window structure
      QRect curg = this->geometry();
      QRect wing = WIN->geometry();
      //qDebug() << " - After Animation Reset:" << curg << wing;
      if(curg!=wing){
        QRect clientg = clientGlobalGeom();
        //qDebug() << "Sub Window geometry:" << clientg;
        WIN->setProperties(QList< NativeWindow::Property>() << NativeWindow::Size << NativeWindow::GlobalPos,
		QList<QVariant>() << clientg.size() << clientg.topLeft() );
      }
    }
    WinWidget->resyncWindow(); //also let the window know about the current geometry
  }
  animResetProp = QVariant(); //clear the variable
  //QTimer::singleShot(10, WinWidget, SLOT(resume()) );
  WinWidget->resume();
  emit windowAnimFinished();
}
