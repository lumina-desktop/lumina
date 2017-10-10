import QtQuick 2.0
import QtGraphicalEffects 1.0
import "." as QML

Rectangle {
  id : canvas
  anchors.fill: parent
  color: "black"
  
  Repeater {
      model: Math.round(Math.random()*canvas.width/10)+100
     QML.Firefly {
       parent: canvas
       x: Math.round(Math.random()*canvas.width)
       y: Math.round(Math.random()*canvas.height)
     }
  }  //end of Repeater

} //end of canvas rectangle
