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
import QtQuick.Controls 1

import Lumina.Backend.RootDesktopObject 2.0

Label{
	anchors.fill: parent
	text: RootObject.currentTime
	wrapMode: Text.WordWrap
	verticalAlignment: Text.AlignVCenter
	horizontalAlignment: Text.AlignHCenter
	rotation: parent.vertical ? 90 : 0
	transformOrigin: Item.Center
}
