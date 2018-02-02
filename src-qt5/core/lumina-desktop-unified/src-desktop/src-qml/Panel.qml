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
	columns: (object.width>object.height) ? -1 : 1
	rows: (object.width>object.height) ? 1 : -1

	//hardcode the "status_tray" plugin at the moment
	PLUGINS.StatusTray{
		id: "statustray"
		Layout.fillHeight: layout.parent.object.isVertical ? false : true
		Layout.fillWidth: layout.parent.object.isVertical ? true : false

	}
	PLUGINS.Clock_Digital{
		id: "clock"
		Layout.fillHeight: layout.parent.object.isVertical ? false : true
		Layout.fillWidth: layout.parent.object.isVertical ? true : false
	}
    }

}
