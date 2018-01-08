//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017-2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "NativeWindowObject.h"
#include <QQmlEngine>
#include <QDebug>
#include <QBuffer>

// == QML Type Registration ==
void NativeWindowObject::RegisterType(){
  static bool done = false;
  if(done){ return; }
  done=true;
  qmlRegisterType<NativeWindowObject>("Lumina.Backend.NativeWindowObject",2,0, "NativeWindowObject");
}

// === PUBLIC ===
NativeWindowObject::NativeWindowObject(WId id) : QObject(){
  winid = id;
  frameid = 0;
  dmgID = 0;
}

NativeWindowObject::~NativeWindowObject(){
  hash.clear();
}

void NativeWindowObject::addFrameWinID(WId fid){
  frameid = fid;
}

void NativeWindowObject::addDamageID(unsigned int dmg){
  dmgID = dmg;
}

bool NativeWindowObject::isRelatedTo(WId tmp){
  return (relatedTo.contains(tmp) || winid == tmp || frameid == tmp);
}

WId NativeWindowObject::id(){
  return winid;
}

WId NativeWindowObject::frameId(){
  return frameid;
}

unsigned int NativeWindowObject::damageId(){
  return dmgID;
}

QVariant NativeWindowObject::property(NativeWindowObject::Property prop){
  if(hash.contains(prop)){ return hash.value(prop); }
  else if(prop == NativeWindowObject::RelatedWindows){ return QVariant::fromValue(relatedTo); }
  return QVariant(); //null variant
}

void NativeWindowObject::setProperty(NativeWindowObject::Property prop, QVariant val, bool force){
  if(prop == NativeWindowObject::RelatedWindows){ relatedTo = val.value< QList<WId> >(); }
  else if(prop == NativeWindowObject::None || (!force && hash.value(prop)==val)){ return; }
  else if(prop == NativeWindowObject::WinImage){
    //special case - QImage is passed in, but QString is passed out (needed for QML)
    QByteArray ba;
    QBuffer buffer(&ba);
      buffer.open(QIODevice::WriteOnly);
      val.value<QImage>().save(&buffer, "PNG");
    QString img("data:image/png:base64,");
    img.append(QString::fromLatin1(ba.toBase64().data()));
    hash.insert(prop, img); //save the string instead
  }
  else{ hash.insert(prop, val); }
  emitSinglePropChanged(prop);
  emit PropertiesChanged(QList<NativeWindowObject::Property>() << prop, QList<QVariant>() << val);
}

void NativeWindowObject::setProperties(QList<NativeWindowObject::Property> props, QList<QVariant> vals, bool force){
  for(int i=0; i<props.length(); i++){
    if(i>=vals.length()){ props.removeAt(i); i--; continue; } //no corresponding value for this property
    if(props[i] == NativeWindowObject::None || (!force && (hash.value(props[i]) == vals[i])) ){
      props.removeAt(i); vals.removeAt(i); i--; continue; //Invalid property or identical value
    }else if(props[i] == NativeWindowObject::WinImage){
      //special case - QImage is passed in, but QString is passed out (needed for QML)
      QByteArray ba;
      QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        vals[i].value<QImage>().save(&buffer, "PNG");
      QString img("data:image/png:base64,");
      img.append(QString::fromLatin1(ba.toBase64().data()));
      hash.insert(props[i], img); //save the string instead
    }else{
      hash.insert(props[i], vals[i]);
    }
    emitSinglePropChanged(props[i]);
  }
  emit PropertiesChanged(props, vals);
}

void NativeWindowObject::requestProperty(NativeWindowObject::Property prop, QVariant val, bool force){
  if(prop == NativeWindowObject::None || prop == NativeWindowObject::RelatedWindows || (!force && hash.value(prop)==val) ){ return; }
  emit RequestPropertiesChange(winid, QList<NativeWindowObject::Property>() << prop, QList<QVariant>() << val);
}

void NativeWindowObject::requestProperties(QList<NativeWindowObject::Property> props, QList<QVariant> vals, bool force){
  //Verify/adjust inputs as needed
  for(int i=0; i<props.length(); i++){
    if(i>=vals.length()){ props.removeAt(i); i--; continue; } //no corresponding value for this property
    if(props[i] == NativeWindowObject::None || props[i] == NativeWindowObject::RelatedWindows || (!force && hash.value(props[i])==vals[i]) ){ props.removeAt(i); vals.removeAt(i); i--; continue; } //Invalid property or identical value
    /*if( (props[i] == NativeWindowObject::Visible || props[i] == NativeWindowObject::Active) && frameid !=0){
      //These particular properties needs to change the frame - not the window itself
      emit RequestPropertiesChange(frameid, QList<NativeWindowObject::Property>() << props[i], QList<QVariant>() << vals[i]);
      props.removeAt(i); vals.removeAt(i); i--;
    }*/
  }
  emit RequestPropertiesChange(winid, props, vals);
}

QRect NativeWindowObject::geometry(){
  //Calculate the "full" geometry of the window + frame (if any)
  //Check that the size is between the min/max limitations
  QSize size = hash.value(NativeWindowObject::Size).toSize();
  QSize min = hash.value(NativeWindowObject::MinSize).toSize();
  QSize max = hash.value(NativeWindowObject::MaxSize).toSize();
  if(min.isValid() && min.width() > size.width() ){ size.setWidth(min.width()); }
  if(min.isValid() && min.height() > size.height()){ size.setHeight(min.height()); }
  if(max.isValid() && max.width() < size.width()  && max.width()>min.width()){ size.setWidth(max.width()); }
  if(max.isValid() && max.height() < size.height()  && max.height()>min.height()){ size.setHeight(max.height()); }
  //Assemble the full geometry
  QRect geom( hash.value(NativeWindowObject::GlobalPos).toPoint(), size );
  //Now adjust the window geom by the frame margins
  QList<int> frame = hash.value(NativeWindowObject::FrameExtents).value< QList<int> >(); //Left,Right,Top,Bottom
  //qDebug() << "Calculate Geometry:" << geom << frame;
  if(frame.length()==4){
    geom = geom.adjusted( -frame[0], -frame[2], frame[1], frame[3] );
  }
  //qDebug() << " - Total:" << geom;
  return geom;
}

// QML ACCESS FUNCTIONS (shortcuts for particular properties in a format QML can use)
QString NativeWindowObject::winImage(){
  return this->property(NativeWindowObject::WinImage).toString();
}

QString NativeWindowObject::name(){
  return this->property(NativeWindowObject::Name).toString();
}

QString NativeWindowObject::title(){
  return this->property(NativeWindowObject::Title).toString();
}

QString NativeWindowObject::shortTitle(){
  QString tmp = this->property(NativeWindowObject::ShortTitle).toString();
  if(tmp.isEmpty()){ tmp = title(); }
  if(tmp.isEmpty()){ tmp = name(); }
  return tmp;
}

QIcon NativeWindowObject::icon(){
  return this->property(NativeWindowObject::Name).value<QIcon>();
}

//QML Button states
bool NativeWindowObject::showCloseButton(){
  QList<NativeWindowObject::Type> types = this->property(NativeWindowObject::WinTypes).value<QList < NativeWindowObject::Type> >();
  QList<NativeWindowObject::Type> badtypes;
  badtypes << NativeWindowObject::T_DESKTOP << NativeWindowObject::T_TOOLBAR << NativeWindowObject::T_MENU \
	<< NativeWindowObject::T_SPLASH << NativeWindowObject::T_DROPDOWN_MENU << NativeWindowObject::T_POPUP_MENU \
	<< NativeWindowObject::T_NOTIFICATION << NativeWindowObject::T_COMBO << NativeWindowObject::T_DND;
  for(int i=0; i<types.length(); i++){
    if(badtypes.contains(types[i])){ return false; }
  }
  return true;
}

bool NativeWindowObject::showMaxButton(){
  QList<NativeWindowObject::Type> types = this->property(NativeWindowObject::WinTypes).value<QList < NativeWindowObject::Type> >();
  QList<NativeWindowObject::Type> badtypes;
  badtypes << NativeWindowObject::T_DESKTOP << NativeWindowObject::T_TOOLBAR << NativeWindowObject::T_MENU \
	<< NativeWindowObject::T_SPLASH << NativeWindowObject::T_DROPDOWN_MENU << NativeWindowObject::T_POPUP_MENU \
	<< NativeWindowObject::T_NOTIFICATION << NativeWindowObject::T_COMBO << NativeWindowObject::T_DND;
  for(int i=0; i<types.length(); i++){
    if(badtypes.contains(types[i])){ return false; }
  }
  return true;
}

bool NativeWindowObject::showMinButton(){
  QList<NativeWindowObject::Type> types = this->property(NativeWindowObject::WinTypes).value<QList < NativeWindowObject::Type> >();
  QList<NativeWindowObject::Type> badtypes;
  badtypes << NativeWindowObject::T_DESKTOP << NativeWindowObject::T_TOOLBAR << NativeWindowObject::T_MENU \
	<< NativeWindowObject::T_SPLASH << NativeWindowObject::T_DROPDOWN_MENU << NativeWindowObject::T_POPUP_MENU \
	<< NativeWindowObject::T_NOTIFICATION << NativeWindowObject::T_COMBO << NativeWindowObject::T_DND << NativeWindowObject::T_DIALOG;
  for(int i=0; i<types.length(); i++){
    if(badtypes.contains(types[i])){ return false; }
  }
  return true;
}

bool NativeWindowObject::showTitlebar(){
  QList<NativeWindowObject::Type> types = this->property(NativeWindowObject::WinTypes).value<QList < NativeWindowObject::Type> >();
  QList<NativeWindowObject::Type> badtypes;
  badtypes << NativeWindowObject::T_DESKTOP << NativeWindowObject::T_TOOLBAR << NativeWindowObject::T_MENU \
	<< NativeWindowObject::T_SPLASH << NativeWindowObject::T_DROPDOWN_MENU << NativeWindowObject::T_POPUP_MENU \
	<< NativeWindowObject::T_NOTIFICATION << NativeWindowObject::T_COMBO << NativeWindowObject::T_DND;
  for(int i=0; i<types.length(); i++){
    if(badtypes.contains(types[i])){ return false; }
  }
  return true;
}

bool NativeWindowObject::showGenericButton(){
  QList<NativeWindowObject::Type> types = this->property(NativeWindowObject::WinTypes).value<QList < NativeWindowObject::Type> >();
  QList<NativeWindowObject::Type> badtypes;
  badtypes << NativeWindowObject::T_DESKTOP << NativeWindowObject::T_TOOLBAR << NativeWindowObject::T_MENU \
	<< NativeWindowObject::T_SPLASH << NativeWindowObject::T_DROPDOWN_MENU << NativeWindowObject::T_POPUP_MENU \
	<< NativeWindowObject::T_NOTIFICATION << NativeWindowObject::T_COMBO << NativeWindowObject::T_DND;
  for(int i=0; i<types.length(); i++){
    if(badtypes.contains(types[i])){ return false; }
  }
  return true;
}

bool NativeWindowObject::showWindowFrame(){
  QList<NativeWindowObject::Type> types = this->property(NativeWindowObject::WinTypes).value<QList < NativeWindowObject::Type> >();
  QList<NativeWindowObject::Type> badtypes;
  badtypes << NativeWindowObject::T_DESKTOP << NativeWindowObject::T_TOOLBAR << NativeWindowObject::T_MENU \
	<< NativeWindowObject::T_SPLASH << NativeWindowObject::T_DROPDOWN_MENU << NativeWindowObject::T_POPUP_MENU \
	<< NativeWindowObject::T_NOTIFICATION << NativeWindowObject::T_COMBO << NativeWindowObject::T_DND;
  for(int i=0; i<types.length(); i++){
    if(badtypes.contains(types[i])){ return false; }
  }
  return true;
}

//QML Window States
bool NativeWindowObject::isSticky(){
  return (this->property(NativeWindowObject::Workspace).toInt()<0 || this->property(NativeWindowObject::States).value<QList<NativeWindowObject::State> >().contains(NativeWindowObject::S_STICKY) );
}

int NativeWindowObject::workspace(){
  return this->property(NativeWindowObject::Workspace).toInt();
}

//QML Geometry reporting
QRect NativeWindowObject::frameGeometry(){
  return geometry();
}

QRect NativeWindowObject::imageGeometry(){
  QRect geom( this->property(NativeWindowObject::GlobalPos).toPoint(), this->property(NativeWindowObject::Size).toSize() );
  return geom;
}

// ==== PUBLIC SLOTS ===
void NativeWindowObject::toggleVisibility(){
  setProperty(NativeWindowObject::Visible, !property(NativeWindowObject::Visible).toBool() );
}

void NativeWindowObject::requestClose(){
  emit RequestClose(winid);
}

void NativeWindowObject::requestKill(){
  emit RequestKill(winid);
}

void NativeWindowObject::requestPing(){
  emit RequestPing(winid);
}

// ==== PRIVATE ====
void NativeWindowObject::emitSinglePropChanged(NativeWindowObject::Property prop){
  //Simple switch to emit the QML-usable signals as properties are changed
  switch(prop){
	case NativeWindowObject::Name:
		emit nameChanged(); break;
	case NativeWindowObject::Title:
		emit titleChanged(); break;
	case NativeWindowObject::ShortTitle:
		emit shortTitleChanged(); break;
	case NativeWindowObject::Icon:
		emit iconChanged(); break;
	case NativeWindowObject::Workspace:
	case NativeWindowObject::States:
		emit stickyChanged(); break;
	case NativeWindowObject::WinImage:
		emit winImageChanged(); break;
	case NativeWindowObject::WinTypes:
		emit winTypeChanged(); break;
	default:
		break; //do nothing otherwise
  }
}
