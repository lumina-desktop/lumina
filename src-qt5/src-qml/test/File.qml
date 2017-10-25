import QtQuick 2.0
import Qt.labs.folderlistmodel 2.1

ListView {
    width: 200; height: 400

    
    FolderListModel {
      id: folderModel 
      folder: "/usr/local/videos"
    }

    Component {
        id: fileDelegate
        Text { text: fileName }
    }

    model: folderModel
    delegate: fileDelegate
    Component.onCompleted: { console.log(folderModel.count) }
}
