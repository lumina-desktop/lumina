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
import "./plugins" as PLUGINS

Rectangle {
    //C++ backend object
    property string panel_id
    property PanelObject object

    //Normal geometries/placements
    color: object.background
    x: object.x
    y: object.y
    width: object.width
    height: object.height

    GridLayout{
	id: layout
	anchors.fill: parent;
	columns: parent.object.isVertical ? 1 : -1
	rows: parent.object.isVertical ? -1 : 1

	//hardcode some plugins for the moment
	//PLUGINS.Spacer{}

	PLUGINS.StatusTray{
		id: "statustray"
		vertical: layout.parent.object.isVertical
		//Layout.fillWidth: layout.parent.object.isVertical ? true : false
		//Layout.fillHeight: layout.parent.object.isVertical ? false : true
	}

	//PLUGINS.Spacer{}

	PLUGINS.Clock_Digital{
		id: "clock"
		vertical: layout.parent.object.isVertical
		//Layout.fillWidth: layout.parent.object.isVertical ? true : false
		//Layout.fillHeight: layout.parent.object.isVertical ? false : true
	}

	//PLUGINS.Spacer{}

    } //end of grid layout
}
