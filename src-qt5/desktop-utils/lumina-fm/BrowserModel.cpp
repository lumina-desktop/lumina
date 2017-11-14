//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore & JT Pennington
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "BrowserModel.h"

BrowserModel::BrowserModel(QObject *parent){

}

BrowserModel::~BrowserModel(){

}

//Virtual overrides
QModelIndex BrowserModel::index(int row, int column, const QModelIndex &parent) const{

}

QModelIndex BrowserModel::parent(const QModelIndex &index) const{

}

// item management
int BrowserModel::rowCount(const QModelIndex &parent) const{

}

bool BrowserModel::insertRows(int row, int count, const QModelIndex &parent){

}

bool BrowserModel::removeRow(int row, const QModelIndex &parent){

}

bool BrowserModel::removeRows(int row, int count, const QModelIndex &parent){

}

int BrowserModel::columnCount(const QModelIndex &parent) const{

}

bool BrowserModel::insertColumns(int column, int count, const QModelIndex &parent){

}

bool BrowserModel::removeColumn(int column, const QModelIndex &parent){

}

bool BrowserModel::removeColumns(int column, int count, const QModelIndex &parent){

}


//bool BrowserModel::hasChildren(const QModelIndex &parent) const{

}


// data functions
Qt::ItemFlags BrowserModel::flags(const QModelIndex &index) const{

}

QVariant BrowserModel::data(const QModelIndex &index, int role) const{
  QFileInfo *info = indexToInfo(index);
  switch(role){
	case Qt::DisplayRole:
		if(index.column()==0){ return QVariant(info->fileName()); }
		else if(index.column()==1){ return QVariant(info->fileSize()); }
		else if(index.column()==2){ return QVariant(info->mimetype()); }
		else if(index.column()==3){ return QVariant(info->lastModified()->toString(Qt::DefaultLocaleShortDate)); }
		else if(index.column()==4){ return QVariant(info->created()->toString(Qt::DefaultLocaleShortDate)); }
	case Qt::ToolTipRole:
		return QVariant(info->absoluteFilePath());
	case Qt::StatusTipRole:
		return QVariant(info->absoluteFilePath());
	case Qt::SizeHintRole;
		return QVariant(QSize(100,14));
	case Qt::DecorationRole:
		return QVariant(LXDG::findIcon(info->iconFile(), "unknown"));
  }
  return QVariant();
}

QVariant BrowserModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if(role == Qt::DisplayRole){
      if(orientation == Qt::Horizontal){
        switch(section){
		case 0:
		  return QVariant(tr("File Name"));
		case 1:
		  return QVariant("Size");
		case 2:
		  return QVariant("Type");
		case 3:
		  return QVariant("Date Modified");
		case 4:
		  return QVariant("Date Created");
        }
      }
    }
	case Qt::DisplayRole:
		return QVariant(tr("File Name");
	/*case Qt::ToolTipRole:
		return QVariant("ToolTip");
	case Qt::StatusTipRole:
		return QVariant("Status Tip");
	case Qt::SizeHintRole;
		return QVariant(QSize(100,14));*/
	case Qt::DecorationRole:
		return QVariant("Icon"));
  }
  return QVariant();
}

// data modification functions
bool BrowserModel::setData(const QModelIndex &index, const QVariant &value, int role){

}

bool BrowserModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role){

}


// drag and drop
//QMimeData* BrowserModel::mimeData(const QModelIndexList &indexes) const{

}

//QStringList BrowserModel::mimeTypes() const{

}

//bool BrowserModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles){

}

//Qt::DropActions BrowserModel::supportedDropActions() const{

}

//bool BrowserModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent){

}

//Special Functions (non virtual replacements)
LFileInfo* BrowserModel::fileInfo(QString name){

}

QString BrowserModel::currentDirectory(){
  return cDir;
}


// ==============
//    PUBLIC SLOTS
// ==============
void BrowserModel::loadDirectory(QString dir){


}

void BrowserModel::loadItem(QString itempath){
  LFileInfo *it = new LFileInfo(itempath);
  //Determine the row/column that it needs to be
  int row, column;
  row = 0;
  //Now create the index
  for(int i=0; i<5; i++){
    QModelIndex index = createIndex(row, i, it);
  }
}


// =============
//      PRIVATE
// =============
/*QString BrowserModel::findInHash(QString path){
  QStringList keys = HASH.keys();
  for(int i=0; i<keys.length(); i++){
    if(HASH[keys[i]]->filePath() == path){ return keys[i]; }
  }
  return "";
}

QString BrowserModel::findInHash(QModelIndex index){
  QString id = QString::number(index.row())+"/"+QString::number(index.column());
  if(HASH.contains(id)){ return id; }
  return "";
}

LFileInfo* BrowserModel::indexToInfo(QString path){
  QString id = findInHash(path);
  if(id.isEmpty()){ return 0;}
  return HASH[id];
}*/

LFileInfo* BrowserModel::indexToInfo(QModelIndex index){
  return static_cast<LFileInfo*>(index.internalPointer());
}
