import QtQuick 2.0
import "." as QML

Rectangle {
    id: root
    width: 800; height: 600
    color: "#646464"

    ListView {
        id: clockview
        anchors.fill: parent
        orientation: ListView.Horizontal
        cacheBuffer: 2000
        snapMode: ListView.SnapOneItem
        highlightRangeMode: ListView.ApplyRange

        delegate: QML.Clock { city: cityName; shift: timeShift }
        model: ListModel {
            ListElement { cityName: "New York"; timeShift: -4 }
            ListElement { cityName: "London"; timeShift: 0 }
            ListElement { cityName: "Oslo"; timeShift: 1 }
            ListElement { cityName: "Mumbai"; timeShift: 5.5 }
            ListElement { cityName: "Tokyo"; timeShift: 9 }
            ListElement { cityName: "Brisbane"; timeShift: 10 }
            ListElement { cityName: "Los Angeles"; timeShift: -8 }
        }
    }

    Image {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 10
        source: "content/arrow.png"
        rotation: -90
        opacity: clockview.atXBeginning ? 0 : 0.5
        Behavior on opacity { NumberAnimation { duration: 500 } }
    }

    Image {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        source: "content/arrow.png"
        rotation: 90
        opacity: clockview.atXEnd ? 0 : 0.5
        Behavior on opacity { NumberAnimation { duration: 500 } }
    }
}
