//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LuminaRandR.h"

#include "xcb/randr.h"
#include "xcb/xcb_atom.h"

#include <QDebug>
#include <QX11Info>

static QString atomToName(xcb_atom_t atom){
  xcb_get_atom_name_reply_t *nreply = xcb_get_atom_name_reply(QX11Info::connection(), xcb_get_atom_name_unchecked(QX11Info::connection(), atom), NULL);
    QString name = QString::fromLocal8Bit(xcb_get_atom_name_name(nreply), xcb_get_atom_name_name_length(nreply));
    free(nreply);
  return name;
};

//More efficient method for converting lots of atoms to strings
static QStringList atomsToNames(xcb_atom_t *atoms, unsigned int num){
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

class OutputDevice::p_objects{
public:
	xcb_atom_t monitor_atom;
	QList<xcb_randr_output_t> outputs; //the actual output devices used by the monitor

};

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
OutputDevice::OutputDevice(){
  enabled = false;
  p_obj = new p_objects();
  p_obj->monitor_atom = 0;
}

OutputDevice::~OutputDevice(){
  
}

//Modification
bool OutputDevice::setAsPrimary(){
  if(isPrimary){ return true; }
  if( !p_obj->outputs.isEmpty() ){
    xcb_randr_set_output_primary (QX11Info::connection(), QX11Info::appRootWindow(), p_obj->outputs[0]);
    isPrimary = true;
  }
  return isPrimary;
}

bool OutputDevice::disable(){
  if(p_obj->monitor_atom!=0){
    xcb_randr_delete_monitor(QX11Info::connection(), QX11Info::appRootWindow(), p_obj->monitor_atom);
    p_obj->monitor_atom = 0;
    return true; 
  }
  return false;
}

void OutputDevice::enable(QRect geom){
  //if no geom provided, will add as the right-most screen at optimal resolution
  if(p_obj->monitor_atom!=0){ return; }
  qDebug() << "Enable Monitor:" << geom;

}

void OutputDevice::changeResolution(QSize){

}

OutputDeviceList::OutputDeviceList(){
  xcb_randr_get_screen_resources_reply_t *reply = xcb_randr_get_screen_resources_reply(QX11Info::connection(),
		xcb_randr_get_screen_resources_unchecked(QX11Info::connection(), QX11Info::appRootWindow()),
		NULL);
  int outputnum = xcb_randr_get_screen_resources_outputs_length(reply);
  qDebug() << "Probing Screen Resources:";
  qDebug() << " - Number of Outputs:" << outputnum;
  qDebug() << " - Number of CRTC's:" << xcb_randr_get_screen_resources_crtcs_length(reply);
  int mode_len =xcb_randr_get_screen_resources_modes_length(reply);
  qDebug() << " - Modes:" << mode_len;
    for(int m=0; m<mode_len; m++){
      xcb_randr_mode_info_t mode = xcb_randr_get_screen_resources_modes(reply)[m];
      qDebug() << " -- Mode:" << mode.id;
      qDebug() << "    - Size:" << mode.width <<"x"<<mode.height;
    }
  //qDebug() << "  -- " << atomsToNames( (xcb_atom_t*) xcb_randr_get_screen_resources_modes(reply), xcb_randr_get_screen_resources_modes_length(reply) );
  qDebug() << " - Names:" << xcb_randr_get_screen_resources_names_length(reply);
  //qDebug() << "  -- " << atomsToNames( (xcb_atom_t*) xcb_randr_get_screen_resources_names(reply), xcb_randr_get_screen_resources_names_length(reply));
  for(int i=0; i<outputnum; i++){
    xcb_randr_output_t output = xcb_randr_get_screen_resources_outputs(reply)[i];
    //Now display the info about this output
    xcb_randr_get_output_info_reply_t *info = xcb_randr_get_output_info_reply(QX11Info::connection(), 
		xcb_randr_get_output_info_unchecked(QX11Info::connection(), output, QX11Info::appTime()),
		NULL);
    qDebug() << "==== Output Information #"+QString::number(i);

    //Modes
    qDebug() << "Number of Modes:" << xcb_randr_get_output_info_modes_length(info);


    //Clones    
    qDebug() << "Number of Clones:" << xcb_randr_get_output_info_clones_length(info);

    //Names
    int name_len = xcb_randr_get_output_info_name_length(info);
    qDebug() << "Names:" << atomsToNames( (xcb_atom_t*) xcb_randr_get_output_info_name(info), name_len);
    for(int n=0; n<name_len; n++){
      QString name = atomToName( xcb_randr_get_output_info_name(info)[n] );
      qDebug() << " -- " << name;
    }

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
      /*for(int v=0; v<xcb_randr_query_output_property_valid_values_length(pvalue); v++){
        //values << QString::number(xcb_randr_query_output_property_valid_values(pvalue)[v] );
        values << atomToName( xcb_randr_query_output_property_valid_values(pvalue)[v] );
      }*/
      free(pvalue);
      qDebug() << " -- " << name << "=" << values;

    }
    free(pinfo);

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

}
