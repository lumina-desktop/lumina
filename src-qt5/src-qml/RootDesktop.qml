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
//    root->engine()->rootContext->setContextProperty("RootObject", rootobj);
//===========================================
import QtQuick 2.0
import QtQuick.Window 2.2

Rectangle {
  id: RootCanvas
  color: "black"

  //Setup the wallpapers
  Repeater{
    model: RootObject.screens
    AnimatedImage {
      id: modelData.name
      asynchronous: true
      clip: true
      source: modelData.background
      x: modelData.screen.virtualX
      y: modelData.screen.virtualY
      width: modelData.screen.width
      height: modelData.screen.height
    }
  }
}
