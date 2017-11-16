//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is the base QML script the launches/controls the desktop interface itself
//===========================================
// NOTE: This needs to be paired/used with the corresponding C++ class: RootDesktopObject
//  Which should be added as the "RootObject" context property to the QML engine
//------------------
//  Example Code:
//  RootDesktopObject *rootobj = new RootDesktopObject();
//  QQuickView *root = new QQuickView();
//    root->setResizeMode(QQuickView::SizeRootObjectToView);
//    root->engine()->rootContext()->setContextProperty("RootObject", rootobj);
//===========================================
import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 1

import "." as QML

import Lumina.Backend.RootDesktopObject 2.0
import Lumina.Backend.ScreenObject 2.0

Rectangle {
  id: rootCanvas
  color: "black"

  //Setup the right-click context menu
  /*MouseArea { 
    anchors.fill: rootCanvas
    acceptedButtons: Qt.RightButton
    onClicked: { 
      contextMenu.x = mouseX
      contextMenu.y = mouseY
      contextMenu.open() 
    }
    onPositionChanged: {
      RootObject.mousePositionChanged()
    }
  }

  //Create the context menu itself
 QML.ContextMenu { id: contextMenu }*/

  //Setup the wallpapers
  Repeater{
    model: RootObject.screens
    QML.WallpaperImage{
      screen_id: modelData
      z: 0+index
    }
  }
}
