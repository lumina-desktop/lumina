//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the main browsing frontend for the file manager
//===========================================
#ifndef _LUMINA_FM_BROWSE_FRONTEND_H
#define _LUMINA_FM_BROWSE_FRONTEND_H

#include "Browser.h"
#include "widgets/DDListWidgets.h"

class BrowserWidget : public QWidget{
	Q_OBJECT
private:
	Browser *DIR:
	QString ID;
	int numItems; //used for checking if all the items have loaded yet
	bool details; //show details or not

	//The drag and drop brower widgets
	DDListWidget *listWidget;
	DDTreeWidget *treeWidget;

public:
	BrowserWidget(QString objID, QWidget *parent = 0);
	~BrowserWidget();

	QString id(){ return ID; }

	void changeDirectory(QString dir);

	void showDetails(bool show);
	bool hasDetails();

public slots:

private slots:
	//Brower connections
	void clearItems();
	void itemUpdated(QString);
	void itemDataAvailable(QIcon, LFileInfo);
	void itemsLoading(int total);

signals:
	//void activated(QString); //current dir path
	void dirChanged(QString); //current dir path
	
};

/*
 * Virtual class for managing the sort of folders/files items. The problem with base class is that it only manages texts fields and
 * we have dates and sizes.
 *
 * On this class, we overwrite the function operator<.
 */

class CQTreeWidgetItem : public QTreeWidgetItem {
public:
    CQTreeWidgetItem(int type = Type) : QTreeWidgetItem(type) {}
    CQTreeWidgetItem(const QStringList & strings, int type = Type) : QTreeWidgetItem(strings, type) {}
    CQTreeWidgetItem(QTreeWidget * parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    CQTreeWidgetItem(QTreeWidget * parent, const QStringList & strings, int type = Type) : QTreeWidgetItem(parent, strings, type) {}
    CQTreeWidgetItem(QTreeWidget * parent, QTreeWidgetItem * preceding, int type = Type) : QTreeWidgetItem(parent, preceding, type) {}
    CQTreeWidgetItem(QTreeWidgetItem * parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    CQTreeWidgetItem(QTreeWidgetItem * parent, const QStringList & strings, int type = Type) : QTreeWidgetItem(parent, strings, type) {}
    CQTreeWidgetItem(QTreeWidgetItem * parent, QTreeWidgetItem * preceding, int type = Type) : QTreeWidgetItem(parent, preceding, type) {}
    virtual ~CQTreeWidgetItem() {}
    inline virtual bool operator<(const QTreeWidgetItem &tmp) const {
      int column = this->treeWidget()->sortColumn();
      // We are in date text
      if(column == DirWidget::DATEMOD || column == DirWidget::DATECREATE)
        return this->whatsThis(column) < tmp.whatsThis(column);
      // We are in size text
      else if(column == DirWidget::SIZE) {
        QString text = this->text(column);
        QString text_tmp = tmp.text(column);
        double filesize, filesize_tmp;
        // On folders, text is empty so we check for that
        // In case we are in folders, we put -1 for differentiate of regular files with 0 bytes.
        // Doing so, all folders we'll be together instead of mixing with files with 0 bytes.
        if(text.isEmpty())
          filesize = -1;
        else
          filesize = LUtils::DisplaySizeToBytes(text);
        if(text_tmp.isEmpty())
          filesize_tmp = -1;
        else
          filesize_tmp = LUtils::DisplaySizeToBytes(text_tmp);
        return filesize < filesize_tmp;
      }
      // In other cases, we trust base class implementation
      return QTreeWidgetItem::operator<(tmp);
    }
};
#endif
