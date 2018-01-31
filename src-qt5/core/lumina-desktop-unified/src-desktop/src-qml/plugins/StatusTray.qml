//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is the QML plugin that displays the OS status/system tray
//===========================================
import QtQuick 2.2
import QtQuick.Layouts 1.3

import "./status_tray" as QML


Rectangle {
	property int prefsize: (parent.width < parent.height) ? parent.width : parent.height
	id: "status_tray"
	color: "transparent"
	Layout.fillHeight: true
	Layout.fillWidth: true

	GridLayout{
		anchors.fill: parent
		flow: GridLayout.LeftToRight
		columnSpacing: 2
		rowSpacing: 2

		//Volume Status
		QML.VolumeButton{
			Layout.preferredHeight: status_tray.prefsize
			Layout.preferredWidth: status_tray.prefsize
		}
		//Network Status
		QML.NetworkButton{
			Layout.preferredHeight: status_tray.prefsize
			Layout.preferredWidth: status_tray.prefsize
		}
		//Battery Status
		QML.BatteryButton{
			Layout.preferredHeight: status_tray.prefsize
			Layout.preferredWidth: status_tray.prefsize
		}
		//System Tray Menu Popup
	
	} //end of layout
  
}
