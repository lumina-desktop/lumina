//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 1

import Lumina.Backend.PanelObject 2.0

Rectangle {
    //C++ backend object
    property string panel_id
    property PanelObject object

    //Normal geometries/placements
    color: object.background
    x: object.x
    y: object.y
    width: object.width
    height: object.height
  }
