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
	id: "netButton"
	property OSInterface os: RootObject.os_interface()
	iconName: os.networkIcon
	tooltip: os.networkStatus
	visible: os.networkAvailable
	enabled: os.hasNetworkManager()
	onClicked: {
		RootObject.launchApplication(os.networkManagerUtility())
	}
}
