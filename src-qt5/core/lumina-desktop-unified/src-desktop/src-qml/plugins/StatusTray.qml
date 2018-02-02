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
	property bool vertical
	property int prefsize: vertical ? parent.width : parent.height
	id: "status_tray"

	GridLayout{
		anchors.fill: parent
		flow: parent.vertical ? GridLayout.TopToBottom : GridLayout.LeftToRight
		columns: vertical ? 2 : -1
		rows: vertical ? -1 : 2
		//columnSpacing: 2
		//rowSpacing: 2

		//Volume Status
		QML.VolumeButton{
			//Layout.preferredHeight: status_tray.prefsize
			//Layout.preferredWidth: status_tray.prefsize
		}
		//Network Status
		QML.NetworkButton{
			//Layout.preferredHeight: status_tray.prefsize
			//Layout.preferredWidth: status_tray.prefsize
		}
		//Battery Status
		QML.BatteryButton{
			//Layout.preferredHeight: status_tray.prefsize
			//Layout.preferredWidth: status_tray.prefsize
		}
		//Update Status
		QML.UpdateButton{
			//Layout.preferredHeight: status_tray.prefsize
			//Layout.preferredWidth: status_tray.prefsize
		}
		//System Tray Menu Button
		ToolButton{
			id: "trayButton"
			text: "Tray"
			//iconName: "view-more-vertical"
			visible: RootObject.hasTrayWindows
			onClicked: trayMenu.open()
			menu: Menu{
				id: "trayMenu"
				//MenuItem{ text: "sample" }
				//MenuItem{ text: "sample2" }
				//Rectangle{ color: "blue"; width: 50; height: 50 }
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
