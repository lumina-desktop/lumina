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
import Lumina.Backend.NativeWindowObject 2.0

import "./status_tray" as QML


Item {
	property int prefsize: parent.vertical ? parent.width : parent.height
	id: "status_tray"
	anchors.fill: parent

	GridLayout{
		anchors.fill: parent
		flow: status_tray.parent.vertical ? GridLayout.TopToBottom : GridLayout.LeftToRight
		//columns: vertical ? 1 : -1
		//rows: vertical ? -1 : 1
		columnSpacing: 2
		rowSpacing: 2

		//Volume Status
		QML.VolumeButton{
			//Layout.preferredHeight: status_tray.prefsize
			//Layout.preferredWidth: status_tray.prefsize
			Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
		}
		//Network Status
		QML.NetworkButton{
			//Layout.preferredHeight: status_tray.prefsize
			//Layout.preferredWidth: status_tray.prefsize
			Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
		}
		//Battery Status
		QML.BatteryButton{
			//Layout.preferredHeight: status_tray.prefsize
			//Layout.preferredWidth: status_tray.prefsize
			Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
		}
		//Update Status
		QML.UpdateButton{
			//Layout.preferredHeight: status_tray.prefsize
			//Layout.preferredWidth: status_tray.prefsize
			Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
		}
		//System Tray Menu Button
		ToolButton{
			id: "trayButton"
			text: "Tray"
			Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
			//iconName: "view-more-vertical"
			visible: RootObject.hasTrayWindows
			onClicked: trayMenu.open()
			menu: Menu{
				id: "trayMenu"
				//MenuItem{ text: "sample" }
				//MenuItem{ text: "sample2" }
				Rectangle{ color: "blue"; width: 48; height: 48 }
				GridLayout{ 
					columns: 4
					Repeater{
						model: RootObject.trayWindows
						QML.TrayIcon{

							winObj: RootObject.trayWindow(modelData)
							parent: trayMenu
							Layout.preferredWidth: 48
							Layout.preferredHeight: 48
						}
					}
				}
			}
		 } //end of system tray menu button
		/*Repeater{
			model: RootObject.trayWindows
			QML.TrayIcon{
				winObj: RootObject.trayWindow(modelData)
				Layout.preferredHeight: status_tray.prefsize
				Layout.preferredWidth: status_tray.prefsize
			}
		}*/
	} //end of layout
  
}
