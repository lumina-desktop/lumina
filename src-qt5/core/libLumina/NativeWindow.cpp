//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "NativeWindow.h"

#include <QDebug>

// === PUBLIC ===
NativeWindow::NativeWindow(WId id) : QObject(){
  winid = id;
  frameid = 0;
  dmgID = 0;
}

NativeWindow::~NativeWindow(){
  hash.clear();
}

void NativeWindow::addFrameWinID(WId fid){
  frameid = fid;
}

void NativeWindow::addDamageID(unsigned int dmg){
  dmgID = dmg;
}

bool NativeWindow::isRelatedTo(WId tmp){
  return (relatedTo.contains(tmp) || winid == tmp || frameid == tmp);
}

WId NativeWindow::id(){
  return winid;
}

WId NativeWindow::frameId(){
  return frameid;
}

unsigned int NativeWindow::damageId(){
  return dmgID;
}

QVariant NativeWindow::property(NativeWindow::Property prop){
  if(hash.contains(prop)){ return hash.value(prop); }
  else if(prop == NativeWindow::RelatedWindows){ return QVariant::fromValue(relatedTo); }
  return QVariant(); //null variant
}

void NativeWindow::setProperty(NativeWindow::Property prop, QVariant val, bool force){
  if(prop == NativeWindow::RelatedWindows){ relatedTo = val.value< QList<WId> >(); }
  else if(prop == NativeWindow::None || (!force && hash.value(prop)==val)){ return; }
  else{ hash.insert(prop, val); }
  emit PropertiesChanged(QList<NativeWindow::Property>() << prop, QList<QVariant>() << val);
}

void NativeWindow::setProperties(QList<NativeWindow::Property> props, QList<QVariant> vals, bool force){
  for(int i=0; i<props.length(); i++){
    if(i>=vals.length()){ props.removeAt(i); i--; continue; } //no corresponding value for this property
    if(props[i] == NativeWindow::None || (!force && (hash.value(props[i]) == vals[i])) ){ props.removeAt(i); vals.removeAt(i); i--; continue; } //Invalid property or identical value
    hash.insert(props[i], vals[i]);
  }
  emit PropertiesChanged(props, vals);
}

void NativeWindow::requestProperty(NativeWindow::Property prop, QVariant val, bool force){
  if(prop == NativeWindow::None || prop == NativeWindow::RelatedWindows || (!force && hash.value(prop)==val) ){ return; }
  emit RequestPropertiesChange(winid, QList<NativeWindow::Property>() << prop, QList<QVariant>() << val);
}

void NativeWindow::requestProperties(QList<NativeWindow::Property> props, QList<QVariant> vals, bool force){
  //Verify/adjust inputs as needed
  for(int i=0; i<props.length(); i++){
    if(i>=vals.length()){ props.removeAt(i); i--; continue; } //no corresponding value for this property
    if(props[i] == NativeWindow::None || props[i] == NativeWindow::RelatedWindows || (!force && hash.value(props[i])==vals[i]) ){ props.removeAt(i); vals.removeAt(i); i--; continue; } //Invalid property or identical value
    /*if( (props[i] == NativeWindow::Visible || props[i] == NativeWindow::Active) && frameid !=0){
      //These particular properties needs to change the frame - not the window itself
      emit RequestPropertiesChange(frameid, QList<NativeWindow::Property>() << props[i], QList<QVariant>() << vals[i]);
      props.removeAt(i); vals.removeAt(i); i--;
    }*/
  }
  emit RequestPropertiesChange(winid, props, vals);
}

QRect NativeWindow::geometry(){
  //Calculate the "full" geometry of the window + frame (if any)
  QRect geom( hash.value(NativeWindow::GlobalPos).toPoint(), hash.value(NativeWindow::Size).toSize() );
  //Now adjust the window geom by the frame margins
  QList<int> frame = hash.value(NativeWindow::FrameExtents).value< QList<int> >(); //Left,Right,Top,Bottom
  //qDebug() << "Calculate Geometry:" << geom << frame;
  if(frame.length()==4){
    geom = geom.adjusted( -frame[0], -frame[2], frame[1], frame[3] );
  }
  //qDebug() << " - Total:" << geom;
  return geom;
}
// ==== PUBLIC SLOTS ===
void NativeWindow::requestClose(){
  emit RequestClose(winid);
}

void NativeWindow::requestKill(){
  emit RequestKill(winid);
}

void NativeWindow::requestPing(){
  emit RequestPing(winid);
}
