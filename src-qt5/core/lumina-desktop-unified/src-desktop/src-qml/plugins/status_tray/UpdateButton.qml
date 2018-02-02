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
	id: "updateButton"
	property OSInterface os: RootObject.os_interface()
	iconName: os.updateIcon
	tooltip: os.updateStatus
	visible: os.updateInfoAvailable
	/*enabled: os.hasUpdateManager()
	onClicked: {
		RootObject.launchApplication(os.updateManagerUtility())
	}*/
	//TODO - add a menu with update manager and start/view updates options
}
