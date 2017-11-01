//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore & JT Pennington
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the main browsing backend for the file manager
//===========================================
#ifndef _LUMINA_FM_BROWSER_MODEL_BACKEND_H
#define _LUMINA_FM_BROWSER_MODEL_BACKEND_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QMimeData>
#include <QMap>
#include <QVariant>
#include <QHash>

#include <LuminaXDG.h>

class BrowserModel : public QAbstractItemModel {
	Q_OBJECT
public:
	BrowserModel(QObject *parent = 0);
	~BrowserModel();

	//Virtual overrides
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;

	// item management
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	bool removeRow(int row, const QModelIndex &parent = QModelIndex());
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
	bool removeColumn(int column, const QModelIndex &parent = QModelIndex());
	bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

	//bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

	// data functions
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	// data modification functions
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);

	// drag and drop
	//QMimeData* mimeData(const QModelIndexList &indexes) const;
	//QStringList mimeTypes() const;
	//bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
	//Qt::DropActions supportedDropActions() const;
	//bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

	//Special Functions (non virtual replacements)
	LFileInfo* fileInfo(QString name);
	QString currentDirectory();

public slots:
	void loadDirectory(QString dir="");
	void loadItem(QString item);

private:
	QHash<quintptr, QModelIndex> HASH; //QString: "row/column"
	QString cDir;
	//simplification functions
	/*QString findInHash(QString path);
	QString findInHash(QModelIndex index);
	LFileInfo* indexToInfo(QString path);*/
	LFileInfo* indexToInfo(QModelIndex index);
	
private slots:

protected:

signals:

};

#endif

