import QtQuick 2.0
import QtMultimedia 5.7
import QtQuick.Window 2.2

Rectangle {
  width: Window.width
  height: Window.height
  color: "black"

  Row{
    id: masterRow
    anchors.left: parent.left
    anchors.horizontalCenter: parent.horizontalCenter
    width: parent.width
    spacing: 5 
    Repeater {
      id: cR
      model: Window.width / 15 + 1 
      Column {
        id: masterColumn
        width: 10
        Text { 
          id: column
          color: "#ff4d4d" 
          font.pixelSize: 10
          transform: Rotation { origin.x: 0; origin.y: 0; angle: 90 }
          Timer {
            interval: 50
            repeat: true
            running: true
            onTriggered: {
              if(Math.random() < 0.95) {
                var bottom = column.text.charAt(column.text.length-1)
                var newString = bottom+column.text.substring(0, column.text.length-1)
                column.text = newString 
                interval = 50
              }else{
                interval = 1000
              }
            }
          }
          Component.onCompleted: {
            var str = " "
            var numberChar = Math.random() * 100 + (Window.height * 0.1);
            if(Math.random() < 0.80) {
              while(str.length < numberChar) {
                if(Math.random() < 0.5) {
                  var charCount = Math.random() * 8 + 10
                  var segStr = ""
                  while(segStr.length < charCount) {
                    var randChar = String.fromCharCode(0x30A0 + Math.random() * (0x30FF-0x30A0+1));
                    segStr += randChar
                  }
                  str += segStr
                }else{
                  var charCount = Math.random() * 6 + 14
                  var segStr = ""
                  while(segStr.length < charCount) {
                    segStr += " "
                  }
                  str += segStr
                }
              }
            }
            column.text = str
          }
        }
      }
    }
  }
}
