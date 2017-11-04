import QtQuick 2.2
import QtQuick.Layouts 1.3

Rectangle {
  id: canvas
  color: "black"

  ColumnLayout{
    anchors.fill: canvas

    Repeater{
      model: cppdata.list
      Text{
        color: "white"
        text: modelData
      }
    }
  } //end row layout
}
