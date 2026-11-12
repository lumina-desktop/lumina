//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
import QtQuick 2.2
import QtQuick.Controls 2

import Lumina.Backend.NativeWindowObject 2.0

Image{
	property NativeWindowObject winObj
	source: winObj.icon
	id: "trayIcon"
	fillMode: Qt.PreserveAspectFit
	smooth: true
	cache: false
	asynchronous: false
	//text: winObj.name

	onXChanged: { }

	MouseArea{
		//property point globalPos
		anchors.fill: parent
		acceptedButtons: Qt.NoButton
		onEntered: {
			//Need to ensure the invisible native window is over this place right now
			console.log("Enter Tray Icon:", parent.mapToGlobal(0,0));
			winObj.updateGeometry( parent.mapToGlobal(0,0).x, parent.mapToGlobal(0,0).y, parent.width(), parent.height(), true)
		}
	}
}
