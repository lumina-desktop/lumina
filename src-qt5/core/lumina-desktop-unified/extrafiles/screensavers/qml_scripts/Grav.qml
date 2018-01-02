import QtQuick 2.7
import QtGraphicalEffects 1.0

Rectangle {
  id : canvas
  anchors.fill: parent
  width: Screen.width 
  height: Screen.height
  color: "black"

  //TODO Add orbital trails option
  //TODO Fix jitteryness and start position
  //TODO Make orbits more extreme 

  //Between 5 and 15 planets, read from settings
  property int planets: Math.round(( Math.random() * 10 ) + 5 )
  property int cx: Math.round(width/2)
  property int cy: Math.round(height/2)

  //Create planets
  Repeater {
    id: planetRepeater
    model: planets

    Rectangle {
      id : index
      parent: canvas

      //Creates random distance for elipse
      property double c: Math.random() * 250
      property double b: Math.random() * 150 + c
      property double a: Math.sqrt(b*b+c*c)
      //Random angle of rotation
      property double th: Math.random() * Math.PI 
      property var path: []

      //Calculates starting position
      x: Math.round(cx + a * Math.cos(th))
      y: Math.round(cy + b * Math.sin(th))

      //Planet size between 14 and 32 pixels
      width: Math.round(1.75 * (((Math.random() * 10) + 8 )))
      height: width

      //Make each rectangle look circular
      radius: width / 2

      //Give each planet a random color, semi-transparent
      color: Qt.rgba(Math.random(), Math.random(), Math.random(), 0.5)

      /*Timer {
        //Each planet updates between 1ms and 51ms (smaller times=faster)
        interval: Math.round(Math.random() * 50 ) + 1
        repeat: true
        running: true
        property int time: 0

        onTriggered: {
          //Parametric equation that calculates the position of the general ellipse. Completes a loop ever 314 cycles. Credit to 
          x = cx+a*Math.cos(2*Math.PI*(time/314.0))*Math.cos(th) - b*Math.sin(2*Math.PI*(time/314.0))*Math.sin(th)
          y = cy+a*Math.cos(2*Math.PI*(time/314.0))*Math.sin(th) + b*Math.sin(2*Math.PI*(time/314.0))*Math.cos(th)
          time++;

          //Move a planet 80 pixels away from the sun if the planet is too close
          if(x > cx && Math.abs(cx-x) < 80) {
            x+=80
          }else if(x < cx && Math.abs(cx-x) < 80) {
            x-=80
          }

          if(y > cy && Math.abs(cy-y) < 80) {
            y+=80
          }else if(y < cy && Math.abs(cy-y) < 80) {
            y-=80
          }
        }
      }*/

      Component.onCompleted: {
        pahtX[0] = x
        pahtY[0] = y
        for(int i = 1; i <= 200; i++) {
          pathX[i] = cx+a*Math.cos(2*Math.PI*(i/200.0)*Math.cos(th) - b*Math.sin(2*Math.PI*(i/200.0)*Math.sin(th)
          pathY[i] = cy+a*Math.cos(2*Math.PI*(i/200.0)*Math.sin(th) + b*Math.sin(2*Math.PI*(i/200.0)*Math.cos(th)
        }
      }
    }
  }

  //Create the star
  Rectangle{
    id: star
    parent: canvas

    //Centers in star in the center of the canvas, with an offset to center the animation
    x: cx - 30
    y: cy - 30

    width: 60 
    height: width 

    //Create the wobble animation
    SequentialAnimation on height {
      loops: Animation.Infinite
      PropertyAnimation { duration: 2000; to: 90 }
      PropertyAnimation { duration: 2000; to: 60 }
    }

    SequentialAnimation on width {
      loops: Animation.Infinite
      PropertyAnimation { duration: 2000; to: 90 }
      PropertyAnimation { duration: 2000; to: 60 }
    }

    color: "black"
    radius: width / 2

    //Creates a radial gradient to make the star look cool
    RadialGradient {
      anchors.fill: parent
      gradient: Gradient { 
        GradientStop { position:0    ;color: Qt.rgba(0,0,0,0)}
        GradientStop { position:0.18 ;color: Qt.rgba(0,0,0,0)}
        GradientStop { position:0.2  ;color: Qt.rgba(0.32,0.47,0.30,0.13)}
        GradientStop { position:0.3  ;color: Qt.rgba(0.62,0.92,0.58,0.25)}
        GradientStop { position:0.4  ;color: Qt.rgba(1.00,0.93,0.59,0.51)}
        GradientStop { position:0.5  ;color: Qt.rgba(0,0,0,0)}
      }
    } 

  }
}
