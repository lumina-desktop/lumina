//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 1

import Lumina.Backend.ScreenObject 2.0
import Lumina.Backend.PanelObject 2.0

import "." as QML

AnimatedImage {
    //C++ backend object
    property string screen_id
    property ScreenObject object

    //Normal geometries/placements
    asynchronous: true
    clip: true
    source: object.background
    x: object.x
    y: object.y
    width: object.width
    height: object.height

  //Setup the Panels
  Repeater{
    model: object.panels
    QML.Panel{
      panel_id: modelData
      object: parent.object.panel(panel_id)
      z: 10000+index
    }
  }

  }
