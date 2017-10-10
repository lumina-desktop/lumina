import QtQuick 2.0
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0

Item {

  RectangularGlow {
    anchors.fill: fly
    glowRadius: Math.round(fly.radius /2)
    spread: 0.5
    color: Qt.rgba(1,1,1,0.3)
    cornerRadius: fly.radius + glowRadius
  }

  Rectangle {
    id: fly
    width: Math.round(Math.random()*canvas.width/200)+2
    height: width
    x: parent.x
    y: parent.y
    color: Qt.rgba(Math.random(),Math.random(),0,0.5)    
    radius: Math.floor(width/2)
    property int jitterX: Math.round(Math.random()*100)+10
    property int jitterY: Math.round(Math.random()*100)+10
     
    Behavior on color {
      ColorAnimation {
        duration: 500
      }
    }
   Behavior on x {
       SmoothedAnimation {
           velocity: 10+Math.random()*canvas.width/100
       }
   }
   Behavior on y {
       SmoothedAnimation {
           velocity: 10+Math.random()*canvas.height/100
       }
   }

  }


  
      Timer {
        interval: 5
         repeat: true
         running: true
         property bool starting: true
         onTriggered: {
              if(starting){ interval = Math.round(Math.random()*1000)+500; starting = false; }
              if ( (fly.x+fly.jitterX)>parent.width || (fly.x+fly.jitterX)<0 ){ fly.jitterX = 0-fly.jitterX }
              fly.x = fly.x+fly.jitterX
              if( (fly.y+fly.jitterY)>parent.height || (fly.y+fly.jitterY)<0 ){ fly.jitterY = 0-fly.jitterY }
              fly.y = fly.y+fly.jitterY
              fly.jitterX = (Math.round(Math.random())*2 - 1) *fly.jitterX
              fly.jitterY = (Math.round(Math.random())*2 - 1) *fly.jitterY
            fly.color = Qt.rgba(Math.random(),Math.random(),Math.random(),0.5)
           
         }
     } //end of timer
} //end of item
