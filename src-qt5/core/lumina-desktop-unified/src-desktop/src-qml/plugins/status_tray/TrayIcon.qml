//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
import QtQuick 2.2
import QtQuick.Controls 1

import Lumina.Backend.NativeWindowObject 2.0

Image{
	property NativeWindowObject winObj
	source: winObj.icon

	MouseArea{
		anchors.fill: parent
		acceptedButtons: Qt.NoButton
		onEntered: {
			//Need to ensure the invisible native window is over this place right now
			point pt: parent.mapToGlobal(0,0)
			winObj.updateGeometry( pt.x(), pt.y(), parent.width(), parent.height(), true);
		}
	}
}
