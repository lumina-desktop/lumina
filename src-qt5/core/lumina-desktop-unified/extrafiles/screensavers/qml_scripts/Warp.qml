import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle {
  id : canvas
  anchors.fill: parent
  color: "black"

  function getStarColor(num){
    if(num < 1) { return "white" }
    if(num < 2) { return "mistyrose" }
    return "lightblue"
  }  

  // CREATE STARFIELD
  Repeater {
      model: Math.round(Math.random()*canvas.width/10)+500
     Rectangle {
       parent: canvas
       x: Math.round(Math.random()*canvas.width)
       y: Math.round(Math.random()*canvas.height)
       width: Math.round(Math.random()*3)+3
       height: width
       radius: width/2
       color: getStarColor( (index%3) )

     }
  }  //end of Repeater

  // NOW CREATE THE WARP EFFECT
  ZoomBlur {
    id: blur
    anchors.fill: canvas
    source: canvas
    samples: 24
    length: canvas.width / 20
    horizontalOffset: 0
    verticalOffset: 0

    Behavior on horizontalOffset{
      NumberAnimation{
        duration: 3000
      }
    }
    Behavior on verticalOffset{
      NumberAnimation{
        duration: 3000
      }
    }
  } //end of zoom blur

  Timer {
        interval: 5
         repeat: true
         running: true
         property bool starting: true
         onTriggered: {
              if(starting){ interval = 3010; starting = false; }
              blur.horizontalOffset = (Math.random()*canvas.width/4) - (canvas.width/8)
              blur.verticalOffset = (Math.random()*canvas.height/4) - (canvas.height/8)
         }
     } //end of timer

} //end of canvas rectangle
