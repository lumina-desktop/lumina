import QtQuick 2.7
import QtGraphicalEffects 1.0

Rectangle {
  id : canvas
  anchors.fill: parent
  width: 800
  height: 600
  color: "black"

  //Between 5 and 15 planets, read from settings
  property var planets: Math.round(( Math.random() * 10 ) + 5 )

  //Create planets
  Repeater {
     model: planets

     Rectangle {
       id : index
       parent: canvas

       //Place the planet randomly on the canvas, but not too close to the edge
       x: Math.round(Math.random()*canvas.width)

       //Check to make sure the planets are not too close to the sun (outside a 50px radius)
       //while( Math.round(Math.random()*canvas.width) < (width/2 + 50) or Math.round(Math.random()*canvas.width) > (width/2 - 50))

       y: Math.round(Math.random()*canvas.height)


       //Create the orbit animation

       //Planet size between 14 and 32 pixels
       width: Math.round(1.75 * (((Math.random() * 10) + 8 )))
       height: width

       //Make each rectangle look circular
       radius: width / 2

       //Give each planet a random color, semi-transparent
       color: Qt.rgba(Math.random(), Math.random(), Math.random(), 0.5)
     }
  }

  //Create the star
  Rectangle{
    id: star
    parent: canvas

    //Centers in star in the center of the canvas 
    x: Math.round(canvas.width / 2)
    y: Math.round(canvas.height / 2)

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

    //border.width: 4
    //border.color: "blue"
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

  /*Motion timer
  Timer {
    interval: 1
    repeat: true
    running: true
    property bool starting: true

    onTriggered: {
      if(starting) { interval = 3010; starting = false; }

    }
  }*/
  
  Component.onCompleted: {
    //console.log(Math.random())
  } 
}
