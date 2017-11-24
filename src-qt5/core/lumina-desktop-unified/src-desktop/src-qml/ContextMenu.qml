//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 1

import Lumina.Backend.RootDesktopObject 2.0

Menu { 
    id: contextMenu
    MenuItem {
      text: "Lock Screen"
      iconName: "system-lock-screen"
      onTriggered: {
        RootObject.lockscreen()
      }
    }

    MenuItem {
      text: "Logout"
      iconName: "system-log-out"
      onTriggered: {
        RootObject.logout()
      }
    }
  }
