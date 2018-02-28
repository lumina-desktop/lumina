//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 1
import QtQuick.Layouts 1.3

import Lumina.Backend.PanelObject 2.0

Rectangle {
    //C++ backend object
    property string panel_id
    property PanelObject object

    id: panel
    //Normal geometries/placements
    color: object.background
    x: object.x
    y: object.y
    width: object.width
    height: object.height

    GridLayout{
	id: layout
	anchors.fill: parent
	//columns: parent.object.isVertical ? 1 : -1
	//rows: parent.object.isVertical ? -1 : 1
	flow: parent.isVertical ? GridLayout.TopToBottom : GridLayout.LeftToRight
	//horizontalItemAlignment: parent.object.isVertical ? Grid.AlignHCenter : Qt.AlignLeft
	//verticalItemAlignment: parent.object.isVertical ? Grid.AlignTop : Qt.AlignVCenter
	Repeater { 
		model: panel.object.plugins
		Loader{
			asynchronous: true
			property bool vertical : layout.parent.object.isVertical
			property bool isspacer : modelData.endsWith("/Spacer.qml");
			source: modelData
			Layout.fillWidth : (vertical || isspacer) ? true : false
			Layout.fillHeight : (vertical && ! isspacer) ? false : true
			Layout.alignment : Qt.AlignVCenter | Qt.AlignHCenter
		}

	}
    } //end of grid layout
}
