import QtQuick 2.0
import QtMultimedia 5.7
import QtQuick.Window 2.2
import Qt.labs.folderlistmodel 2.1

Rectangle {
  width: Screen.width
  height: Screen.height
  color: "black"

  FolderListModel {
    id: folderModel 
    folder: "/usr/local/videos"
  } 

  Repeater {
    model: folderModel
    Component {
      Item { 
        Component.onCompleted: { playlist.addItem(fileURL) }
      }
    }
  }

  Playlist {
    id: playlist
    playbackMode: Playlist.Random
    PlaylistItem { source: "/" }
    onError: { console.log("ERROR") }
  }

  MediaPlayer {
    id: player
    autoPlay: true
    playlist: playlist
  }

  VideoOutput {
    id: videoOutput
    source: player
    anchors.fill: parent
  }
  
  Component.onCompleted: {
    playlist.shuffle()
    console.log(playlist.itemCount)
  }
}
