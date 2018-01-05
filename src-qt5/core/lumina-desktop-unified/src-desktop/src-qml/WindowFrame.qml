// vi: ft=qml
import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.4 
import QtQuick.Layouts 1.3

Rectangle {
  id: background
  color: "grey"

  Rectangle {
    id: windowFrame
    border.width: 5
    border.color: "black"
    color: "white"
    width: 400 
    height: 300

    MouseArea {
      id: resizeArea
      anchors.fill: parent
      property int positionX: 0
      property int positionY: 0
      property int newWidth: 0
      property int newHeight: 0

      onPositionChanged: { 
        var globalP = windowFrame.mapToItem(background, mouse.x, mouse.y)
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
            console.log("oldHeight: ", windowFrame.height);
            windowFrame.height -= 1
            console.log("newHeight: ", windowFrame.height);
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
        PropertyChanges { target: canvas; color:"red" }
      },
      State {
        when: dragArea.drag.held
        AnchorChanges { target: windowFrame; anchors.verticalCenter: undefined; anchors.horizontalCenter: undefined }
      }

    ]


    Rectangle {
      id: titleBar
      border.width: 2
      color: "black" 
      height: 25
      anchors.top: windowFrame.top
      anchors.right: windowFrame.right
      anchors.left: windowFrame.left
      anchors.margins: windowFrame.border.width
      width: parent.width

      RowLayout {
        anchors.right: parent.right
        spacing: 0

        Button {
          iconName: "window-minimize"
          //action: 
        }

        Button {
          iconName: "window-maximize"
          //action: 
        }

        Button {
          iconName: "document-close"
          //action: 
        }

      }

      Button {
        iconName: "emblem-synchronized"
        anchors.left: parent.left
      }

      Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        color: "white" 
        text: "zwelch@trueos~8905:~/lumina/src-qt5/src-qml/test"
        font.pixelSize: 10
      }

      MouseArea {
        acceptedButtons: Qt.RightButton
        anchors.fill: parent
        //onClicked: contextMenu.open()
      }
    }

    Image {
      id: frameContents
//      source: "balloon.png"
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
        height: parent.height; 
        anchors.fill: frameContents; 
        onClicked: { console.log(parent.mapToGlobal(mouse.x, mouse.y)); }

      }
    }
  }
}
