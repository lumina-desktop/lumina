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

inline bool loadScreenInfo(p_objects *p_obj, xcb_randr_monitor_info_t *info){
  if(p_obj->monitor_atom == 0){ p_obj->monitor_atom = info->name; }
  if(p_obj->name.isEmpty()){ p_obj->name = atomToName(info->name); }
  //Now update all the info in the cache
  p_obj->primary = (info->primary == 1);
  p_obj->automatic = (info->automatic == 1);
  p_obj->geometry = QRect(info->x, info->y, info->width, info->height);
  p_obj->physicalSizeMM = QSize(info->width_in_millimeters, info->height_in_millimeters);
  //Load the "outputs"
  /*p_obj->outputs.clear();
  int out_len =  xcb_randr_monitor_info_outputs_length(info);
  for(int i=0; i<out_len; i++){ p_obj->outputs << xcb_randr_monitor_info_outputs(info)[i]; }
  qDebug() << "Info Loaded:" << p_obj->name;
  for(int i=0; i<p_obj->outputs.length(); i++){*/
    xcb_randr_get_output_info_reply_t *info = xcb_randr_get_output_info_reply(QX11Info::connection(),
		xcb_randr_get_output_info_unchecked(QX11Info::connection(), p_obj->output, QX11Info::appTime()),
		NULL);
    if(info==0){ continue; } //bad output
    //Modes
    int mode_len = xcb_randr_get_output_info_modes_length(info);
    //qDebug() << "Number of Modes:" << mode_len;
    if(mode_len<=0){ continue; } //skip this output - not a physical screen which can be used
     p_obj->modes.clear();
    for(int j=0; j<mode_len; j++){
      p_obj->modes.append( xcb_randr_get_output_info_modes(info)[j] );
    }
  //}
  //qDebug() << "INFO:" << p_obj->name;
  //qDebug() << "Found Outputs:" << p_obj->outputs;
  //qDebug() << "Found Modes:" << p_obj->modes;
  p_obj->resolutions.clear();
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
  return true;
}

/*class OutputDevice::p_objects{
public:
	xcb_atom_t monitor_atom; //This is the index used to identify particular monitors (unique ID)

	//Cached Information
	bool primary, automatic;
	QRect geometry;
	QList<QSize> resolutions;
	QSize physicalSizeMM;
	QString name;
	QList<xcb_randr_output_t> outputs;

	p_objects(){
          // Set the defaults for non-default-constructed variables
	  primary = automatic = false;
	  monitor_atom = 0;
	}

};*/

//Global Listing of Devices
QList<OutputDevice> OutputDevice::availableMonitors(){
  QList<OutputDevice> list;
  //Get the list of monitors
  xcb_randr_get_monitors_cookie_t cookie = xcb_randr_get_monitors_unchecked(QX11Info::connection(), QX11Info::appRootWindow(), 1);
  xcb_randr_get_monitors_reply_t *reply = xcb_randr_get_monitors_reply(QX11Info::connection(), cookie, NULL);
  if(reply==0){
    qDebug() << "Could not get monitor list";
    return list;
  }
  xcb_randr_monitor_info_iterator_t iter = xcb_randr_get_monitors_monitors_iterator(reply);
  qDebug() << "Number of Monitors:" << xcb_randr_get_monitors_monitors_length(reply);
  while(iter.rem>0){
    qDebug() << "Found Monitor:";
    //qDebug() << " Index:" << iter.index << "Rem:" << iter.rem;
    QString name = atomToName(iter.data->name);
    /*xcb_get_atom_name_reply_t *nreply = xcb_get_atom_name_reply(QX11Info::connection(), xcb_get_atom_name_unchecked(QX11Info::connection(), iter.data->name), NULL);
    QString name = QString::fromLocal8Bit(xcb_get_atom_name_name(nreply), xcb_get_atom_name_name_length(nreply));
    free(nreply);*/

    qDebug() << "  - Name:" << iter.data->name << name;
    qDebug() << "  - Primary:" << (iter.data->primary == 1);
    qDebug() << "  - Automatic:" << (iter.data->automatic == 1);
    qDebug() << "  - nOutput:" << iter.data->nOutput;
    qDebug() << "  - Geometry:" << QRect(iter.data->x, iter.data->y, iter.data->width, iter.data->height);
    qDebug() << "  - Physical Size (mm):" << iter.data->width_in_millimeters << "x" << iter.data->height_in_millimeters;
    qDebug() << "  - Number of outputs:" << xcb_randr_monitor_info_outputs_length(iter.data);
    xcb_randr_monitor_info_next(&iter);
  }

  //Free up any objects we are done with
  free(reply);
  //Return the list
  return list;
}

//FUNCTIONS (do not use directly - use the static list function instead)
OutputDevice::OutputDevice(QString id){
  //p_obj = new p_objects();
  p_obj.name = id;
  p_obj.primary = p_obj.automatic = false;
  p_obj.monitor_atom = 0;
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
bool OutputDevice::isAutomatic(){ return p_obj.automatic; }
bool OutputDevice::isConnected(){ return !p_obj.modes.isEmpty(); }

QList<QSize> OutputDevice::availableResolutions(){ return p_obj.resolutions; }
QSize OutputDevice::currentResolution(){ return p_obj.geometry.size(); } //no concept of panning/scaling yet
QRect OutputDevice::currentGeometry(){ return p_obj.geometry; }
QSize OutputDevice::physicalSizeMM(){ return p_obj.physicalSizeMM; }

//Modification
bool OutputDevice::setAsPrimary(bool set){
  if(p_obj.primary == set){ return true; } //no change needed
  bool ok = false;
  for(int i=0; i<p_obj.outputs.length(); i++){
    if(set){ xcb_randr_set_output_primary (QX11Info::connection(), QX11Info::appRootWindow(), p_obj.outputs[i]); }
    p_obj.primary = set; //Only need to push a "set" primary up through XCB - will automatically deactivate the other monitors
    ok = true;
    break;
  }
  return ok;
}

bool OutputDevice::disable(){
  //qDebug() << "Disable Monitor:" << p_obj.monitor_atom;
  if(p_obj.monitor_atom!=0){
    //qDebug() << " - Go ahead";
    for(int o=0; o<p_obj.outputs.length(); o++){
      for(int m=0; m<p_obj.modes.length(); m++){
        qDebug() << "Deleting Mode for Output:" << "Mode:" << p_obj.modes[m] << "Output:" << p_obj.outputs[o];
        //XLib version
        //XRRDeleteOutputMode(QX11Info::display(), p_obj.outputs[o], p_obj.modes[m]);
        //XCB version
        xcb_randr_delete_output_mode(QX11Info::connection(), p_obj.outputs[o], p_obj.modes[m]);
        xcb_flush(QX11Info::connection());
      }
    }
    //xcb_randr_delete_monitor_checked(QX11Info::connection(), QX11Info::appRootWindow(), p_obj.monitor_atom);
    //p_obj.monitor_atom = 0;
    return true;
  }
  return false;
}

void OutputDevice::enable(QRect geom){
  //if no geom provided, will add as the right-most screen at optimal resolution
  if(p_obj.monitor_atom!=0){ return; }
  qDebug() << "Enable Monitor:" << geom;

}

void OutputDevice::changeResolution(QSize){

}

void OutputDevice::updateInfoCache(){
  xcb_randr_get_screen_resources_reply_t *reply = xcb_randr_get_screen_resources_reply(QX11Info::connection(),
		xcb_randr_get_screen_resources_unchecked(QX11Info::connection(), QX11Info::appRootWindow()),
		NULL);
  int outputnum = xcb_randr_get_screen_resources_outputs_length(reply);
  for(int i=0; i<outputnum; i++){
    xcb_randr_output_t output = xcb_randr_get_screen_resources_outputs(reply)[i];
    if(p_obj->output==0){
      //Need to detect the name for this output (inefficient - better to pass in the output number directly)
      xcb_randr_get_output_info_reply_t *info = xcb_randr_get_output_info_reply(QX11Info::connection(),
		xcb_randr_get_output_info_unchecked(QX11Info::connection(), output, QX11Info::appTime()),
		NULL);
      //Name
      QString name = QString::fromLocal8Bit( (char*) xcb_randr_get_output_info_name(info), xcb_randr_get_output_info_name_length(info));
      if(
  }

    //Find the **active** monitor with the given id/name
  if(p_obj.monitor_atom !=0 || !p_obj.name.isEmpty() ){
    bool found = false;
    for(int i=0; i<2 && !found; i++){
      xcb_randr_get_monitors_cookie_t cookie = xcb_randr_get_monitors_unchecked(QX11Info::connection(), QX11Info::appRootWindow(), (i==0 ? 1 : 0) ); //toggle between active/not monitors
      xcb_randr_get_monitors_reply_t *reply = xcb_randr_get_monitors_reply(QX11Info::connection(), cookie, NULL);
      if(reply!=0){
        xcb_randr_monitor_info_iterator_t iter = xcb_randr_get_monitors_monitors_iterator(reply);
        //qDebug() << "Number of Monitors:" << xcb_randr_get_monitors_monitors_length(reply);
        while(iter.rem>0){
          if( p_obj.monitor_atom == iter.data->name || p_obj.name == atomToName(iter.data->name) ){
            loadScreenInfo(&p_obj, iter.data);
            found = true;
            break; //Finished with the information for this particular monitor
          }
          xcb_randr_monitor_info_next(&iter);
        }
        free(reply);
      } //end check for reply structure
    } //end loop over active/inactive monitor state
  } //end loading of active/enabled monitor information

}

// ============================
//             OutputDeviceList
// ============================

OutputDeviceList::OutputDeviceList(){
  xcb_randr_get_screen_resources_reply_t *reply = xcb_randr_get_screen_resources_reply(QX11Info::connection(),
		xcb_randr_get_screen_resources_unchecked(QX11Info::connection(), QX11Info::appRootWindow()),
		NULL);
  int outputnum = xcb_randr_get_screen_resources_outputs_length(reply);
  for(int i=0; i<outputnum; i++){
    xcb_randr_output_t output = xcb_randr_get_screen_resources_outputs(reply)[i];
    //Now display the info about this output
    xcb_randr_get_output_info_reply_t *info = xcb_randr_get_output_info_reply(QX11Info::connection(),
		xcb_randr_get_output_info_unchecked(QX11Info::connection(), output, QX11Info::appTime()),
		NULL);
    //Name
    QString name = QString::fromLocal8Bit( (char*) xcb_randr_get_output_info_name(info), xcb_randr_get_output_info_name_length(info));
    OutputDevice dev(name);
    out_devs.append(dev); //add to the internal list
  }
  //QList<xcb_atom_t> usedOutputs;
  //Get the information about all the "enabled" monitors
  /*for(int i=0; i<2; i++){ //loop over active/inactive monitors
    qDebug() << "Scanning For Monitors:" << (i==0 ? "active" : "inactive");
    xcb_randr_get_monitors_cookie_t cookieA = xcb_randr_get_monitors_unchecked(QX11Info::connection(), QX11Info::appRootWindow(), (i==0 ? 1 : 10)); //toggle active/inactive monitors
    xcb_randr_get_monitors_reply_t *replyA = xcb_randr_get_monitors_reply(QX11Info::connection(), cookieA, NULL);
    if(replyA!=0){
      xcb_randr_monitor_info_iterator_t iter = xcb_randr_get_monitors_monitors_iterator(replyA);
      qDebug() << "Number of Monitors:" << xcb_randr_get_monitors_monitors_length(replyA);
      while(iter.rem>0){
        //qDebug() << "Found Monitor:";
        //qDebug() << " Index:" << iter.index << "Rem:" << iter.rem;
        if(!usedOutputs.contains(iter.data->name)){
          QString name = atomToName(iter.data->name);
          OutputDevice dev(name);
          usedOutputs << iter.data->name;
          out_devs.append(dev); //add to the internal list
        }
        xcb_randr_monitor_info_next(&iter);
      }
      //Free up any objects we are done with
      free(replyA);
    } //end loading of active/enabled monitors
  } //end loop over active/inactive monitors
 */
  qDebug() << "=========================";
  //Now get the information about any **UNUSED** monitors/outputs
  xcb_randr_get_screen_resources_reply_t *reply = xcb_randr_get_screen_resources_reply(QX11Info::connection(),
		xcb_randr_get_screen_resources_unchecked(QX11Info::connection(), QX11Info::appRootWindow()),
		NULL);
  int outputnum = xcb_randr_get_screen_resources_outputs_length(reply);
  qDebug() << "Probing Screen Resources:";
  qDebug() << " - Number of Outputs:" << outputnum;
  //qDebug() << " - Number of CRTC's:" << xcb_randr_get_screen_resources_crtcs_length(reply);
  //int mode_len =xcb_randr_get_screen_resources_modes_length(reply);
  //qDebug() << " - Modes:" << mode_len;
    /*for(int m=0; m<mode_len; m++){
      xcb_randr_mode_info_t mode = xcb_randr_get_screen_resources_modes(reply)[m];
      //qDebug() << " -- Mode:" << mode.id;
      qDebug() << "    - Size Option:" << mode.width <<"x"<<mode.height;
    }*/
  for(int i=0; i<outputnum; i++){
    xcb_randr_output_t output = xcb_randr_get_screen_resources_outputs(reply)[i];
    //Now display the info about this output
    xcb_randr_get_output_info_reply_t *info = xcb_randr_get_output_info_reply(QX11Info::connection(),
		xcb_randr_get_output_info_unchecked(QX11Info::connection(), output, QX11Info::appTime()),
		NULL);
    qDebug() << "==== Output Information #"+QString::number(i);
    //Name
    int name_len = xcb_randr_get_output_info_name_length(info);
    qDebug() << "Name:" << QString::fromLocal8Bit( (char*) xcb_randr_get_output_info_name(info), name_len);

    //Modes
    int mode_len = xcb_randr_get_output_info_modes_length(info);
    qDebug() << "Number of Modes:" << mode_len;
    if(mode_len<=0){ continue; } //skip this output - not a physical screen which can be used


    //Clones
    qDebug() << "Number of Clones:" << xcb_randr_get_output_info_clones_length(info);

    //Properties
   /* xcb_randr_list_output_properties_reply_t *pinfo = xcb_randr_list_output_properties_reply(QX11Info::connection(),
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
    free(pinfo);
     */
    free(info);
  }

  free(reply);
}

OutputDeviceList::~OutputDeviceList(){

}

//Simplification functions for dealing with multiple monitors
void OutputDeviceList::setPrimaryMonitor(QString id){

}

void OutputDeviceList::disableMonitor(QString id){
  for(int i=0; i<out_devs.length(); i++){
    if(out_devs[i].ID() == id){
        out_devs[i].disable();
        out_devs[i].updateInfoCache();
      break;
    }
  }
}
