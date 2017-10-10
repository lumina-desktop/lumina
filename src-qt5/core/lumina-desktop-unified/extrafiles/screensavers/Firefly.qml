import QtQuick 2.0
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0

Item {

  RectangularGlow {
    anchors.fill: fly
    glowRadius: 0.1
    spread: 0.1
    color: Qt.rgba(Math.random()*255,Math.random()*255,0,0.3)
    cornerRadius: fly.radius + glowRadius
  }

  Rectangle {
    id: fly
    width: Math.round(Math.random()*3)+2
    height: width
    color: Qt.rgba(Math.random()*255,Math.random()*255,0,0.8)    
    radius: Math.floor(width/2)
    property int jitterX: Math.round(Math.random()*100)+10
    property int jitterY: Math.round(Math.random()*100)+10
      
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

      Timer {
        interval: Math.round(Math.random()*1000)
         repeat: true
         running: true
         onTriggered: {
              if ( (x+fly.jitterX)>canvas.width || (x+fly.jitterX)<0 ){ fly.jitterX = 0-fly.jitterX }
              x = x+fly.jitterX
              if( (y+fly.jitterY)>canvas.height || (y+fly.jitterY)<0 ){ fly.jitterY = 0-fly.jitterY }
              y = y+fly.jitterY
              fly.jitterX = (Math.round(Math.random())*2 - 1) *fly.jitterX
              fly.jitterY = (Math.round(Math.random())*2 - 1) *fly.jitterY
            fly.color = Qt.rgba(Math.random()*255,Math.random()*255,Math.random()*150,0.8)
           
         }
     } //end of timer
} //end of item
