//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class governs all the xcb/randr interactions
//  and provides simpler Qt-based functions for use elsewhere
//===========================================

//Qt includes
#include <QSize>

#include "xcb/randr.h"

class outputDevice{
public:
  QString id; //output ID
  bool enabled;
  //Monitor Geometry
  QPoint geom; //geometry of monitor within session
  //Monitor Resolution
  QSize cRes; //current resolution of the monitor (could be different from geom.size() if panning is enabled)
  QList<QSize> availRes; //available resolutions supported by the monitor
  //Refresh Rate
  int cHz; //current refresh rate
  QList<int> availHz; //available refresh rates
  //Expand this later to include:
  // panning (current/possible)
  // rotation (current/possible)

 //FUNCTIONS
  
  //Modification
};
