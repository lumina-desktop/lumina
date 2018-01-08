// vi: ft=qml
import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

import Lumina.Backend.NativeWindowObject 2.0

Rectangle {
  property NativeWindowObject object
  property string window_id

  SystemPalette { id:palette }

  id: windowFrame
  border.width: 5
  //border.color: palette.window 
  color: palette.window 
  x: object.frameGeometry.x
  y: object.frameGeometry.y
  width: object.frameGeometry.width
  height: object.frameGeometry.height

  MouseArea {
    id: resizeArea
    anchors.fill: parent
    property int positionX: 0
    property int positionY: 0
    property int newWidth: 0
    property int newHeight: 0

    onPositionChanged: { 
      var globalP = windowFrame.mapToGlobal(mouse.x, mouse.y)
      if(positionY < windowFrame.y + 15 ) {
        /*if(positionX < windowFrame.x + 15) {
          console.log("Top Left");
          //Top Left 
          newWidth = windowFrame.width + (windowFrame.x - mouse.x) 
          newHeight = windowFrame.height + (windowFrame.y - mouse.y) 
          windowFrame.x = mouse.x
          windowFrame.y = mouse.y
        }else if(positionX > windowFrame.x + windowFrame.width - 15) {
          console.log("Top Right");
          //Top Right
          newX = positionX - mouse.x
          newY = positionY - mouse.y
          newWidth = windowFrame.width - (positionX - mouse.x)
          newHeight = windowFrame.height + (windowFrame.y - mouse.y)
          windowFrame.y = mouse.y
        }else{*/
          //Top
          windowFrame.height -= 1
          windowFrame.y = globalP.y
       // }
      }
/*else if(mouse.x < windowFrame.x + 15) {
        if(mouse.y > windowFrame.y + windowFrame.height - 15) {
          //Bottom Left
          newX = positionX - mouse.x
          newWidth = windowFrame.width - newX
          newHeight = windowFrame.height - newY
        }else{
          //Left
        }
      }else if(mouse.y > windowFrame.y + windowFrame.height - 15) {
        if(mouse.x > windowFrame.x + windowFrame.width - 15) {
          //Bottom Right
        }else{
          //Bottom
        }
      }else if(mouse.x > windowFrame.x + windowFrame.width - 15) {
        //Right
      } else {
        console.log("Cursor error");
      }*/
    }
  }

  MouseArea {
    id: dragArea
    anchors.fill: titleBar
    drag.target: windowFrame
    drag.axis: Drag.XAndYAxis
    onClicked: { console.log("dragArea"); }
    //released: { function(); }
  }

  states: [ 
    State {
      when: resizeArea.drag.held
      PropertyChanges { target: Window; color:"red" }
    },
    State {
      when: dragArea.drag.held
      AnchorChanges { target: windowFrame; anchors.verticalCenter: undefined; anchors.horizontalCenter: undefined }
    }

  ]


  Rectangle {
    id: titleBar
    border.width: 2
    color: palette.window 
    height: 25
    anchors.top: windowFrame.top
    anchors.right: windowFrame.right
    anchors.left: windowFrame.left
    anchors.margins: windowFrame.border.width
    width: parent.width

    RowLayout {
      anchors.fill: titleBar
      spacing: 0

      Button {
        id: otherButton
        anchors.left: parent.left
        Layout.fillHeight: true
        iconSource: windowFrame.object.icon
      }

      Text {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        color: palette.windowText
        text: windowFrame.object.shortTitle

        MouseArea {
          acceptedButtons: Qt.RightButton
          anchors.fill: parent
          //onClicked: contextMenu.open()
        }
      }

      Button {
        id: minButton
        Layout.fillHeight: true
        iconName: "window-minimize"
        onClicked: { windowFrame.object.toggleVisibility() }
      }

      Button {
        id: maxButton
        Layout.fillHeight: true
        iconName: "window-maximize"
        //onClicked: { windowFrame.object.toggleMaximize() }
      }

      Button {
        id: closeButton
        Layout.fillHeight: true
        iconName: "document-close"
        onClicked: { windowFrame.object.requestClose() }
      }
    }
  }

  Image {
    id: frameContents
    source: windowFrame.object.winImage
    anchors.top: titleBar.bottom
    anchors.bottom: parent.bottom
    anchors.left: windowFrame.left
    anchors.right: windowFrame.right
    anchors.leftMargin: windowFrame.border.width
    anchors.rightMargin: windowFrame.border.width
    anchors.bottomMargin: windowFrame.border.width
    width: parent.width
    height: parent.height

    MouseArea { 
      width: parent.width;
      height: parent.height
      anchors.fill: frameContents
      onClicked: { console.log(parent.mapToGlobal(mouse.x, mouse.y)); }

    }
  }
}
