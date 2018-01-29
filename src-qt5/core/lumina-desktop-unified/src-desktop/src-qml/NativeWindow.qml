// vi: ft=qml
import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

import Lumina.Backend.NativeWindowObject 2.0
import Lumina.Backend.RootDesktopObject 2.0

Rectangle {
  property NativeWindowObject object
  property string window_id

  SystemPalette { id:palette }

  id: windowFrame
  visible: object.isVisible
  border.width: 5
  border.color: palette.highlight
  radius: 5
  color: palette.window //"transparent"
  x: object.frameGeometry.x
  y: object.frameGeometry.y
  width: object.frameGeometry.width
  height: object.frameGeometry.height

  onXChanged: {
    windowFrame.object.updateGeometry(windowFrame.x, windowFrame.y, windowFrame.width, windowFrame.height)
  }
  onYChanged: {
    windowFrame.object.updateGeometry(windowFrame.x, windowFrame.y, windowFrame.width, windowFrame.height)
  }

  MouseArea {
    id: resizeArea
    anchors.fill: parent
    drag.target: undefined
    property int resizeDirection: NativeWindowObject.TOP_LEFT
    property int positionX: -1
    property int positionY: -1

    onPressed: {
      //NOTE: This is only triggered for resize events
      var globalP = windowFrame.mapToItem(rootCanvas, mouse.x, mouse.y)
      positionX = globalP.x
      positionY = globalP.y
      if(positionY <= windowFrame.y + 10 ) {
        if(positionX <= windowFrame.x + 10)
          resizeDirection = NativeWindowObject.TOP_LEFT
        else if(positionX >= windowFrame.x + windowFrame.width - 10)
          resizeDirection = NativeWindowObject.TOP_RIGHT
        else
          resizeDirection = NativeWindowObject.TOP
      }else if(positionY >= windowFrame.y + windowFrame.height - 10) {
        if(positionX <= windowFrame.x + 10)
          resizeDirection = NativeWindowObject.BOTTOM_LEFT
        else if(positionX >= windowFrame.x + windowFrame.width - 10)
          resizeDirection = NativeWindowObject.BOTTOM_RIGHT
        else
          resizeDirection = NativeWindowObject.BOTTOM
      }else if(positionX <= windowFrame.x + 10) {
        resizeDirection = NativeWindowObject.LEFT
      }else if(positionX >= windowFrame.x + windowFrame.width - 10) {
        resizeDirection = NativeWindowObject.RIGHT
      }
      //console.log("Initial X: ", positionX, "Initial Y: ", positionY);
      //console.log("Initial X Frame: ", windowFrame.x, "Initial Y Frame: ", windowFrame.y);
    }

    onReleased: {
      positionX = -1
      positionY = -1
      //windowFrame.object.updateGeometry(windowFrame.x, windowFrame.y, windowFrame.width, windowFrame.height)
    }

    onPositionChanged: {
      //NOTE: This is only triggered for resize events
      if(positionX != -1 && positionY != -1) {
        var globalP = windowFrame.mapToItem(rootCanvas, mouse.x, mouse.y)
        /*console.log("Global P: ", globalP);
        console.log("Position X: ", positionX, "Position Y: ", positionY)
        console.log("Old Position : ", windowFrame.x, " , ", windowFrame.y)
        console.log(resizeDirection);*/
        if(resizeDirection == NativeWindowObject.TOP_LEFT) {
          windowFrame.height -= globalP.y - positionY
          windowFrame.width -= globalP.x - positionX
          windowFrame.y = globalP.y
          windowFrame.x = globalP.x
        }else if(resizeDirection == NativeWindowObject.TOP_RIGHT) {
          //console.log("TOP RIGHT Old Height: ", windowFrame.height, "Old Width: ", windowFrame.width)
          windowFrame.height -= globalP.y - positionY
          windowFrame.width += globalP.x - positionX
          //console.log("New Height: ", windowFrame.height, "New Width: ", windowFrame.width)
          windowFrame.y = globalP.y
          //console.log("New Position : ", windowFrame.x, " , ", windowFrame.y)
        }else if(resizeDirection == NativeWindowObject.TOP) {
          windowFrame.height -= globalP.y - positionY
          windowFrame.y = globalP.y
        } else if(resizeDirection == NativeWindowObject.RIGHT) {
          windowFrame.width += globalP.x - positionX
        } else if(resizeDirection == NativeWindowObject.BOTTOM_RIGHT) {
          windowFrame.height += globalP.y - positionY
          windowFrame.width += globalP.x - positionX
        } else if(resizeDirection == NativeWindowObject.BOTTOM) {
          windowFrame.height += globalP.y - positionY
        } else if(resizeDirection == NativeWindowObject.BOTTOM_LEFT) {
          windowFrame.width -= globalP.x - positionX
          windowFrame.height += globalP.y - positionY
          windowFrame.x = globalP.x
        } else if(resizeDirection == NativeWindowObject.LEFT) {
          windowFrame.width -= globalP.x - positionX
          windowFrame.x = globalP.x
        }
        //Set a miniumum width and height as 80x50
        if(windowFrame.width < 80) {
          windowFrame.width = 80 
        }
        if(windowFrame.height < 50) {
          windowFrame.height = 50 
        }
        positionY = globalP.y
        positionX = globalP.x
      }
      windowFrame.object.updateGeometry(windowFrame.x, windowFrame.y, windowFrame.width, windowFrame.height)
    }
  }

  Rectangle {
    id: titleBar
    border.width: 0
    color: palette.window
    height: 25
    anchors.top: windowFrame.top
    anchors.right: windowFrame.right
    anchors.left: windowFrame.left
    anchors.margins: windowFrame.border.width
    width: parent.width

    MouseArea {
      id: dragArea
      anchors.fill: parent
      drag.target: windowFrame
      drag.axis: Drag.XAndYAxis
      //acceptedButtons: Qt.RightButton
      //onClicked: contextMenu.open()
      //released: { function(); }
    }

    ToolButton {
      id: otherButton
      anchors.left: parent.left
      height: parent.height
      iconSource: windowFrame.object.icon
    }

    Text {
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.verticalCenter: parent.verticalCenter
      color: palette.windowText
      text: windowFrame.object.shortTitle
      fontSizeMode: Text.Fit
    }

    RowLayout {
      spacing: 0
      anchors.right: parent.right
      height: parent.height

      ToolButton {
        id: minButton
        Layout.fillHeight: true
        iconName: "window-minimize"
        onClicked: { windowFrame.object.toggleVisibility() }
      }

      ToolButton {
        id: maxButton
        Layout.fillHeight: true
        iconName: "window-maximize"
        //onClicked: { windowFrame.object.toggleMaximize() }
      }

      ToolButton {
        id: closeButton
        Layout.fillHeight: true
        iconName: "document-close"
        onClicked: { windowFrame.object.requestClose() }
      }
    }
  }

  Image {
    id: frameContents
    cache: false
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
    //color: palette.window

    //Image {
      //anchors.fill: frameContents
      //cache: false
      //source: windowFrame.object.winImage
    //}

    MouseArea { 
      width: parent.width
      height: parent.height
      anchors.fill: frameContents
      onClicked: { console.log(parent.mapToGlobal(mouse.x, mouse.y)); }
      onPositionChanged: {
        RootObject.mousePositionChanged()
      }
    }
  }
}
