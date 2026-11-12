//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 2

import Lumina.Backend.RootDesktopObject 2.0



Menu { 
    id: contextMenu

    MenuItem {
      text: "Launch Terminal"
      //iconName: "utilities-terminal"
      onTriggered: {
        RootObject.launchApp("application/terminal")
      }
    }
    MenuItem {
      text: "Launch File Browser"
      //iconName: "user-home"
      onTriggered: {
        RootObject.launchApp("inode/directory")
      }
    }

    MenuItem {
      text: "Lock Screen"
      //iconName: "system-lock-screen"
      onTriggered: {
        RootObject.lockscreen()
      }
    }

    MenuItem {
      text: "Logout"
      //iconName: "system-log-out"
      onTriggered: {
        RootObject.logout()
      }
    }
  }
