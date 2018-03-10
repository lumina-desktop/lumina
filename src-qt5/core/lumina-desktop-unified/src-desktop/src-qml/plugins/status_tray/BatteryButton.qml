//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
import QtQuick 2.2
import QtQuick.Controls 1

import Lumina.Backend.RootDesktopObject 2.0
import Lumina.Backend.OSInterface 2.0

ToolButton{
	property OSInterface os: RootObject.os_interface()
	id: "batButton"
	iconName: os.batteryIcon
	tooltip: os.batteryStatus
	visible: os.batteryAvailable()
	//enabled: false
}
