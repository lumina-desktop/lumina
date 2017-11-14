//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 2.0

import Lumina.Backend.RootDesktopObject 2.0

Menu { 
    id: contextMenu
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    MenuItem {
      text: "Lock Screen"
      onTriggered: {
        RootObject.lockscreen()
      }
    }

    MenuItem {
      text: "Logout"
      //iconName: "system-log-out"
      indicator: Image{
        asynchronous: true
        //autoTransform: true
        //source: "image://theme/system-logout"
        source: "file:///usr/local/share/icons/material-design-light/scalable/actions/system-log-out.svg"
      }
      onTriggered: {
        RootObject.logout()
      }
    }
  }
