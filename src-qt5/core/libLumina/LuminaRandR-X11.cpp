//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LuminaRandR.h"

//#include "X11/extensions/Xrandr.h"

inline QString atomToName(xcb_atom_t atom){
  xcb_get_atom_name_reply_t *nreply = xcb_get_atom_name_reply(QX11Info::connection(), xcb_get_atom_name_unchecked(QX11Info::connection(), atom), NULL);
    QString name = QString::fromLocal8Bit(xcb_get_atom_name_name(nreply), xcb_get_atom_name_name_length(nreply));
    free(nreply);
  return name;
};

//More efficient method for converting lots of atoms to strings
inline QStringList atomsToNames(xcb_atom_t *atoms, unsigned int num){
  //qDebug() << "atomsToNames:" << num;
  QList< xcb_get_atom_name_cookie_t > cookies;
  //qDebug() << " - Get cookies";
  for(unsigned int i=0; i<num; i++){ cookies << xcb_get_atom_name_unchecked(QX11Info::connection(), atoms[i]);  }
  QStringList names;
  //qDebug() << " - Get names";
  for(int i=0; i<cookies.length(); i++){
    xcb_get_atom_name_reply_t *nreply = xcb_get_atom_name_reply(QX11Info::connection(), cookies[i], NULL);
    if(nreply==0){ continue; }
      names << QString::fromLocal8Bit(xcb_get_atom_name_name(nreply), xcb_get_atom_name_name_length(nreply));
    free(nreply);
  }
  return names;
};

inline bool loadScreenInfo(p_objects *p_obj){
  //Reset the primary cached values (just in case things error out below and it can't finish)
  p_obj->current_mode = 0;
  p_obj->geometry = QRect();
  p_obj->physicalSizeMM = QSize();
  p_obj->primary = false;
  p_obj->modes.clear();
  p_obj->resolutions.clear();
  p_obj->crtc = 0;

  //Get the information associated with the output and save it in the p_objects cache
  xcb_randr_get_output_info_reply_t *info = xcb_randr_get_output_info_reply(QX11Info::connection(),
		xcb_randr_get_output_info_unchecked(QX11Info::connection(), p_obj->output, QX11Info::appTime()),
		NULL);
  if(info==0){ return false; } //bad output value
  //First read off the information associated with the output itself
  if(p_obj->name.isEmpty()){ p_obj->name = QString::fromLocal8Bit( (char*) xcb_randr_get_output_info_name(info), xcb_randr_get_output_info_name_length(info)); }
  p_obj->physicalSizeMM = QSize(info->mm_width, info->mm_height);

    //Modes
    int mode_len = xcb_randr_get_output_info_modes_length(info);
    for(int j=0; j<mode_len; j++){
      p_obj->modes.append( xcb_randr_get_output_info_modes(info)[j] );
    }
    //int pref_len = info->num_preferred;
    //qDebug() << "Modes:" << p_obj->modes << "Num Preferred:" << pref_len;
    /*for(int j=0; j<pref_len; j++){
      p_obj->preferred.append( xcb_randr_get_output_info_preferred(info)[j] );
    }*/
  p_obj->crtc = info->crtc;
  free(info); //done with output_info

  //Now load the current status of the output (crtc information)
  xcb_randr_get_crtc_info_reply_t *cinfo = xcb_randr_get_crtc_info_reply(QX11Info::connection(),
		xcb_randr_get_crtc_info_unchecked(QX11Info::connection(), p_obj->crtc, QX11Info::appTime()),
		NULL);
  if(cinfo!=0){
    p_obj->geometry = QRect(cinfo->x, cinfo->y, cinfo->width, cinfo->height);
    p_obj->current_mode = cinfo->mode;
    free(cinfo); //done with crtc_info
  }

  if(!p_obj->modes.isEmpty()){
    //And see if this output is currently the primary output
    xcb_randr_get_output_primary_reply_t *preply = xcb_randr_get_output_primary_reply(QX11Info::connection(),
		xcb_randr_get_output_primary_unchecked(QX11Info::connection(), QX11Info::appRootWindow()), NULL);

    if(preply !=0){
      p_obj->primary = (preply->output == p_obj->output);
      free(preply);
    }

    //Now load all the screen resources information, and find matches for the current modes
    xcb_randr_get_screen_resources_reply_t *srreply = xcb_randr_get_screen_resources_reply(QX11Info::connection(),
		xcb_randr_get_screen_resources_unchecked(QX11Info::connection(), QX11Info::appRootWindow()), NULL);
    if(srreply!=0){
      for(int i=0; i<xcb_randr_get_screen_resources_modes_length(srreply); i++){
        xcb_randr_mode_info_t minfo = xcb_randr_get_screen_resources_modes(srreply)[i];
        if(p_obj->modes.contains(minfo.id)){
          QSize sz(minfo.width, minfo.height);
          if(!p_obj->resolutions.contains(sz)){ p_obj->resolutions.append( sz); }
        }
      }
      free(srreply);
    }
  }
  return true;
}


inline xcb_randr_mode_t modeForResolution(QSize res, QList<xcb_randr_mode_t> modes){
  xcb_randr_mode_t det_mode = XCB_NONE;
  xcb_randr_get_screen_resources_reply_t *srreply = xcb_randr_get_screen_resources_reply(QX11Info::connection(),
		xcb_randr_get_screen_resources_unchecked(QX11Info::connection(), QX11Info::appRootWindow()), NULL);
  if(srreply!=0){
    unsigned int refreshrate = 0;
    QSize sz;
    for(int i=0; i<xcb_randr_get_screen_resources_modes_length(srreply); i++){
      xcb_randr_mode_info_t minfo = xcb_randr_get_screen_resources_modes(srreply)[i];
      if(modes.contains(minfo.id)){
       //qDebug() << "Found mode!" << minfo.id << res << refreshrate;
        if(res.isNull() && (minfo.width > sz.width() || minfo.height > sz.height()) ){
          //No resolution requested - pick the largest one
          //qDebug() << "Found Bigger Mode:" << sz << QSize(minfo.width, minfo.height);
          sz = QSize(minfo.width, minfo.height);
          det_mode = minfo.id;
        }else if(!res.isNull()){
          sz = QSize(minfo.width, minfo.height);
           //qDebug() << "Compare Sizes:" << sz << res;
          if(sz == res && minfo.dot_clock > refreshrate){ det_mode = minfo.id; refreshrate = minfo.dot_clock; }
        }
      }
    }
    free(srreply);
  }
  return det_mode;
}

inline void adjustScreenTotal(xcb_randr_crtc_t output, QRect geom, bool addingoutput){
  QRect total, mmTotal;
  xcb_randr_get_screen_resources_reply_t *srreply = xcb_randr_get_screen_resources_reply(QX11Info::connection(),
		xcb_randr_get_screen_resources_unchecked(QX11Info::connection(), QX11Info::appRootWindow()), NULL);
  if(srreply!=0){
    for(int i=0; i<xcb_randr_get_screen_resources_crtcs_length(srreply); i++){
      xcb_randr_crtc_t crtc = xcb_randr_get_screen_resources_crtcs(srreply)[i];
      if(output == crtc){
        //Found the output we are (going) to treat differently
        if(addingoutput){
          total = total.united(geom);
        }
        //ignore the output if we just removed it
      }else{
        //Get the current geometry of this crtc (if available) and add it to the total
        xcb_randr_get_crtc_info_reply_t *cinfo = xcb_randr_get_crtc_info_reply(QX11Info::connection(),
		xcb_randr_get_crtc_info_unchecked(QX11Info::connection(), crtc, QX11Info::appTime()),
		NULL);
        if(cinfo!=0){
          total = total.united( QRect(cinfo->x, cinfo->y, cinfo->width, cinfo->height) );
	   //QSize dpi( qRound((cinfo->width * 25.4)/cinfo->), qRound((p_obj.geometry.height() * 25.4)/p_obj.physicalSizeMM.height() ) );
          free(cinfo); //done with crtc_info
        }
      }
    }
    free(srreply);
  }
  QSize newRes = total.size();
  QSize newMM = mmTotal.size();
  xcb_randr_set_screen_size(QX11Info::connection(), QX11Info::appRootWindow(), newRes.width(), newRes.height(), newMM.width(), newMM.height());
}

inline bool showOutput(QRect geom, p_objects *p_obj){
  //if no geom provided, will add as the right-most screen at optimal resolution
  qDebug() << "Enable Monitor:" << geom;
  xcb_randr_mode_t mode = modeForResolution(geom.size(), p_obj->modes);
  if(mode==XCB_NONE){ qDebug() << "[ERROR] Invalid resolution supplied!"; return false; } //invalid resolution for this monitor
  //qDebug() << " - Found Mode:" << mode;
  if(p_obj->crtc == 0){
    //Need to scan for an available crtc to use (turning on a monitor for the first time)
    xcb_randr_get_screen_resources_reply_t *reply = xcb_randr_get_screen_resources_reply(QX11Info::connection(),
		xcb_randr_get_screen_resources_unchecked(QX11Info::connection(), QX11Info::appRootWindow()),
		NULL);
    int num = xcb_randr_get_screen_resources_crtcs_length(reply);
    for(int i=0; i<num && p_obj->crtc==0; i++){
      xcb_randr_crtc_t crt = xcb_randr_get_screen_resources_crtcs(reply)[i];
      xcb_randr_get_crtc_info_reply_t *info = xcb_randr_get_crtc_info_reply(QX11Info::connection(),
		xcb_randr_get_crtc_info_unchecked(QX11Info::connection(), crt, QX11Info::appTime()),
		NULL);
      //Verify that the output is supported by this crtc
      QList<xcb_randr_output_t> possible;
      if(xcb_randr_get_crtc_info_outputs_length(info) < 1){ //make sure it is not already associated with an output
        int pnum = xcb_randr_get_crtc_info_possible_length(info);
        for(int p=0; p<pnum; p++){ possible << xcb_randr_get_crtc_info_possible(info)[p]; }
      }
      if(possible.contains(p_obj->output)){ p_obj->crtc = crt; }
      free(info);
    }
    free(reply);
  }
if(p_obj->crtc == 0){ qDebug() << "[ERROR] No Available CRTC devices for display"; return false; }
  //Now need to update the overall session size (if necessary)
  adjustScreenTotal(p_obj->crtc, geom, true); //adding output at this geometry

  //qDebug() << " - Using crtc:" << p_obj->crtc;
  //qDebug() << " - Using mode:" << mode;
  xcb_randr_output_t outList[1]{ p_obj->output };

  xcb_randr_set_crtc_config_cookie_t cookie = xcb_randr_set_crtc_config_unchecked(QX11Info::connection(), p_obj->crtc,
		XCB_CURRENT_TIME, XCB_CURRENT_TIME, geom.x(), geom.y(), mode, XCB_RANDR_ROTATION_ROTATE_0, 1, outList);
    //Now check the result of the configuration
    xcb_randr_set_crtc_config_reply_t *reply = xcb_randr_set_crtc_config_reply(QX11Info::connection(), cookie, NULL);
    bool ok = false;
    if(reply!=0){ ok = (reply->status == XCB_RANDR_SET_CONFIG_SUCCESS); }
    free(reply);
    return ok;
}
/*
    //Clones
    qDebug() << "Number of Clones:" << xcb_randr_get_output_info_clones_length(info);
    //Properties
    xcb_randr_list_output_properties_reply_t *pinfo = xcb_randr_list_output_properties_reply(QX11Info::connection(),
		xcb_randr_list_output_properties_unchecked(QX11Info::connection(), output),
		NULL);
    int pinfo_len = xcb_randr_list_output_properties_atoms_length(pinfo);
    qDebug() << "Properties:" << pinfo_len;
    for(int p=0; p<pinfo_len; p++){
      xcb_atom_t atom = xcb_randr_list_output_properties_atoms(pinfo)[p];
      //Property Name
      QString name = atomToName(atom);
      //Property Value
      xcb_randr_query_output_property_reply_t *pvalue = xcb_randr_query_output_property_reply(QX11Info::connection(),
		xcb_randr_query_output_property_unchecked(QX11Info::connection(), output, atom),
		NULL);
      QStringList values = atomsToNames ( (xcb_atom_t*) xcb_randr_query_output_property_valid_values(pvalue), xcb_randr_query_output_property_valid_values_length(pvalue) ); //need to read values
      free(pvalue);
      qDebug() << " -- " << name << "=" << values;

    }
*/

//FUNCTIONS (do not use typically use manually - use the OutputDeviceList class instead)
OutputDevice::OutputDevice(QString id){
  //p_obj = new p_objects();
  p_obj.name = id;
  p_obj.primary = false;
  p_obj.output = 0;
  bool ok = false;
  p_obj.output = id.toInt(&ok);
  if(ok){
    //output ID number instead
    p_obj.name.clear();
  }
  updateInfoCache();
}

OutputDevice::~OutputDevice(){
  //delete p_obj;
}

// INFORMATION FUNCTIONS (simply read from cache)
QString OutputDevice::ID(){ return p_obj.name; }
bool OutputDevice::isEnabled(){ return !p_obj.geometry.isNull(); }
bool OutputDevice::isPrimary(){ return p_obj.primary; }
bool OutputDevice::isConnected(){ return !p_obj.modes.isEmpty(); }

QList<QSize> OutputDevice::availableResolutions(){ return p_obj.resolutions; }
QSize OutputDevice::currentResolution(){ return p_obj.geometry.size(); } //no concept of panning/scaling yet
QRect OutputDevice::currentGeometry(){ return p_obj.geometry; }
QSize OutputDevice::physicalSizeMM(){ return p_obj.physicalSizeMM; }
QSize OutputDevice::physicalDPI(){
  QSize dpi( qRound((p_obj.geometry.width() * 25.4)/p_obj.physicalSizeMM.width()), qRound((p_obj.geometry.height() * 25.4)/p_obj.physicalSizeMM.height() ) );
  return dpi;
}

//Modification
bool OutputDevice::setAsPrimary(bool set){
  if(p_obj.primary == set){ return true; } //no change needed
    if(set){ xcb_randr_set_output_primary (QX11Info::connection(), QX11Info::appRootWindow(), p_obj.output); }
    p_obj.primary = set; //Only need to push a "set" primary up through XCB - will automatically deactivate the other monitors
  return true;
}

bool OutputDevice::disable(){
  if(p_obj.output!=0 && p_obj.current_mode!=0 && p_obj.crtc!=0){
    //qDebug() << " - Go ahead";
    xcb_randr_set_crtc_config_cookie_t cookie = xcb_randr_set_crtc_config_unchecked(QX11Info::connection(), p_obj.crtc,
		XCB_CURRENT_TIME, XCB_CURRENT_TIME, 0, 0, XCB_NONE, XCB_RANDR_ROTATION_ROTATE_0, 0, NULL);
    //Now check the result of the configuration
    xcb_randr_set_crtc_config_reply_t *reply = xcb_randr_set_crtc_config_reply(QX11Info::connection(), cookie, NULL);
    if(reply==0){ return false; }
    bool ok = (reply->status == XCB_RANDR_SET_CONFIG_SUCCESS);
    free(reply);
    if(ok){
      adjustScreenTotal(p_obj.crtc, QRect(), false); //adding output at this geometry
    }
    return ok;
  }
  return false;
}

bool OutputDevice::enable(QRect geom){
  if(this->isEnabled()){ return false; } //already enabled
  return showOutput(geom, &p_obj);
}

bool OutputDevice::changeResolution(QSize res){
  if(!this->isEnabled()){ return false; }
  return showOutput( QRect( p_obj.geometry.topLeft(), res), &p_obj );
}

bool OutputDevice::move(QPoint pt){
  if(!this->isEnabled()){ return false; }
  return showOutput( QRect( pt, p_obj.geometry.size()), &p_obj);
}

bool OutputDevice::setGeometry(QRect geom){
  if(!this->isEnabled()){ return false; }
  return showOutput(geom, &p_obj);
}

void OutputDevice::updateInfoCache(){
  if(p_obj.output==0){
      //Only have a name (first run) - need to find the corresponding output for this ID
      xcb_randr_get_screen_resources_reply_t *reply = xcb_randr_get_screen_resources_reply(QX11Info::connection(),
		xcb_randr_get_screen_resources_unchecked(QX11Info::connection(), QX11Info::appRootWindow()),
		NULL);
    int outputnum = xcb_randr_get_screen_resources_outputs_length(reply);
    for(int i=0; i<outputnum; i++){
      xcb_randr_output_t output = xcb_randr_get_screen_resources_outputs(reply)[i];
      xcb_randr_get_output_info_reply_t *info = xcb_randr_get_output_info_reply(QX11Info::connection(),
		xcb_randr_get_output_info_unchecked(QX11Info::connection(), output, QX11Info::appTime()),
		NULL);
      //Compare names
      QString name = QString::fromLocal8Bit( (char*) xcb_randr_get_output_info_name(info), xcb_randr_get_output_info_name_length(info));
      free(info);
      if(name == p_obj.name){ p_obj.output = output; break; }
    }
    free(reply);
  }
  if(p_obj.output == 0){ return; } //bad ID/output?
  loadScreenInfo(&p_obj);
}

// ============================
//             OutputDeviceList
// ============================

OutputDeviceList::OutputDeviceList(){
  xcb_randr_get_screen_resources_reply_t *reply = xcb_randr_get_screen_resources_reply(QX11Info::connection(),
		xcb_randr_get_screen_resources_unchecked(QX11Info::connection(), QX11Info::appRootWindow()),
		NULL);
  if(reply==0){ return; } //could not get screen information
  int outputnum = xcb_randr_get_screen_resources_outputs_length(reply);
  for(int i=0; i<outputnum; i++){
    xcb_randr_output_t output = xcb_randr_get_screen_resources_outputs(reply)[i];
    OutputDevice dev(QString::number(output)); //use the raw output integer
    out_devs.append(dev); //add to the internal list
  }
  free(reply);
}

OutputDeviceList::~OutputDeviceList(){

}

//Simplification functions for dealing with multiple monitors
void OutputDeviceList::setPrimaryMonitor(QString id){
  for(int i=0; i<out_devs.length(); i++){
    out_devs[i].setAsPrimary(out_devs[i].ID() == id);
  }
}

QString OutputDeviceList::primaryMonitor(){
  for(int i=0; i<out_devs.length(); i++){
    if(out_devs[i].isPrimary()){ return out_devs[i].ID(); }
  }
  return "";
}

bool OutputDeviceList::disableMonitor(QString id){
  bool ok = false;
  for(int i=0; i<out_devs.length(); i++){
    if(out_devs[i].ID() == id){
        ok = out_devs[i].disable();
        out_devs[i].updateInfoCache();
      break;
    }
  }
  return ok;
}

bool OutputDeviceList::enableMonitor(QString id, QRect geom){
  bool ok = false;
  for(int i=0; i<out_devs.length(); i++){
    if(out_devs[i].ID() == id){
        ok = out_devs[i].enable(geom);
        out_devs[i].updateInfoCache();
      break;
    }
  }
  return ok;
}
